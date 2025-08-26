#include "Engine.h"
#include "ImGui/ImGuiManager.h"
#include "../Scene/SceneManager.h"
#include <DirectXColors.h>
#include "../main.h"
#include "../GameObject/Camera/CameraBase.h"
using namespace EngineCore;

bool Engine::Init()
{
	Logger::Log("Engine", "Engine initialized.");
	TaskManager::Init();


	m_gameViewRT = std::make_shared<KdTexture>();
	Math::Viewport vp{};
	KdDirect3D::Instance().CopyViewportInfo(vp);
	EnsureGameRTSize((int)vp.width, (int)vp.height);

	return true;
}

void Engine::Shutdown()
{
	Logger::Log("Engine", "Engine shutdown");
}

void Engine::Update()
{
	SceneManager::Instance().Update();
	//Logger::Log("Engine", "Engine update.");
}

void EngineCore::Engine::PostUpdate()
{
	SceneManager::Instance().PostUpdate();
}

void Engine::Draw()
{
	if (!m_gameViewRT || !m_gameViewRT->WorkRTView()) 
	{
		Math::Viewport vp{};
		KdDirect3D::Instance().CopyViewportInfo(vp);
		EnsureGameRTSize((int)vp.width, (int)vp.height);
	}

	auto rtv = m_gameViewRT->WorkRTView();
	auto dsv = KdDirect3D::Instance().WorkZBuffer()->WorkDSView();

	KdDirect3D::Instance().SetRenderTarget(rtv, dsv);
	KdDirect3D::Instance().ClearRenderTarget(rtv, DirectX::Colors::CornflowerBlue);
	KdDirect3D::Instance().ClearDepthStencil(dsv);
	SceneManager::Instance().Draw();
}

void EngineCore::Engine::Release()
{
	TaskManager::Shutdown(TaskManager::ShutdownMode::Graceful);
}

void EngineCore::Engine::SetMouseGrabbed(bool grab)
{
	if (m_mouseGrabbed == grab) return;
	m_mouseGrabbed = grab;

	HWND hwnd = Application::Instance().GetWindowHandle();

	if (grab) {
		// カーソル非表示（ShowCursorは参照カウント制）
		while (ShowCursor(FALSE) >= 0) {}
		// クライアント矩形にClip
		RECT rc; GetClientRect(hwnd, &rc);
		POINT tl{ rc.left, rc.top }, br{ rc.right, rc.bottom };
		ClientToScreen(hwnd, &tl); ClientToScreen(hwnd, &br);
		RECT clip{ tl.x, tl.y, br.x, br.y };
		ClipCursor(&clip);
		SetCapture(hwnd);

		// 中央へ一度だけ寄せたい場合（お好み）
		POINT c{ (tl.x + br.x) / 2, (tl.y + br.y) / 2 };
		SetCursorPos(c.x, c.y);
	}
	else {
		ReleaseCapture();
		ClipCursor(nullptr);
		while (ShowCursor(TRUE) < 0) {}
	}
}

void EngineCore::Engine::EnsureGameRTSize(int w, int h)
{
	if (w <= 0 || h <= 0) return;

	if (m_gameViewRT &&
		(int)m_gameViewRT->GetWidth() == w &&
		(int)m_gameViewRT->GetHeight() == h) {
		return; // 既に同サイズ
	}

	if (!m_gameViewRT) m_gameViewRT = std::make_shared<KdTexture>();
	m_gameViewRT->CreateRenderTarget(w, h);
}

void EngineCore::Engine::UpdateCameraProjectionForGameRT()
{
	if (auto camObj = SceneManager::Instance().GetObjList().empty()
		? nullptr
		: *std::find_if(SceneManager::Instance().GetObjList().begin(), SceneManager::Instance().GetObjList().end(),
			[](const std::shared_ptr<KdGameObject>& o) { return dynamic_cast<CameraBase*>(o.get()) != nullptr; }))
	{
		if (auto camBase = dynamic_cast<CameraBase*>(camObj.get())) {
			float aspect = (float)m_gameViewRT->GetWidth() / (float)m_gameViewRT->GetHeight();
			camBase->WorkCamera()->SetProjectionMatrix(
				60.0f,   // FOV はお好み
				1000.0f, // far
				0.1f,    // near
				aspect   // ← ここをRTのサイズに
			);
		}
	}
}
