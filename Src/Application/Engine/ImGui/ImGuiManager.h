#pragma once
class Entity;
class EditorManager;
enum class EditorMode;
class ImGuiManager
{
public:
	void GuiInit();
	void GuiProcess();

	void SetMode(EditorMode m);

	void ToggleMode();
private:
	void GuiRelease();
	std::shared_ptr<EditorManager> m_editor;

	void GameScreen();
	void DrawMainMenu();
	void DrawGame();

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
