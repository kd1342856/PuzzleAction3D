#include "EditorBridge.h"
#include "../../../Engine/ImGui/ImGuiManager.h"
#include "../../../Engine/ImGui/Editor/EditorManager.h"
#include "../../../Engine/ImGui/Editor/EditorScene/EditorScene.h"
#include "../../../GameObject/Camera/CameraBase.h"
#include "../../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../../Engine/Entity/Entity/Entity.h"
#include "../../../Engine/Engine.h"

namespace
{
	inline bool QueryIsGameMode()
	{
		if (auto editor = ImGuiManager::Instance().m_editor)
		{
			return !editor->IsEditorMode();
		}
		return true;
	}
}
namespace EditorBridge 
{
	bool ToggleEditorGame()
	{
		auto& imgui = ImGuiManager::Instance();
		imgui.ToggleMode();
		return QueryIsGameMode();
	}
	bool IsGameMode()
	{
		return QueryIsGameMode();
	}
	void SetModeGame()
	{
		ImGuiManager::Instance().SetMode(EditorMode::Game);
	}
	void SetCameras(const std::shared_ptr<CameraBase>& tps, const std::shared_ptr<CameraBase>& overhead)
	{
		if (auto editor = ImGuiManager::Instance().m_editor)
		{
			editor->SetCameras(tps, overhead);
		}
	}
	void SetUseTPSCamera(bool enable)
	{
		if (auto editor = ImGuiManager::Instance().m_editor)
		{
			editor->SetUseTPSCamera(enable);
		}
	}
	void ApplyOverheadOnce(OverheadCamPose& pending)
	{
		if (!pending.has)return;

		if (auto editor = ImGuiManager::Instance().m_editor)
		{
			if (!pending.has) return;
			if (auto editor = ImGuiManager::Instance().m_editor) 
			{
				if (auto overhead = editor->GetOverheadCamera())
				{
					overhead->SetPosition(pending.pos);
					overhead->SetEulerDeg(pending.rot);
					pending.has = false;      // ← 参照なので元に反映される
				}
			}
		}
	}
	void SyncFromScene(EditorSyncArgs& args)
	{
		if (auto editor = ImGuiManager::Instance().m_editor) 
		{
			editor->SetEntityList(args.sceneEntities);
			editor->SetCameras(args.tpsCam, args.overheadCam);
		}
	}
}
