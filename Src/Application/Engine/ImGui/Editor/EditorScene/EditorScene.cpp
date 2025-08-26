#include "EditorScene.h"
#include "../../ImGuiManager.h"
#include "../EditorUI/EditorUI.h"
#include "../../Log/Logger.h"
void EditorScene::Init()
{
}
void EditorScene::Update()
{
}
void EditorScene::Draw()
{
	if (!IsEditorMode())return;
	EngineCore::Logger::DrawImGui();
}
