#pragma once
class Entity;
class EditorScene;
class EditorUI;
class EditorCamera;
enum class EditorMode;
class EditorManager
{
public:

	void Init();
	void Update();
	void Draw();

	EditorMode GetMode() const;
	void SetMode(EditorMode mode);
	
	std::vector<std::shared_ptr<Entity>>& GetEntityList();
	void SetEntityList(const std::vector<std::shared_ptr<Entity>>& list);

	bool IsEditorMode()const;

	std::shared_ptr<EditorScene> GetScene()const { return m_scene; }

private:
	std::shared_ptr<EditorScene>	m_scene;
	std::shared_ptr<EditorUI>		m_ui;

};