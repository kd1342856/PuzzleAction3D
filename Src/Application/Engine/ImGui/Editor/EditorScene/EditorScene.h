#pragma once
class Entity;
enum class EditorMode
{
	Editor,
	Game
};
class EditorScene
{
public:
	EditorScene(){}
	~EditorScene(){}

	void Init();
	void Update();
	void Draw();


	void SetMode(EditorMode mode) { m_mode = mode; }
	EditorMode GetMode()const { return m_mode; }
	bool IsEditorMode()const { return m_mode == EditorMode::Editor; }

	std::vector<std::shared_ptr<Entity>>& GetEntityList() { return m_entityList; }
	const std::vector<std::shared_ptr<Entity>>& GetEntityList()const { return m_entityList; }

private:
	EditorMode m_mode = EditorMode::Editor;
	std::vector<std::shared_ptr<Entity>> m_entityList;
};
