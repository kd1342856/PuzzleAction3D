#pragma once
class Entity;
class EditorScene;
class EditorUI;
class EditorCamera;
class Entity;
enum class EditorMode;
class CameraBase;

enum class CamView { TPS, Overhead };

class EditorManager
{
public:

	void Init();
	void Update();
	void Draw();

	bool GetUseTPSCamera() const { return m_useTPSCamera; }
	void SetUseTPSCamera(bool v);

	EditorMode GetMode() const;
	void SetMode(EditorMode mode);
	
	// これを「コピー」ではなく「参照バインド」にする
	void SetEntityList(std::vector<std::shared_ptr<Entity>>& list) { m_entityListRef = &list; }
	std::vector<std::shared_ptr<Entity>>& GetEntityList() { return m_entityListRef ? *m_entityListRef : m_fallbackList; }

	// 便利ヘルパ
	int  FindByName(const std::string& name) const;
	void SelectIndex(int idx) { m_selectedIndex = idx; }

	// 追加系は必ずここ経由で（Hierarchyからの追加も同じ配列に入る）
	void PushEntity(const std::shared_ptr<Entity>& e);

	bool IsEditorMode()const;

	std::shared_ptr<EditorScene> GetScene()const { return m_scene; }

	void SetCameras(const std::shared_ptr<CameraBase>& tpsCam,
		const std::shared_ptr<CameraBase>& overheadCam);

	void SetActiveCamView(CamView v);

	std::shared_ptr<CameraBase> GetOverheadCamera() const { return m_wpOverheadCam.lock(); }


	void DrawCameraPanel();

	CamView GetActiveCamView() const { return m_camView; }

private:

	void ApplyCameraState();

	std::vector<std::shared_ptr<Entity>>* m_entityListRef = nullptr; // バインド先
	std::vector<std::shared_ptr<Entity>>  m_fallbackList;            // 未バインド時用


	bool m_useTPSCamera = true;
	std::weak_ptr<CameraBase> m_wpTPSCam;
	std::weak_ptr<CameraBase> m_wpOverheadCam;
	CamView m_camView = CamView::Overhead; // 初期は俯瞰に

	std::shared_ptr<EditorScene>	m_scene;
	std::shared_ptr<EditorUI>		m_ui;
	int m_selectedIndex = -1;

};