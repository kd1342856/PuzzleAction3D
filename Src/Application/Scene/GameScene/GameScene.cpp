#include "GameScene.h"
#include"../SceneManager.h"
#include "../../Engine/Entity/Entity/Entity.h"
#include "../../Engine/Engine.h"
#include "../../Engine/Entity/Component/Trans/TransformComponent.h"
#include "../../Engine/Entity/Component/Collider/ColliderComponent.h"
#include "../../Engine/Entity/Component/Controller/Player/PlayerCtrlComp.h"
#include "../../Engine/Entity/Component/Render/RenderComponent.h"
#include "../../Engine/Entity/Entity/EntityFactory/EntityFactory.h"
#include "../../Engine/ImGui/Editor/EditorCamera/EditorCamera.h"
#include "../../Engine/ImGui/Editor/EditorCamera/Player/PlayerCamera.h"
#include "../../Engine/ImGui/Editor/EditorManager.h"
#include "../../Engine/ImGui/ImGuiManager.h"
#include "../../Engine/Data/ObjData.h"
#include "../../Engine/System/Thread/LoadedEntityQueue.h"
#include "../../Engine/System/Thread/MainThreadTask.h"
#include "../../GameObject/Map/Stage/StageLoader.h"
#include "../../GameObject/Camera/TPSCamera/TPSCamera.h"

using namespace EngineCore;
using namespace EntityFactory;
void GameScene::Event()
{
	std::shared_ptr<Entity> entity;
	while (LoadedEntityQueue::Instance().TryPop(entity))
	{
		if (entity->GetName() == "Player")
		{
			m_playerEnt = entity;
			if (m_playerCam) m_playerCam->SetTargetEntity(m_playerEnt);

			if (m_playerEnt->HasComponent<PlayerCtrlComp>()) 
			{
				auto& pc = m_playerEnt->GetComponent<PlayerCtrlComp>();
				for (auto& e : m_entities) 
				{
					if (e && e->GetName() == "Map")
					{
						pc.RegisterHitEntity(e);
					}
				}
			}

			// ★ここで保留スポーンがあれば一度だけ適用
			if (m_pendingSpawn.has && m_playerEnt->HasComponent<TransformComponent>())
			{
				auto& tf = m_playerEnt->GetComponent<TransformComponent>();
				tf.SetPos(m_pendingSpawn.pos);
				tf.SetRotation(m_pendingSpawn.rot);
				m_pendingSpawn.has = false;
			}
		}

		// めり込み防止用：Map等をカメラのHit対象に登録したい場合
		if (entity->GetName() == "Map" && m_playerCam)
		{
			// Map の見た目に連動したモデル当たり（静的）
			if (!entity->HasComponent<ColliderComponent>())
			{
				entity->AddComponent<ColliderComponent>(std::make_shared<ColliderComponent>());
				entity->GetComponent<ColliderComponent>().Init();
			}
			auto& rc = entity->GetComponent<RenderComponent>();
			auto& cc = entity->GetComponent<ColliderComponent>();
			if (auto md = rc.GetModelData()) cc.RegisterModel("map", md, KdCollider::TypeGround | KdCollider::TypeBump);
			if (auto mw = rc.GetModelWork()) cc.RegisterModel("map", mw, KdCollider::TypeGround | KdCollider::TypeBump);

			// プレイヤーの“当て先”に Map を登録（プレイヤー生成済みなら）
			if (m_playerEnt && m_playerEnt->HasComponent<PlayerCtrlComp>())
			{
				m_playerEnt->GetComponent<PlayerCtrlComp>().RegisterHitEntity(entity);
			}
		}
		if (std::find(m_entities.begin(), m_entities.end(), entity) == m_entities.end())
		{
			m_entities.push_back(entity);
			m_objList.push_back(entity);
		}
	}

	bool pNow = (GetAsyncKeyState('P') & 0x8000);
	if (pNow && !m_prevP) {
		m_playMode = !m_playMode;

		// カメラ切替
		if (m_playerCam) m_playerCam->SetActive(m_playMode);
		if (m_camera)    m_camera->SetActive(!m_playMode);

		// プレイヤー操作のON/OFF
		if (m_playerEnt && m_playerEnt->HasComponent<PlayerCtrlComp>())
		{
			auto& pc = m_playerEnt->GetComponent<PlayerCtrlComp>();
			pc.SetEnabled(m_playMode);
		}

		// プレイ時はフリーカメラ操作を無効化、エディタ時は有効化
		EngineCore::Engine::Instance().m_isCameraControlActive = !m_playMode;
		EngineCore::Engine::Instance().SetMouseGrabbed(m_playMode);

		// ImGui側の表示モード切替（全画面⇔エディタ）
		ImGuiManager::Instance().ToggleMode();
	}
	m_prevP = pNow;

	if (m_playMode && m_playerEnt && m_playerCam && m_playerEnt->HasComponent<PlayerCtrlComp>())
	{
		// CameraBase の m_DegAng.y を使って yaw を渡す
		float camYawDeg = m_playerCam->GetYawDeg(); // 無ければ m_playerCam->GetRotationMatrix() から算出
		auto& pc = m_playerEnt->GetComponent<PlayerCtrlComp>();
		pc.SetCameraYawDeg(camYawDeg);
	}

	MainThreadTask::Instance().Drain();

	if (m_editor)
	{
		m_editor->SetEntityList(m_entities);
	}
}

void GameScene::Init()
{
	GameStageSpec spec;
	spec.mapModelPath = "Asset/Models/Stage/StageMap.gltf";
	spec.objDataPath = "Asset/Data/ObjData/Stage1.json";      // 任意
	spec.playerModelPath = "Asset/Models/Character/Robot/Robot.gltf";     // ← プレイヤーもここで指定
	spec.playerDynamic = true;                                  // 動的なら true
	//spec.spawnJsonPath = "Asset/Data/Spawn/Stage1_Spawn.json";  // 任意
	//spec.playerStatePath = "Asset/Data/Save/PlayerState.json";    // 任意
	//spec.spawnJsonPath = "Asset/Data/Spawn/Stage1_Spawn.json"; // あれば

	StageLoader::LoadStageAsync(spec, this);

	//	エディタ
	m_editor = std::make_shared<EditorManager>();
	m_editor->Init();

	//	フリーカメラ
	m_camera = std::make_shared<EditorCamera>();
	m_camera->Init();
	m_objList.push_back(m_camera);

	//	プレイヤーカメラ
	m_playerCam = std::make_shared<TPSCamera>();
	m_playerCam->Init();
	m_playerCam->SetActive(false); // 初期はフリーカメラ
	AddObject(m_playerCam);
}
