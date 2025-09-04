#include "GameScene.h"
#include"../SceneManager.h"

#include "Wiring/GameSceneWiring.h"
#include "Wiring/EditorBridge.h"

#include "../../Engine/Engine.h"
#include "../../Engine/Data/ObjData.h"
#include "../../GameObject/Map/Stage/StageLoader.h"
#include "../../Engine/System/Thread/MainThreadTask.h"
#include "../../Engine/System/Thread/LoadedEntityQueue.h"

#include "../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../Engine/ImGui/Editor/EditorCamera/EditorCamera.h"

#include "../../Engine/Entity/Entity/Entity.h"
#include "../../Engine/Entity/Component/Trans/TransformComponent.h"

using namespace EngineCore;
void GameScene::Event()
{
	std::shared_ptr<Entity> entity;
	while (LoadedEntityQueue::Instance().TryPop(entity))
	{
		if (entity->GetName() == "Player")
		{
			m_playerEnt = entity;
			if (m_playerCam) m_playerCam->SetTargetEntity(m_playerEnt);

			SceneWire::WirePlayer(m_playerEnt, m_playerCam, m_camera);

			for (auto& entity : m_entities)
			{
				if (entity && entity->GetName() == "Map")
				{
					SceneWire::WireMap(entity, { m_playerEnt });
				}
			}

			// 保留スポーン適用
			if (m_pendingSpawn.has && m_playerEnt->HasComponent<TransformComponent>())
			{
				auto& tf = m_playerEnt->GetComponent<TransformComponent>();
				tf.SetPos(m_pendingSpawn.pos);
				tf.SetRotation(m_pendingSpawn.rot);
				m_pendingSpawn.has = false;
			}
		}

		// -------- Map 到着時：当たりの準備と登録 --------
		if (entity->GetName() == "Map" && m_playerCam)
		{
			SceneWire::WireMap(entity, { m_playerEnt });
		}

		// シーン配列へ（重複なし）
		if (std::find(m_entities.begin(), m_entities.end(), entity) == m_entities.end())
		{
			m_entities.push_back(entity);
			m_objList.push_back(entity);
		}
	}

	// Pで Editor <-> Game をトグル（モード反映はGameModeManagerに一任）
	bool pNow = (GetAsyncKeyState('P') & 0x8000);
	if (pNow && !m_prevP)
	{
		const bool nowGame = EditorBridge::ToggleEditorGame();
		if (m_playerEnt)
		{
			SceneWire::ApplyModeForPlayer(m_playerEnt, nowGame);
		}

	}
	m_prevP = pNow;

	if (EditorBridge::IsGameMode() && m_playerEnt && m_playerCam)
	{
		SceneWire::SyncPlayerYawFromTPS(m_playerEnt, m_playerCam);
	}

	MainThreadTask::Instance().Drain();

	
	EditorBridge::ApplyOverheadOnce(m_pendingOverheadCam);

	// --- エディタ同期（エンティティリスト、カメラなど） ---
	EditorSyncArgs args;
	args.tpsCam = m_playerCam;
	args.overheadCam = m_camera;
	EditorBridge::SyncFromScene(args);
}

void GameScene::Init()
{
	m_camera = std::make_shared<EditorCamera>();  m_camera->Init();
	m_objList.push_back(m_camera);

	m_playerCam = std::make_shared<TPSCamera>();  m_playerCam->Init();
	m_objList.push_back(m_playerCam);

	EditorBridge::BindEntityList(m_entities);
	EditorBridge::SetUseTPSCamera(true);
	EditorBridge::SetCameras(m_playerCam, m_camera);
	EditorBridge::SetModeGame();
	
	if (m_playerCam) m_playerCam->SetActive(true);
	if (m_camera)    m_camera->SetActive(false);

	{
		ObjectData io;
		auto all = io.LoadJson("Asset/Data/ObjData/ObjData/ObjData.json");
		for (const auto& d : all)
		{
			if (d.name == "__OverheadCamera") 
			{
				m_pendingOverheadCam = { true, d.pos, d.rot };
				break;
			}
		}
	}

	GameStageSpec spec;
	spec.mapModelPath = "Asset/Models/Stage/Block/Block.gltf";
	spec.objDataPath = "Asset/Data/ObjData/ObjData/ObjData.json";
	spec.playerModelPath = "Asset/Models/Character/Cat/Cat.gltf";
	spec.playerDynamic = true;

	StageLoader::LoadStageAsync(spec, this);
}
