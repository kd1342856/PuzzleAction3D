#pragma once

#include"../BaseScene/BaseScene.h"
#include"../../Engine/Entity/Entity/Entity.h"
#include "../../Engine/Data/Scene/SceneData.h"
class KdTexture;
class EditorManager;
class EditorCamera;
class TPSCamera;

class GameScene : public BaseScene
{
public :
	GameScene() {}
	~GameScene() {}

	void Init()  override;
	void ApplyPlayerSpawn(const Math::Vector3& pos, const Math::Vector3& rot) { m_pendingSpawn = { true, pos, rot }; }

	std::shared_ptr<Entity>       m_playerEnt;
private:
	struct PendingCam
	{
		bool has = false;
		Math::Vector3 pos{}, rot{};
	} m_pendingOverheadCam;

	void Event() override;

	struct PendingSpawn
	{
		bool has = false;
		Math::Vector3 pos{}, rot{};
	} 
	m_pendingSpawn;

	std::shared_ptr<EditorManager> m_editor;
	std::vector<std::shared_ptr<Entity>> m_entities;
	std::shared_ptr<EditorCamera>	m_camera;
	std::shared_ptr<TPSCamera> m_playerCam;
	std::shared_ptr<Entity> m_buildEnt;


	bool m_playMode = false;
	bool m_prevP = false;
	bool m_prevTab = false;
};
