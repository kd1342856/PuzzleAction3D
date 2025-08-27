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
#include "../../Engine/Entity/Component/Game/Build/BuildModeComponent.h"
#include "../../Engine/Entity/Component/Game/GameModeManager.h"

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

			// ★ BuildModeComponent を付与＆Init
			if (!m_playerEnt->HasComponent<BuildModeComponent>()) 
			{
				m_playerEnt->AddComponent<BuildModeComponent>(std::make_shared<BuildModeComponent>());
				m_playerEnt->GetComponent<BuildModeComponent>().Init();
			}

			// ★ GameModeManager を付与＆Init（初期モードはBuildでもPlayでもOK）
			if (!m_playerEnt->HasComponent<GameModeManager>()) 
			{
				m_playerEnt->AddComponent<GameModeManager>(std::make_shared<GameModeManager>());
				// デフォルトをBuildにしたいならここで SetMode(Mode::Build) でもOK
				m_playerEnt->GetComponent<GameModeManager>().Init();
			}

			if (m_playerEnt && m_playerEnt->HasComponent<GameModeManager>()) {
				auto& gm = m_playerEnt->GetComponent<GameModeManager>();
				bool isBuild = (gm.GetMode() == GameModeManager::Mode::Build);

				// カメラ切替（俯瞰＝EditorCamera、プレイ＝TPS）
				if (m_playerCam) m_playerCam->SetActive(!isBuild);
				if (m_camera)    m_camera->SetActive(isBuild);

				// マウスの捕捉切替（ビルドはフリー）
				EngineCore::Engine::Instance().SetMouseGrabbed(!isBuild);

				// BuildModeComponent の有効化と使用カメラを注入
				if (m_playerEnt->HasComponent<BuildModeComponent>()) {
					auto& build = m_playerEnt->GetComponent<BuildModeComponent>();
					build.SetEnabled(isBuild);
					if (isBuild) {
						build.SetActiveCamera(m_camera); // 俯瞰カメラを使う
					}
				}

				// PlayerCtrl は逆に
				if (m_playerEnt->HasComponent<PlayerCtrlComp>()) {
					m_playerEnt->GetComponent<PlayerCtrlComp>().SetEnabled(!isBuild);
				}
			}

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

			if (m_buildEnt && m_buildEnt->HasComponent<BuildModeComponent>())
			{
				m_buildEnt->GetComponent<BuildModeComponent>().RegisterHitEntity(entity);
			}
		}
		if (std::find(m_entities.begin(), m_entities.end(), entity) == m_entities.end())
		{
			m_entities.push_back(entity);
			m_objList.push_back(entity);
		}
	}

	bool pNow = (GetAsyncKeyState('P') & 0x8000);
	if (pNow && !m_prevP) 
	{
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

		if (m_buildEnt && m_buildEnt->HasComponent<BuildModeComponent>())
		{
			m_buildEnt->GetComponent<BuildModeComponent>().SetEnabled(!m_playMode);
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
	spec.mapModelPath = "Asset/Models/Stage/Block/Block.gltf";
	spec.objDataPath = "Asset/Data/ObjData/Stage1.json";      // 任意
	spec.playerModelPath = "Asset/Models/Character/Cat/Cat.gltf";     // ← プレイヤーもここで指定
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

	m_buildEnt = std::make_shared<Entity>();
	m_buildEnt->SetName("BuildSystem");
	m_buildEnt->AddComponent<TransformComponent>(std::make_shared<TransformComponent>());
	m_buildEnt->AddComponent<BuildModeComponent>(std::make_shared<BuildModeComponent>());

	// 初期はエディタ（＝Build有効）想定。Play開始なら後で切り替えられる
	if (m_buildEnt->HasComponent<BuildModeComponent>()) 
	{
		m_buildEnt->GetComponent<BuildModeComponent>().SetEnabled(true);
		// ブロックモデルの差し替えが必要ならここで：m_buildEnt->GetComponent<BuildModeComponent>().SetBlockModelPath("Asset/Models/Block/Cube.gltf");
	}

	// エンティティ配列へ登録（あなたのエンジンの巡回対象）
	m_entities.push_back(m_buildEnt);
	m_objList.push_back(m_buildEnt);
}
