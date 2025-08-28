#include "GameModeManager.h"
#include "../Controller/Player/PlayerCtrlComp.h"
#include "Build/BuildModeComponent.h"
#include "../../Entity/Entity.h"
#include "../../../Engine.h"
#include "../../../../GameObject/Camera/CameraBase.h"

void GameModeManager::Init() 
{
	SetMode(m_mode);
}

void GameModeManager::Update() 
{
	// Tabで切替
	bool nowTab = (GetAsyncKeyState(VK_TAB) & 0x8000) != 0;

	// デバッグ：押した瞬間にだけログ
	if (nowTab && !m_prevTab) 
	{
		m_mode = (m_mode == Mode::Build) ? Mode::Play : Mode::Build;
		SetMode(m_mode);
	}
	m_prevTab = nowTab;
}

void GameModeManager::SetMode(Mode m) 
{
	m_mode = m;

	auto owner = m_owner.lock();
	if (!owner) return;

	const bool isPlay = (m_mode == Mode::Play);
	const bool isBuild = !isPlay;

	// --- プレイヤー操作の有効/無効 ---
	if (owner->HasComponent<PlayerCtrlComp>())
		owner->GetComponent<PlayerCtrlComp>().SetEnabled(isPlay);

	// --- ビルド操作の有効/無効（＆俯瞰カメラ注入）---
	if (owner->HasComponent<BuildModeComponent>())
	{
		auto& build = owner->GetComponent<BuildModeComponent>();
		build.SetEnabled(isBuild);
		if (isBuild)
		{
			if (auto oh = m_wpOverhead.lock())
				build.SetActiveCamera(oh);
		}
	}

	// --- カメラとカーソルを一元反映 ---
	ApplyCamerasAndCursor();
}

void GameModeManager::SetCameras(const std::shared_ptr<CameraBase>& playCam, const std::shared_ptr<CameraBase>& buildCam)
{
	m_wpTPS			= playCam;
	m_wpOverhead	= buildCam;
	ApplyCamerasAndCursor();
}

void GameModeManager::ApplyCamerasAndCursor()
{
	auto tps = m_wpTPS.lock();
	auto oh  = m_wpOverhead.lock();

	const bool isPlay = (m_mode == Mode::Play);
	const bool isBuild = !isPlay;

	if (tps) tps->SetActive(isPlay);
	if (oh)  oh->SetActive(isBuild);

	// ★ カーソル捕捉：Play=捕捉 / Build=解放
	EngineCore::Engine::Instance().SetMouseGrabbed(isPlay);
}
