#include "BuildModeComponent.h"
#include "../../../Entity/Entity.h"
#include "../../Trans/TransformComponent.h"
#include "../../Render/RenderComponent.h"
#include "../../Collider/ColliderComponent.h"
#include "../../../../../main.h"
#include "../../../../Engine.h"
#include "../../../../../GameObject/Camera/CameraBase.h"

namespace 
{
	inline Math::Vector3 GridSnap(const Math::Vector3& p, float g) 
	{
		Math::Vector3 r = p;
		r.x = std::round(r.x / g) * g;
		r.y = std::round(r.y / g) * g;
		r.z = std::round(r.z / g) * g;
		return r;
	}
}

void BuildModeComponent::Init()
{
	// 俯瞰モード開始時にプレビューを用意
	EnsureGhost();
}

void BuildModeComponent::Update() 
{
	if (!m_enabled) { HideGhost(); return; }

	Math::Vector3 hit;
	if (PickGround(hit)) 
	{
		hit = GridSnap(hit, m_grid);
		UpdateGhostAt(hit);

		// LMB: 設置
		bool lmb = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		static bool prevL = false;
		if (lmb && !prevL) 
		{
			auto e = CreateBlock(hit);
			if (e) m_placedBlocks.push_back(e);
		}
		prevL = lmb;

		// RMB: 直近設置を削除（任意）
		bool rmb = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
		static bool prevR = false;
		if (rmb && !prevR) 
		{
			while (!m_placedBlocks.empty()) 
			{
				if (auto sp = m_placedBlocks.back().lock())
				{
					//EngineCore::World::RemoveEntity(sp);
					m_placedBlocks.pop_back();
					break;
				}
				else 
				{
					m_placedBlocks.pop_back();
				}
			}
		}
		prevR = rmb;
	}
	else 
	{
		HideGhost();
	}
}

bool BuildModeComponent::MakeCenterRayFromMainCamera(Math::Vector3& outRayPos, Math::Vector3& outRayDir) const 
{
	auto spCam = m_wpActiveCam.lock();
	if (!spCam) return false;

	// 1) マウス位置（クライアント座標）
	HWND hwnd = Application::Instance().GetWindowHandle();
	POINT pt; GetCursorPos(&pt);
	ScreenToClient(hwnd, &pt);

	// 2) ビューポートサイズ（ゲームRTに合わせる）
	int vpW = (int)EngineCore::Engine::Instance().GetGameViewTexture()->GetWidth();
	int vpH = (int)EngineCore::Engine::Instance().GetGameViewTexture()->GetHeight();
	if (vpW <= 0 || vpH <= 0) {
		Math::Viewport vp{}; KdDirect3D::Instance().CopyViewportInfo(vp);
		vpW = (int)vp.width; vpH = (int)vp.height;
	}

	// 3) マウス座標 → NDC
	float ndcX = (2.0f * pt.x) / vpW - 1.0f;
	float ndcY = -(2.0f * pt.y) / vpH + 1.0f;

	// 4) 近点/遠点（NDC）をワールドへ
	Math::Vector3 ndcNear(ndcX, ndcY, 0.0f);
	Math::Vector3 ndcFar(ndcX, ndcY, 1.0f);

	// CameraBase 側に合わせて取得（GetViewMatrix / GetProjMatrix がある前提）
	Math::Matrix view = spCam->GetViewMatrix();
	Math::Matrix proj = spCam->GetProjMatrix();
	Math::Matrix invVP = (view * proj).Invert();

	Math::Vector3 p0 = DirectX::XMVector3TransformCoord(ndcNear, invVP);
	Math::Vector3 p1 = DirectX::XMVector3TransformCoord(ndcFar, invVP);

	outRayPos = p0;
	outRayDir = p1 - p0;
	if (outRayDir.LengthSquared() < 1e-6f) return false;
	outRayDir.Normalize();
	return true;
}

bool BuildModeComponent::PickGround(Math::Vector3& outPos) const
{
	Math::Vector3 rayPos, rayDir;
	if (!MakeCenterRayFromMainCamera(rayPos, rayDir)) return false;

	// レイ情報
	KdCollider::RayInfo ray(KdCollider::TypeGround, rayPos, rayDir, 200.0f);

	// 可視化（任意）
	if (auto owner = m_owner.lock()) 
	{
		if (auto dbg = owner->DebugWire()) 
		{
			dbg->AddDebugLine(rayPos, rayDir, 200.0f);
		}
	}

	bool hit = false;
	float bestOverlap = 0.0f;
	Math::Vector3 bestHit = rayPos;

	for (auto& w : m_hitEntities) 
	{
		if (auto e = w.lock()) 
		{
			if (!e->HasComponent<ColliderComponent>()) continue;
			auto& col = e->GetComponent<ColliderComponent>();

			std::list<KdCollider::CollisionResult> res;
			if (col.Intersects(ray, &res)) 
			{
				for (auto& r : res) 
				{
					if (r.m_overlapDistance > bestOverlap) 
					{
						bestOverlap = r.m_overlapDistance;
						bestHit = r.m_hitPos;
						hit = true;
					}
				}
			}
		}
	}

	if (hit) outPos = bestHit;
	return hit;
}

void BuildModeComponent::EnsureGhost() 
{
	if (m_previewEntity) return;
	m_previewEntity = std::make_shared<Entity>();

	// 位置
	m_previewEntity->AddComponent<TransformComponent>(std::make_shared<TransformComponent>());
	auto& tf = m_previewEntity->GetComponent<TransformComponent>();
	tf.SetPos({ 0, -10000, 0 }); // 画面外

	// 見た目
	m_previewEntity->AddComponent<RenderComponent>(std::make_shared<RenderComponent>());
	auto& rc = m_previewEntity->GetComponent<RenderComponent>();
	rc.SetModelData(m_blockModelPath);   // スタティックでもOK（表示だけ）

	// ちょっと薄く表示したい場合は StandardShader の色レートをいじる箇所を
	// Entity::Visibility やマテリアル差し替えで対応（プロジェクト方針に合わせて）

	//EngineCore::World::AddEntity(m_previewEntity);
}

void BuildModeComponent::UpdateGhostAt(const Math::Vector3& pos) 
{
	EnsureGhost();
	if (!m_previewEntity) return;

	if (!m_previewEntity->HasComponent<TransformComponent>()) return;
	auto& tf = m_previewEntity->GetComponent<TransformComponent>();
	tf.SetPos(pos);
}

void BuildModeComponent::HideGhost() 
{
	if (!m_previewEntity) return;
	if (!m_previewEntity->HasComponent<TransformComponent>()) return;
	auto& tf = m_previewEntity->GetComponent<TransformComponent>();
	tf.SetPos({ 0, -10000, 0 });
}

std::shared_ptr<Entity> BuildModeComponent::CreateBlock(const Math::Vector3& pos)
{
	auto entity = std::make_shared<Entity>();
	entity->AddComponent<TransformComponent>(std::make_shared<TransformComponent>());
	// 位置
	auto& tf = entity->GetComponent<TransformComponent>();
	tf.SetPos(pos);

	entity->AddComponent<RenderComponent>(std::make_shared<RenderComponent>());
	// 表示（お好みで Work or Data）
	auto& rc = entity->GetComponent<RenderComponent>();
	rc.SetModelData(m_blockModelPath); // 表示用

	// 当たり（どちらか一方に統一）
	entity->AddComponent<ColliderComponent>(std::make_shared<ColliderComponent>());
	auto& col = entity->GetComponent<ColliderComponent>();
	col.Init();

	// ① モデル三角形で衝突（モデルの共有データを使う）
	col.RegisterModel(
		"solidModel",
		rc.GetModelData(),
		KdCollider::TypeGround | KdCollider::TypeBump);

	// ② シンプル箱：1m立方体の場合（モデルに依存しない）
	// ColliderComponent::BoxDesc box{ Math::Vector3::Zero, {0.5f, 0.5f, 0.5f} };
	// col.RegisterCollisionShape("solidBox", box, KdCollider::TypeGround | KdCollider::TypeBump);

	//EngineCore::World::AddEntity(e);

	// プレイヤーのヒット対象に渡したい場合は、GameModeManager かシーン側で
	// PlayerCtrlComp::RegisterHitEntity(e) を呼んでください。

	return entity;
}
