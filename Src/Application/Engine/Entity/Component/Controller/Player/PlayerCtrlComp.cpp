#include "PlayerCtrlComp.h"
#include "../../../Entity/Entity.h"
#include "../../Trans/TransformComponent.h"
#include "../../Collider/ColliderComponent.h"

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
}

void PlayerCtrlComp::Update()
{
	if (!m_enabled) return;

	auto owner = m_owner.lock();
	if (!owner || !owner->HasComponent<TransformComponent>()) return;
	auto& tf = owner->GetComponent<TransformComponent>();

	float dt = EngineCore::Time::DeltaTime();
	if (dt <= 0.0f || dt > 0.1f) dt = 1.0f / 60.0f;

	// --- 入力（WSAD）→ dirWorld の算出は今のまま ---
	Math::Vector3 input = { 0,0,0 };
	if (GetAsyncKeyState('W') & 0x8000) input.z += 1.0f;
	if (GetAsyncKeyState('S') & 0x8000) input.z -= 1.0f;
	if (GetAsyncKeyState('A') & 0x8000) input.x -= 1.0f;
	if (GetAsyncKeyState('D') & 0x8000) input.x += 1.0f;

	Math::Vector3 dirWorld = { 0,0,0 };
	if (input.x != 0 || input.z != 0) {
		input.Normalize();
		float yawRad = DirectX::XMConvertToRadians(m_camYawDeg);
		Math::Matrix camYaw = Math::Matrix::CreateRotationY(yawRad);
		dirWorld = DirectX::XMVector3TransformNormal(input, camYaw);
		dirWorld.y = 0.0f;
		if (dirWorld.LengthSquared() > 0.0001f) dirWorld.Normalize();

		// 方向向き補間（既存のまま）
		Math::Vector3 rot = tf.GetRotation();
		float wantYaw = DirectX::XMConvertToDegrees(std::atan2(dirWorld.x, dirWorld.z));
		float curYaw = rot.y;
		float diff = wantYaw - curYaw;
		while (diff > 180.0f)  diff -= 360.0f;
		while (diff < -180.0f) diff += 360.0f;
		const float k = 10.0f;
		float alpha = 1.0f - std::exp(-k * dt);
		alpha = std::clamp(alpha * m_turnLerp, 0.0f, 1.0f);
		rot.y = curYaw + diff * alpha;
		rot.x = 0.0f; rot.z = 0.0f;
		tf.SetRotation(rot);
	}

	// --- 位置更新（水平 + 垂直） ---
	Math::Vector3 pos = tf.GetPos();
	pos += dirWorld * m_moveSpeed * dt;

	// ▼ 接地判定（先に計算して m_grounded 更新）
	float groundY = pos.y;
	bool onGround = GroundCheck(pos, groundY);
	if (onGround && pos.y < groundY + m_groundSnap) 
	{
		// いま接地している（接地面に吸着）
		pos.y = groundY + m_groundSnap;
		if (m_vy < 0.0f) m_vy = 0.0f;   // 落下を止める
		m_grounded = true;
	}
	else {
		m_grounded = false;
	}

	// ▼ ジャンプ入力：スペースキー押下“瞬間”
	bool spaceNow = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
	if (spaceNow && !m_prevSpace)
	{
		if (m_grounded) {
			m_vy = m_jumpSpeed;         // 上向き初速
			m_grounded = false;         // 空中へ
		}
	}
	m_prevSpace = spaceNow;

	// ▼ 重力
	if (!m_grounded) {
		m_vy += m_gravity * dt;
	}
	pos.y += m_vy * dt;

	// ▼ 最後にもう一度接地クランプ（下向きに飛び出した分の保険）
	if (GroundCheck(pos, groundY) && pos.y < groundY + m_groundSnap) 
	{
		pos.y = groundY + m_groundSnap;
		if (m_vy < 0.0f) m_vy = 0.0f;
		m_grounded = true;
	}

	tf.SetPos(pos);

	// --- デバッグ（任意） ---
	if (auto dbg = owner->DebugWire()) {
		// レイ可視化は GroundCheck 内でライン追加済みにしてもOK
		dbg->AddDebugSphere(pos + m_bodyOffset, m_bodyRadius); // 体球
	}
}

bool PlayerCtrlComp::GroundCheck(const Math::Vector3& feetPos, float& outY)
{
	// 段差許容ぶんだけ原点を持ち上げた位置から下へ
	KdCollider::RayInfo ray(
		KdCollider::TypeGround,
		feetPos + Math::Vector3(0, m_stepHeight, 0),
		Math::Vector3::Down,
		m_stepHeight + 0.8f // 0.8 は余裕。必要なら調整
	);

	if (auto owner = m_owner.lock()) {
		if (auto dbg = owner->DebugWire())
		{
			dbg->AddDebugLine(ray.m_pos, ray.m_dir, ray.m_range);
		}
	}

	bool hit = false;
	float best = 0.0f;

	for (auto& w : m_hitEntities) {
		if (auto e = w.lock()) {
			if (!e->HasComponent<ColliderComponent>()) continue;
			auto& col = e->GetComponent<ColliderComponent>();
			std::list<KdCollider::CollisionResult> res;
			if (col.Intersects(ray, &res)) {
				for (auto& r : res) {
					if (r.m_overlapDistance > best) {
						best = r.m_overlapDistance;
						outY = r.m_hitPos.y;
						hit = true;
					}
				}
			}
		}
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

	for (int it = 0; it < m_bumpIters; ++it) {
		Math::Vector3 totalPush = { 0,0,0 };
		bool any = false;

		for (auto& w : m_hitEntities) {
			if (auto e = w.lock()) {
				if (!e->HasComponent<ColliderComponent>()) continue;

				auto& col = e->GetComponent<ColliderComponent>();
				std::list<KdCollider::CollisionResult> hits;
				if (col.Intersects(s, &hits)) {
					for (auto& h : hits) {
						// 押し戻しベクトル（法線方向 × 重なり量）
						totalPush += h.m_hitDir * h.m_overlapDistance;
						any = true;
					}
				}
			}
		}

		if (!any) break;

		pos += totalPush;

		// 球中心を更新して次反復へ
		sph.Center = pos + m_bodyOffset;
		s = KdCollider::SphereInfo(KdCollider::TypeBump, sph);
	}
}