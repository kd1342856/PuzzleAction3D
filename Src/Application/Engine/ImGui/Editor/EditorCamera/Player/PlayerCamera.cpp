#include "PlayerCamera.h"
#include "../../../../Entity/Entity/Entity.h"
#include "../../../../Entity/Component/Trans/TransformComponent.h"
#include "../../../../Engine.h"

void PlayerCamera::Init() 
{
	CameraBase::Init();
	m_DegAng = { 10.0f, 0.0f, 0.0f };    // pitch, yaw は CameraBase の m_DegAng を流用
	m_camPos = { 0, 3, -10 };
}

void PlayerCamera::PostUpdate() 
{
	if (!m_active) return;

	// ★ 基底クラスのターゲットを使う（これが SetTarget() で設定される）
	auto t = m_target.lock();
	if (!t || !t->HasComponent<TransformComponent>())
	{
		// ターゲット無しのときも「今の角度＋自分のローカル」で維持しておく
		if (EngineCore::Engine::Instance().m_isCameraControlActive) {
			UpdateRotateByMouse();
		}
		m_DegAng.x = std::clamp(m_DegAng.x, m_minPitch, m_maxPitch);
		m_mRotation = GetRotationMatrix();
		m_mWorld = (m_mRotation * m_mLocalPos); // 従来の挙動のまま一応成立させる
		return;
	}

	// マウスで回す（プレイ時に回したくないならフラグで制御）
	if (EngineCore::Engine::Instance().m_isCameraControlActive) {
		UpdateRotateByMouse();
	}
	m_DegAng.x = std::clamp(m_DegAng.x, m_minPitch, m_maxPitch);

	const auto& tf = t->GetComponent<TransformComponent>();
	Math::Vector3 targetPos = tf.GetPos();

	// yaw/pitch から「カメラの後ろ方向」を算出
	// forward(0,0,-1) を回転して、それをマイナスに取るのが “背後” でもOKですが、
	// ここでは back を直接作る書き方にしています。
	Math::Matrix rot = GetRotationMatrix();
	Math::Vector3 back = DirectX::XMVector3TransformNormal({ 0, 0, 1 }, rot); // カメラ後方ベクトル

	// 望ましいカメラ位置（ターゲットの少し上＋背後）
	Math::Vector3 desired = targetPos + Math::Vector3(0, m_height, 0) - back * m_distance;

	// スムーズ追従
	m_camPos = Math::Vector3::Lerp(m_camPos, desired, m_followLerp);

	// LookAt でビュー行列を作成し、ワールド行列に反映
	Math::Matrix view = Math::Matrix::CreateLookAt(
		m_camPos,
		targetPos + Math::Vector3(0, m_height * 0.5f, 0),   // ちょい上を注視
		{ 0, 1, 0 }
	);
	m_mWorld = view.Invert();
}