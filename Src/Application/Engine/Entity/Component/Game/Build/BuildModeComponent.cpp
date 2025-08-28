#include "BuildModeComponent.h"
#include "../../../Entity/Entity.h"
#include "../../Trans/TransformComponent.h"
#include "../../Render/RenderComponent.h"
#include "../../Collider/ColliderComponent.h"
#include "../../../../../main.h"
#include "../../../../Engine.h"
#include "../../../../../GameObject/Camera/CameraBase.h"
#include "../../../../../Scene/SceneManager.h"
#include "../../../../ImGui/ImGuiManager.h"
#include "../../../../ImGui/Editor/EditorManager.h"
#include "../../Controller/Player/PlayerCtrlComp.h"

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
	if (!m_enabled) return;

	ImGuiIO& io = ImGui::GetIO();

	// ① ホイールでレイヤー移動：ゲームビュー上のみ反応
	{
		// 修飾キーでステップを変える：Alt=×5, Ctrl=×0.1
		float step = m_grid;
		if (GetAsyncKeyState(VK_MENU) & 0x8000) step *= 5.0f;  // Alt
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000) step *= 0.1f;  // Ctrl

		// ★ ゲームビュー上にマウスがある時だけ処理
		if (ImGuiManager::Instance().IsMouseOverGameView()) {
			float wheel = ImGuiManager::Instance().GetWheelDelta();
			if (std::abs(wheel) > 0.0f) {
				m_layerY += (wheel > 0.0f ? +step : -step);
				m_layerY = std::round(m_layerY / m_grid) * m_grid;
			}
		}

		// キーでも調整（Rで上げる、Fで下げる：こちらは常時OK）
		if (GetAsyncKeyState('R') & 0x8000) { m_layerY += step; m_layerY = std::round(m_layerY / m_grid) * m_grid; }
		if (GetAsyncKeyState('F') & 0x8000) { m_layerY -= step; m_layerY = std::round(m_layerY / m_grid) * m_grid; }
	}

	// --- ゴースト追従（Shiftで空中モード、何も押さなければ地面/ブロック優先） ---
	Math::Vector3 hover;
	if (PickPlacePoint(hover)) {
		UpdateGhostAt(GridSnap(hover, m_grid));
	}
	else {
		HideGhost();
	}

	// --- 左クリックで確定 ---
	bool lmb = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	bool pressed = (lmb && !m_prevLMB);
	m_prevLMB = lmb;
	if (!pressed) return;

	Math::Vector3 p;
	if (!PickPlacePoint(p)) return;
	PlaceBlockAt(GridSnap(p, m_grid));
}

bool BuildModeComponent::MakeCenterRayFromMainCamera(Math::Vector3& outRayPos, Math::Vector3& outRayDir) const 
{
	std::shared_ptr<CameraBase> sceneCam;
	for (auto& obj : SceneManager::Instance().GetObjList())
	{
		if (auto cam = std::dynamic_pointer_cast<CameraBase>(obj))
		{
			if (cam->IsActive()) { sceneCam = cam; break; }
		}
	}
	if (!sceneCam) return false;

	auto cam = sceneCam->GetCamera();
	if (!cam) return false;

	// クライアント座標のマウス位置を取得してレイ生成
	POINT pt{};
	::GetCursorPos(&pt);
	HWND hwnd = Application::Instance().GetWindowHandle();
	::ScreenToClient(hwnd, &pt);

	float range = 0.0f;
	cam->GenerateRayInfoFromClientPos(pt, outRayPos, outRayDir, range);
	if (outRayDir.LengthSquared() < 1e-6f) return false;
	outRayDir.Normalize();
	return true;
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

	return entity;
}

bool BuildModeComponent::PickPlacePoint(Math::Vector3& outPos)
{
	auto cam = m_wpActiveCam.lock();
	if (!cam) return false;

	// まず画面内のマウス位置からレイを作る（ダメならセンターレイ）
	Math::Vector3 r0{}, dir{};
	float u = 0.0f, v = 0.0f;
	if (ImGuiManager::Instance().GetGameViewUVFromMouse(u, v)) {
		Math::Matrix V = cam->GetViewMatrix();
		Math::Matrix P = cam->GetProjMatrix();
		Math::Matrix VPInv = (V * P).Invert();

		Math::Vector3 pNear = { 2.0f * u - 1.0f, 1.0f - 2.0f * v, 0.0f };
		Math::Vector3 pFar = { 2.0f * u - 1.0f, 1.0f - 2.0f * v, 1.0f };

		Math::Vector3 wNear = DirectX::XMVector3TransformCoord(pNear, VPInv);
		Math::Vector3 wFar = DirectX::XMVector3TransformCoord(pFar, VPInv);
		r0 = wNear;
		dir = wFar - wNear;
		if (dir.LengthSquared() < 1e-6f) return false;
		dir.Normalize();
	}
	else {
		if (!MakeCenterRayFromMainCamera(r0, dir)) return false;
	}

	// Shift 押しながらなら空中強制（地面ヒットを見ない）
	bool forceAir = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;

	// 1) 地面/ブロックに当たればその点を採用（強制空中でなければ）
	if (!forceAir) {
		Math::Vector3 hit{};
		if (RaycastGroundOrBlocks(r0, dir, hit)) {
			outPos = hit;
			return true;
		}
	}

	// 2) ヒット無し or 空中強制 → 高さレイヤー平面 y = m_layerY と交差
	if (std::abs(dir.y) > 1e-6f) {
		float t = (m_layerY - r0.y) / dir.y;
		if (t > 0.0f) {
			outPos = r0 + dir * t;
			return true;
		}
	}

	// 3) レイが平面と平行（dir.y ≈ 0）のとき：固定距離先で y を強制的に合わせる
	{
		float t = 5.0f; // 任意の距離（必要なら調整）
		Math::Vector3 p = r0 + dir * t;
		p.y = m_layerY;
		outPos = p;
		return true;
	}
}

bool BuildModeComponent::RaycastGroundOrBlocks(const Math::Vector3& r0, const Math::Vector3& dir, Math::Vector3& outPos)
{
	KdCollider::RayInfo ray(KdCollider::TypeGround | KdCollider::TypeBump, r0, dir, 1000.0f);

	bool hit = false;
	float best = -FLT_MAX;  // overlapDistance の最大を採用（エンジン仕様に合わせる）
	Math::Vector3 bestPos{};

	for (auto& w : m_hitEntities) {
		if (auto e = w.lock()) {
			if (!e->HasComponent<ColliderComponent>()) continue;
			auto& col = e->GetComponent<ColliderComponent>();
			std::list<KdCollider::CollisionResult> res;
			if (col.Intersects(ray, &res)) {
				for (auto& r : res) {
					if (r.m_overlapDistance > best) {
						best = r.m_overlapDistance;
						bestPos = r.m_hitPos;
						hit = true;
					}
				}
			}
		}
	}
	if (hit) outPos = bestPos;
	return hit;
}

bool BuildModeComponent::PickOnGround(Math::Vector3& outHitPos)
{
	auto cam = m_wpActiveCam.lock();
	if (!cam) return false;

	Math::Vector3 r0, dir;
	float u = 0.0f, v = 0.0f;

	// ★ UVが取れたら NDC→ワールドでレイを作る
	if (ImGuiManager::Instance().GetGameViewUVFromMouse(u, v))
	{
		Math::Matrix V = cam->GetViewMatrix();
		Math::Matrix P = cam->GetProjMatrix();
		Math::Matrix VPInv = (V * P).Invert();

		Math::Vector3 pNear = { 2.0f * u - 1.0f, 1.0f - 2.0f * v, 0.0f };
		Math::Vector3 pFar = { 2.0f * u - 1.0f, 1.0f - 2.0f * v, 1.0f };

		Math::Vector3 wNear = DirectX::XMVector3TransformCoord(pNear, VPInv);
		Math::Vector3 wFar = DirectX::XMVector3TransformCoord(pFar, VPInv);
		r0 = wNear;
		dir = wFar - wNear;
		if (dir.LengthSquared() < 1e-6f) return false;
		dir.Normalize();
	}
	else
	{
		// ★ UVが取れないときだけフォールバック
		if (!MakeCenterRayFromMainCamera(r0, dir)) return false;
	}

	// コリジョン：Ground/Bump を対象に長いレイ
	KdCollider::RayInfo ray(KdCollider::TypeGround | KdCollider::TypeBump, r0, dir, 1000.0f);

	bool hit = false;
	float bestDist = -FLT_MAX; // overlapDistance で最大を取るエンジンならその仕様に合わせる
	Math::Vector3 bestPos{};

	for (auto& w : m_hitEntities) {
		if (auto e = w.lock()) {
			if (!e->HasComponent<ColliderComponent>()) continue;
			auto& col = e->GetComponent<ColliderComponent>();
			std::list<KdCollider::CollisionResult> results;
			if (col.Intersects(ray, &results)) {
				for (auto& r : results) {
					// ここはエンジンの当たり結果仕様に合わせて選択
					if (r.m_overlapDistance > bestDist) {
						bestDist = r.m_overlapDistance;
						bestPos = r.m_hitPos;
						hit = true;
					}
				}
			}
		}
	}

	if (hit) outHitPos = bestPos;
	return hit;
}

void BuildModeComponent::PlaceBlockAt(const Math::Vector3& posSnapped)
{
	auto ent = std::make_shared<Entity>();
	ent->SetName("Block");

	// Transform
	auto tf = std::make_shared<TransformComponent>();
	tf->SetPos(posSnapped);
	tf->SetRotation({ 0,0,0 });
	tf->SetScale({ 1,1,1 });
	ent->AddComponent<TransformComponent>(tf);

	// Render
	auto rc = std::make_shared<RenderComponent>();
	ent->AddComponent<RenderComponent>(rc);
	if (!m_blockModelPath.empty()) 
	{
		rc->SetModelData(m_blockModelPath);   // 静的でOK
	}

	// Collider
	auto cc = std::make_shared<ColliderComponent>();
	cc->Init();
	ent->AddComponent<ColliderComponent>(cc);

	// モデル由来の三角形コライダー（あれば）
	bool registered = false;
	if (auto md = rc->GetModelData()) 
	{
		cc->RegisterModel("block", md, KdCollider::TypeGround | KdCollider::TypeBump);
		registered = true;
	}
	// フォールバック：単純なAABBを1グリッドの箱として登録（必要ならサイズ調整）
	if (!registered) 
	{
		Math::Vector3 half = { m_grid * 0.5f, m_grid * 0.5f, m_grid * 0.5f };
		cc->RegisterAABB("block_box",
			posSnapped - half, posSnapped + half,
			KdCollider::TypeGround | KdCollider::TypeBump);
	}

	ent->Init();

	// シーンへ
	SceneManager::Instance().AddObject(ent);
	if (auto ed = ImGuiManager::Instance().m_editor) 
	{
		ed->GetEntityList().push_back(ent);
	}

	// ビルド用のヒット対象にも登録（既存）
	RegisterHitEntity(ent);

	// ★ プレイヤーの当たり対象にも登録 ← これが重要！
	if (auto owner = m_owner.lock()) {
		if (owner->HasComponent<PlayerCtrlComp>()) 
		{
			owner->GetComponent<PlayerCtrlComp>().RegisterHitEntity(ent);
		}
	}
}
