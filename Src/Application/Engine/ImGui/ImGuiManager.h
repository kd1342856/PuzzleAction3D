#pragma once
class Entity;
class EditorManager;
class CameraBase;
class GameScene;
enum class EditorMode;

class ImGuiManager
{
public:
	void GuiInit();
	void GuiProcess();

	void SetMode(EditorMode m);

	void ToggleMode();

	void SetCameras(const std::shared_ptr<CameraBase>& tps,
		const std::shared_ptr<CameraBase>& overhead);
	std::shared_ptr<EditorManager> m_editor;

	void SetGameScene(const std::weak_ptr<GameScene>& gs) { m_wpGameScene = gs; } // ← 追加

	float GetWheelDelta() const { return m_lastWheel; }
	bool GetGameViewUVFromMouse(float& u, float& v) const;
	bool IsMouseOverGameView() const;

private:
	void GuiRelease();

	void GameScreen();
	void DrawMainMenu();
	void DrawGame();

	std::weak_ptr<GameScene> m_wpGameScene;
	bool   m_gameImgValid = false;
	ImVec2 m_gameImgMin{}, m_gameImgMax{};
	float m_lastWheel = 0.0f;

private:
	ImGuiManager() {}
	~ImGuiManager() { GuiRelease(); }
public:
	static ImGuiManager& Instance() 
	{
		static ImGuiManager Instance;
		return Instance;
	}
};
