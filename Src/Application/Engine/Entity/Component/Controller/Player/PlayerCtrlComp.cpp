#include "PlayerCtrlComp.h"
#include "../../../Entity/Entity.h"
#include "../../Trans/TransformComponent.h"
#include "../../Collider/ColliderComponent.h"
#include "../../Controller/Player/Anime/AnimatorComponent.h"

void PlayerCtrlComp::Init()
{
	auto owner = m_owner.lock();
	if (!owner) return;
	if (!owner->HasComponent<ColliderComponent>()) 
	{
		owner->AddComponent<ColliderComponent>(std::make_shared<ColliderComponent>());
		owner->GetComponent<ColliderComponent>().Init();
	}
	auto& col = owner->GetComponent<ColliderComponent>();
	col.RegisterSphere("body", m_bodyRadius, KdCollider::TypeBump | KdCollider::TypeGround);

	//if (!owner->HasComponent<AnimatorComponent>()) 
	//{
	//	owner->AddComponent<AnimatorComponent>(std::make_shared<AnimatorComponent>());
	//	owner->GetComponent<AnimatorComponent>().Init();
	//}
}

void PlayerCtrlComp::Update()
{
	if (!m_enabled) return;
	auto owner = m_owner.lock();
	if (!owner || !owner->HasComponent<TransformComponent>()) return;
	auto& tf = owner->GetComponent<TransformComponent>();

	float dt = EngineCore::Time::DeltaTime();
	if (dt <= 0.0f || dt > 0.1f) dt = 1.0f / 60.0f;

	// 入力
	Math::Vector3 input{ 0,0,0 };
	if (GetAsyncKeyState('W') & 0x8000) input.z += 1.0f;
	if (GetAsyncKeyState('S') & 0x8000) input.z -= 1.0f;
	if (GetAsyncKeyState('A') & 0x8000) input.x -= 1.0f;
	if (GetAsyncKeyState('D') & 0x8000) input.x += 1.0f;

	bool sprint = (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
	m_currentSpeed = m_moveSpeed * (sprint ? m_sprintMultiplier : 1.0f);

	m_moveDirWorld = { 0,0,0 };
	if (input.x != 0 || input.z != 0) 
	{
		input.Normalize();
		float yawRad = DirectX::XMConvertToRadians(m_camYawDeg);
		Math::Matrix camYaw = Math::Matrix::CreateRotationY(yawRad);
		m_moveDirWorld = DirectX::XMVector3TransformNormal(input, camYaw);
		m_moveDirWorld.y = 0.0f;
		if (m_moveDirWorld.LengthSquared() > 0.0001f) m_moveDirWorld.Normalize();

		// 見た目の向きだけここで
		Math::Vector3 rot = tf.GetRotation();
		float wantYaw = DirectX::XMConvertToDegrees(std::atan2(m_moveDirWorld.x, m_moveDirWorld.z));
		float curYaw = rot.y;
		float diff = wantYaw - curYaw;
		while (diff > 180.0f)  diff -= 360.0f;
		while (diff < -180.0f) diff += 360.0f;
		const float k = 10.0f;
		float alpha = 1.0f - std::exp(-k * dt);
		alpha = std::clamp(alpha * m_turnLerp, 0.0f, 1.0f);
		rot.y = curYaw + diff * alpha;
		rot.x = rot.z = 0.0f;
		tf.SetRotation(rot);

	}

	// ジャンプの「要求」だけ立てる（縁判定）
	bool spaceNow = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
	m_wantsJump = (spaceNow && !m_prevSpace);
	m_prevSpace = spaceNow;


	//if (owner->HasComponent<AnimatorComponent>()) 
	//{
	//	auto& anim = owner->GetComponent<AnimatorComponent>();

	//	float moveLen2 = m_moveDirWorld.x * m_moveDirWorld.x + m_moveDirWorld.z * m_moveDirWorld.z;

	//	if (!m_grounded && m_vy > 0.1f)
	//	{
	//		anim.Play("Attack", /*loop=*/true, 30.0f); // とりあえずジャンプ中の見た目に流用
	//	}
	//	else if (moveLen2 > 0.0001f) 
	//	{
	//		anim.Play("Walk", /*loop=*/true, 60.0f);    // 歩き
	//	}
	//	else 
	//	{
	//		//anim.Play("Walk", /*loop=*/true, 60.0f);    // 速度0でポーズ保持
	//	}
	//}
}

void PlayerCtrlComp::PostUpdate()
{
	if (!m_enabled) return;
	auto owner = m_owner.lock();
	if (!owner || !owner->HasComponent<TransformComponent>()) return;
	auto& tf = owner->GetComponent<TransformComponent>();

	float dt = EngineCore::Time::DeltaTime();
	if (dt <= 0.0f || dt > 0.1f) dt = 1.0f / 60.0f;

	Math::Vector3 pos = tf.GetPos();

	// 1) 水平移動
	pos += m_moveDirWorld * m_currentSpeed * dt;
	// 2) 横衝突解決（Yは押し戻さない）
	ResolveBump(pos); // 中で totalPush.y = 0; 忘れずに

	// 3) 接地プローブ（水平確定後のposで）
	float groundY = pos.y;
	bool probeGround = GroundCheck(pos, groundY);

	// 4) ジャンプ確定
	if (m_wantsJump && probeGround) {
		m_vy = m_jumpSpeed;
	}
	m_wantsJump = false; // 消費

	// 5) 重力
	if (!probeGround) m_vy += m_gravity * dt;
	else if (m_vy < 0.0f) m_vy = 0.0f;

	// 6) 垂直移動
	pos.y += m_vy * dt;

	// 7) 落下時のみスナップ
	if (m_vy <= 0.0f) {
		float gy = pos.y;
		if (GroundCheck(pos, gy) && pos.y < gy + m_groundSnap) {
			pos.y = gy + m_groundSnap;
			m_vy = 0.0f;
			m_grounded = true;
		}
		else {
			m_grounded = false;
		}
	}
	else {
		m_grounded = false;
	}

	tf.SetPos(pos);

	// デバッグ可視化（任意）
	if (auto dbg = owner->DebugWire()) {
		dbg->AddDebugSphere(pos + m_bodyOffset, m_bodyRadius);
	}
}

bool PlayerCtrlComp::GroundCheck(const Math::Vector3& feetPos, float& outY)
{
	// 足裏想定Y（体球中心 - 半径）
	float feetY = feetPos.y + (m_bodyOffset.y - m_bodyRadius);

	// レイ原点と長さ
	float probeExtra = 0.20f; // 余裕
	Math::Vector3 rayStart = { feetPos.x, feetY + m_stepHeight, feetPos.z };
	float rayLen = m_stepHeight + m_groundSnap + probeExtra;

	KdCollider::RayInfo ray(KdCollider::TypeGround, rayStart, Math::Vector3::Down, rayLen);

	if (auto owner = m_owner.lock())
	{
		if (auto dbg = owner->DebugWire())
		{
			dbg->AddDebugLine(rayStart, Math::Vector3::Down, rayLen); // 可視化
		}
	}

	bool hit = false;
	float bestOverlap = 0.0f;
	float bestY = feetY;

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
						bestY = r.m_hitPos.y; // 足裏が乗るべき面のY
						hit = true;
					}
				}
			}
		}
	}

	if (hit) 
	{
		// outY は足裏の接地Yから、キャラ原点Yに換算して返す
		// 原点（feetPos.y） = 足裏Y - (bodyOffset.y - bodyRadius)
		outY = bestY - (m_bodyOffset.y - m_bodyRadius);
	}
	return hit;
}

// --- 追加：横衝突の押し戻し ---
void PlayerCtrlComp::ResolveBump(Math::Vector3& pos)
{
	// 体球（中心はオフセットつき）
	DirectX::BoundingSphere sph;
	sph.Center = pos + m_bodyOffset;
	sph.Radius = m_bodyRadius;

	KdCollider::SphereInfo s(KdCollider::TypeBump, sph);

	for (int it = 0; it < m_bumpIters; ++it) 
	{
		Math::Vector3 totalPush = { 0,0,0 };
		bool any = false;

		for (auto& w : m_hitEntities) 
		{
			if (auto e = w.lock()) 
			{
				if (!e->HasComponent<ColliderComponent>()) continue;

				auto& col = e->GetComponent<ColliderComponent>();
				std::list<KdCollider::CollisionResult> hits;
				if (col.Intersects(s, &hits)) 
				{
					for (auto& h : hits)
					{
						// 押し戻しベクトル（法線方向 × 重なり量）
						totalPush += h.m_hitDir * h.m_overlapDistance;
						any = true;
					}
				}
			}
		}

		if (!any) break;
		totalPush.y = 0.0f;
		pos += totalPush;

		// 球中心を更新して次反復へ
		sph.Center = pos + m_bodyOffset;
		s = KdCollider::SphereInfo(KdCollider::TypeBump, sph);
	}
}