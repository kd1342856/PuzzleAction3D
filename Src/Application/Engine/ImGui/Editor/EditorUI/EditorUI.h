#pragma once
class EditorManager;
class Entity;
class EditorUI
{
public:
	EditorUI(){}
	~EditorUI(){}

	void Update(EditorManager& editor);
	void DrawEditorUI(std::vector<std::shared_ptr<Entity>>& list);
	void DrawEntityInspector(std::vector<std::shared_ptr<Entity>>& list);

	int GetSelectedIndex() { return m_selectedEntityIndex; }
private:
	int m_selectedEntityIndex = -1;
};