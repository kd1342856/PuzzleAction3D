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
#include "../../Engine/ImGui/Editor/EditorScene/EditorScene.h"

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

			// BuildMode / GameMode を必ず所持＆Init
			if (!m_playerEnt->HasComponent<BuildModeComponent>())
			{
				m_playerEnt->AddComponent<BuildModeComponent>(std::make_shared<BuildModeComponent>());
				m_playerEnt->GetComponent<BuildModeComponent>().Init();
			}
			if (!m_playerEnt->HasComponent<GameModeManager>())
			{
				m_playerEnt->AddComponent<GameModeManager>(std::make_shared<GameModeManager>());
				m_playerEnt->GetComponent<GameModeManager>().Init();
			}

			// カメラ注入（TPS=Play / Overhead=Build）
			if (m_playerEnt->HasComponent<GameModeManager>())
			{
				auto& gm = m_playerEnt->GetComponent<GameModeManager>();
				gm.SetCameras(m_playerCam, m_camera);

				// 起動時はTPS(Play)から開始
				gm.SetMode(GameModeManager::Mode::Play);
			}

			// 既に存在しているMap等をヒット対象に登録（任意）
			if (m_playerEnt->HasComponent<PlayerCtrlComp>())
			{
				auto& pc = m_playerEnt->GetComponent<PlayerCtrlComp>();
				for (auto& e : m_entities)
				{
					if (e && e->GetName() == "Map") pc.RegisterHitEntity(e);
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
			if (!entity->HasComponent<ColliderComponent>())
			{
				entity->AddComponent<ColliderComponent>(std::make_shared<ColliderComponent>());
				entity->GetComponent<ColliderComponent>().Init();
			}
			auto& rc = entity->GetComponent<RenderComponent>();
			auto& cc = entity->GetComponent<ColliderComponent>();
			if (auto md = rc.GetModelData()) cc.RegisterModel("map", md, KdCollider::TypeGround | KdCollider::TypeBump);
			if (auto mw = rc.GetModelWork()) cc.RegisterModel("map", mw, KdCollider::TypeGround | KdCollider::TypeBump);

			if (m_playerEnt && m_playerEnt->HasComponent<PlayerCtrlComp>())
				m_playerEnt->GetComponent<PlayerCtrlComp>().RegisterHitEntity(entity);

			if (m_playerEnt && m_playerEnt->HasComponent<BuildModeComponent>())
				m_playerEnt->GetComponent<BuildModeComponent>().RegisterHitEntity(entity);
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
		ImGuiManager::Instance().ToggleMode();

		const bool nowGame = (ImGuiManager::Instance().m_editor->GetMode() == EditorMode::Game);
		if (m_playerEnt && m_playerEnt->HasComponent<GameModeManager>())
		{
			auto& gm = m_playerEnt->GetComponent<GameModeManager>();
			if (nowGame)
			{
				// ゲームに戻ったら必ずTPS(Play)から
				gm.SetMode(GameModeManager::Mode::Play);
			}
			else
			{
				// エディタに入るならカーソル解放（安全側）
				EngineCore::Engine::Instance().SetMouseGrabbed(false);
			}
		}
	}
	m_prevP = pNow;

	// TPS時のみプレイヤーへYawを渡す（任意）
	if (ImGuiManager::Instance().m_editor &&
		!ImGuiManager::Instance().m_editor->IsEditorMode() &&
		m_playerEnt && m_playerCam && m_playerEnt->HasComponent<PlayerCtrlComp>())
	{
		m_playerEnt->GetComponent<PlayerCtrlComp>().SetCameraYawDeg(m_playerCam->GetYawDeg());
	}

	MainThreadTask::Instance().Drain();

	if (auto editor = ImGuiManager::Instance().m_editor)
	{
		editor->SetEntityList(m_entities);
		editor->SetCameras(m_playerCam, m_camera);
	}

	// Overheadカメラの保留適用（ロード直後1回）
	if (m_pendingOverheadCam.has)
	{
		if (auto editor = ImGuiManager::Instance().m_editor)
		{
			if (auto overhead = editor->GetOverheadCamera())
			{
				overhead->SetPosition(m_pendingOverheadCam.pos);
				overhead->SetEulerDeg(m_pendingOverheadCam.rot);
				m_pendingOverheadCam.has = false;
			}
		}
	}
}

void GameScene::Init()
{
	m_editor = std::make_shared<EditorManager>(); m_editor->Init();
	m_camera = std::make_shared<EditorCamera>();  m_camera->Init();
	m_objList.push_back(m_camera);

	m_playerCam = std::make_shared<TPSCamera>();  m_playerCam->Init();
	m_objList.push_back(m_playerCam);


	ImGuiManager::Instance().SetMode(EditorMode::Game);
	if (auto editor = ImGuiManager::Instance().m_editor)
	{
		editor->SetUseTPSCamera(true);
		editor->SetCameras(m_playerCam, m_camera);
		editor->SetEntityList(m_entities);
	}
	if (m_playerCam) m_playerCam->SetActive(true);
	if (m_camera)    m_camera->SetActive(false);

	GameStageSpec spec;
	spec.mapModelPath = "Asset/Models/Stage/Block/Block.gltf";
	spec.objDataPath = "Asset/Data/ObjData/ObjData/ObjData.json";
	spec.playerModelPath = "Asset/Models/Character/Cat/Cat.gltf";
	spec.playerDynamic = true;

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

	if (m_playerCam) m_playerCam->SetActive(true);
	if (m_camera)    m_camera->SetActive(false);

	if (m_playerEnt && m_playerEnt->HasComponent<PlayerCtrlComp>()) 
	{
		m_playerEnt->GetComponent<PlayerCtrlComp>().SetEnabled(true); // ← 初期状態で必ず操作可能に
	}

	StageLoader::LoadStageAsync(spec, this);
}
