#include "CameraBase.h"
#include "../../Engine/Engine.h"

void CameraBase::Init()
{
	if (!m_spCamera)
	{
		m_spCamera = std::make_shared<KdCamera>();
	}
	// ↓画面中央座標
	m_FixMousePos.x = 640;
	m_FixMousePos.y = 360;

	m_spCamera->SetProjectionMatrix(
		60.0f,   // FOV
		1000.0f, // far
		0.1f,    // near
		1280.0f / 720.0f // aspect
	);
}

void CameraBase::PreDraw()
{
	if (!m_active || !m_spCamera) { return; }

	m_spCamera->SetCameraMatrix(m_mWorld);
	m_spCamera->SetToShader();
}

void CameraBase::SetTarget(const std::shared_ptr<KdGameObject>& target)
{
	if (!target) { return; }

	m_wpTarget = target;
}

void CameraBase::UpdateRotateByMouse()
{
	// マウスでカメラを回転させる処理
	POINT _nowPos;
	GetCursorPos(&_nowPos);

	POINT _mouseMove{};
	_mouseMove.x = _nowPos.x - m_FixMousePos.x;
	_mouseMove.y = _nowPos.y - m_FixMousePos.y;

	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);

	// 実際にカメラを回転させる処理(0.15はただの補正値)
	m_DegAng.x += _mouseMove.y * 0.15f;
	m_DegAng.y += _mouseMove.x * 0.15f;

	// 回転制御
	m_DegAng.x = std::clamp(m_DegAng.x, -45.f, 45.f);
}

void CameraBase::SyncMouseAnchorOnGrabBegin()
{
	static bool prevGrab = false;
	bool curGrab = EngineCore::Engine::Instance().IsMouseGrabbed();
	if (curGrab && !prevGrab) {
		// ★ この瞬間だけ現在座標を基準にする（飛び防止）
		GetCursorPos(&m_FixMousePos);
	}
	prevGrab = curGrab;
}
