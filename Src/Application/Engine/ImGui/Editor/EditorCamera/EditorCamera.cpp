#include "EditorCamera.h"
#include "../../../Engine.h"
void EditorCamera::Init()
{
	CameraBase::Init();
	m_mLocalPos = Math::Matrix::Identity;

	// 原点を向く初期姿勢（必要ならeyeをもっと遠くに）
	const Math::Vector3 eye = { 0, 3, -10 };
	const Math::Vector3 at = { 0, 0,   0 };
	const Math::Vector3 up = { 0, 1,   0 };

	m_mLocalPos = Math::Matrix::CreateTranslation(eye);
	const auto view = Math::Matrix::CreateLookAt(eye, at, up);
	m_mWorld = view.Invert(); // カメラのワールド行列はビューの逆

	// マウス基準位置（既存）
	SetCursorPos(m_FixMousePos.x, m_FixMousePos.y);
}

void EditorCamera::PostUpdate()
{
	const bool ctrl = EngineCore::Engine::Instance().m_isCameraControlActive;
	// ★ 立ち上がり検知：押した“瞬間”に基準点を今にする
	if (ctrl && !m_prevControl)
	{
		GetCursorPos(&m_FixMousePos);  // ← これで「上のほうでクリックしても」差分がゼロから始まる
		ShowCursor(false);
		// （必要なら）ShowCursor(FALSE); ClipCursor(...) などもここで
	}
	if (!ctrl && m_prevControl)
	{
		ShowCursor(true);
		// （必要なら）ShowCursor(TRUE); ClipCursor(nullptr);
	}
	m_prevControl = ctrl;

	if (ctrl)
	{
		UpdateRotateByMouse();                  // ← m_FixMousePos を中心に相対回転
		m_mRotation = GetRotationMatrix();

		Math::Vector3 move = Math::Vector3::Zero;
		if (GetAsyncKeyState('W') & 0x8000) move.z += 1.0f;
		if (GetAsyncKeyState('S') & 0x8000) move.z -= 1.0f;
		if (GetAsyncKeyState('A') & 0x8000) move.x -= 1.0f;
		if (GetAsyncKeyState('D') & 0x8000) move.x += 1.0f;
		if (GetAsyncKeyState('E') & 0x8000) move.y += 1.0f;
		if (GetAsyncKeyState('Q') & 0x8000) move.y -= 1.0f;

		float speed = (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? m_moveSpeed * 3.0f : m_moveSpeed;
		if (move.x || move.y || move.z) {
			Math::Vector3 delta = DirectX::XMVector3TransformNormal(move * speed, m_mRotation);
			m_mLocalPos *= Math::Matrix::CreateTranslation(delta);
		}
	}
	else
	{
		m_mRotation = GetRotationMatrix();
	}
	
	m_mWorld = m_mRotation * m_mLocalPos;
}

Math::Matrix EditorCamera::GetCameraMatrix() const
{
	return m_mWorld.Invert();
}
