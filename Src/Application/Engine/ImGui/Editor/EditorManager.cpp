#include "EditorManager.h"
#include "EditorScene/EditorScene.h"
#include "EditorUI/EditorUI.h"
#include "../../../Scene/SceneManager.h"
#include "EditorCamera/EditorCamera.h"

void EditorManager::Init()
{
	m_scene = std::make_shared<EditorScene>();
	m_ui = std::make_shared<EditorUI>();

}

void EditorManager::Update()
{
	m_scene->Update();
}

void EditorManager::Draw()
{
	if (!m_scene)return;
	m_scene->Draw();
	if (m_ui)
	{
		m_ui->Update(*this);
	}
}


EditorMode EditorManager::GetMode() const
{
	return m_scene->GetMode();
}

void EditorManager::SetMode(EditorMode mode)
{
	m_scene->SetMode(mode);
}

std::vector<std::shared_ptr<Entity>>& EditorManager::GetEntityList()
{
	return m_scene->GetEntityList();
}

void EditorManager::SetEntityList(const std::vector<std::shared_ptr<Entity>>& list)
{
	if (!m_scene)return;
	auto& targetList = m_scene->GetEntityList();
	targetList.clear();
	targetList.insert(targetList.end(), list.begin(), list.end());
}

bool EditorManager::IsEditorMode() const
{
	return m_scene && m_scene->IsEditorMode();
}
