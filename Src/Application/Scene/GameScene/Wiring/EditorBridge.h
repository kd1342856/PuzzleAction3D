#pragma once
#include "SceneSharedTypes.h" 

class Entity;
class CameraBase;
class TPSCamera;

namespace EditorBridge 
{
	bool ToggleEditorGame();
	bool IsGameMode();

	void SetModeGame();

	// カメラの登録（Editor 側に渡す）
	void SetCameras(const std::shared_ptr<CameraBase>& tps,
		const std::shared_ptr<CameraBase>& overhead);

	void SetUseTPSCamera(bool enable);

	// Overhead カメラの保留適用（1回きりで has=false にする）
	void ApplyOverheadOnce(OverheadCamPose& pending);

	// シーン→エディタへの単方向“橋渡し”API（ImGuiやEditorManagerの詳細は隠す）
	void SyncFromScene(EditorSyncArgs& args);

	void BindEntityList(std::vector<std::shared_ptr<Entity>>& list);

}