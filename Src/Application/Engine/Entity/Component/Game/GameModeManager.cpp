#include "GameModeManager.h"
#include "../Controller/Player/PlayerCtrlComp.h"
#include "Build/BuildModeComponent.h"
#include "../../Entity/Entity.h"

void GameModeManager::Init() 
{
	SetMode(m_mode);
}

void GameModeManager::Update() 
{
	// Tabで切替
	bool nowTab = (GetAsyncKeyState(VK_TAB) & 0x8000) != 0;
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

	// その都度取り出して切替（弱参照を保持しない）
	if (owner->HasComponent<PlayerCtrlComp>())
	{
		owner->GetComponent<PlayerCtrlComp>().SetEnabled(m == Mode::Play);
	}
	if (owner->HasComponent<BuildModeComponent>()) 
	{
		owner->GetComponent<BuildModeComponent>().SetEnabled(m == Mode::Build);
	}
}