#include "GameSceneWiring.h"

#include "../../../Engine/Entity/Entity/Entity.h"
#include "../../../Engine/Entity/Component/Trans/TransformComponent.h"
#include "../../../Engine/Entity/Component/Render/RenderComponent.h"
#include "../../../Engine/Entity/Component/Collider/ColliderComponent.h"
#include "../../../Engine/Entity/Component/Controller/Player/PlayerCtrlComp.h"
#include "../../../Engine/Entity/Component/Game/GameModeManager.h"
#include "../../../Engine/Entity/Component/Game/Build/BuildmodeComponent.h"
#include "../../../GameObject/Camera/CameraBase.h"
#include "../../../GameObject/Camera/TPSCamera/TPSCamera.h"
#include "../../../Engine/Engine.h"

namespace
{
	template <class T>
	T& EnsureComp(const std::shared_ptr<Entity>& entity)
	{
		if (!entity->HasComponent<T>())
		{
			entity->AddComponent<T>(std::make_shared<T>());
			entity->GetComponent<T>().Init();
		}
		return entity->GetComponent<T>();
	}
	void RegisterModelCollider(const std::shared_ptr<Entity>& eCollideFrom,
		ColliderComponent& col,
		UINT types)
	{
		if (!eCollideFrom->HasComponent<RenderComponent>())return;
		auto& render = eCollideFrom->GetComponent<RenderComponent>();

		bool registered = false;
		if (auto model = render.GetModelData())
		{
			col.RegisterModel("model", model, types);
			registered = true;
		}
		if (!registered)
		{
			if (auto modelWork = render.GetModelWork())
			{
				col.RegisterModel("model", modelWork, types);
				registered = true;
			}
		}
	}
}
void SceneWire::WirePlayer(const std::shared_ptr<Entity>& player, const std::shared_ptr<TPSCamera>& tpsCam, const std::shared_ptr<CameraBase>& overheadCam)
{
	auto& Pctrl = EnsureComp<PlayerCtrlComp>(player);
	auto& build = EnsureComp<BuildModeComponent>(player);
	auto& mode = EnsureComp<GameModeManager>(player);

	//	BuildModeにビルド用カメラを注入
	if (overheadCam)
	{
		build.SetActiveCamera(overheadCam);
	}

	//	GameModeManagerにカメラを登録
	if (tpsCam || overheadCam)
	{
		mode.SetCameras(tpsCam, overheadCam);
	}

	//	初期モードはプレイ
	mode.SetMode(GameModeManager::Mode::Play);
}

void SceneWire::WireMap(const std::shared_ptr<Entity>& map, const std::vector<std::shared_ptr<Entity>>& extraHitTargets)
{
	auto& mapCol = EnsureComp<ColliderComponent>(map);
	RegisterModelCollider(map, mapCol, KdCollider::TypeGround | KdCollider::TypeBump);

	for (auto& targets : extraHitTargets)
	{
		if (!targets)continue;

		if (targets->HasComponent<PlayerCtrlComp>())
		{
			targets->GetComponent<PlayerCtrlComp>().RegisterHitEntity(map);
		}
		if (targets->HasComponent<BuildModeComponent>())
		{
			targets->GetComponent<BuildModeComponent>().RegisterHitEntity(map);
		}
	}
}

void SceneWire::ApplyModeForPlayer(const std::shared_ptr<Entity>& player, bool isGameMode)
{
	if (!player || !player->HasComponent<GameModeManager>())return;
	auto& gamemode = player->GetComponent<GameModeManager>();

	gamemode.SetMode(isGameMode ? GameModeManager::Mode::Play : GameModeManager::Mode::Build);

	if (!isGameMode)
	{
		EngineCore::Engine::Instance().SetMouseGrabbed(false);
	}
}

void SceneWire::SyncPlayerYawFromTPS(const std::shared_ptr<Entity>& player, const std::shared_ptr<CameraBase>& tpsCam)
{
	if (!player || !tpsCam)return;
	if (!player->HasComponent<PlayerCtrlComp>())return;

	if (auto tps = std::dynamic_pointer_cast<TPSCamera>(tpsCam))
	{
		player->GetComponent<PlayerCtrlComp>().SetCameraYawDeg(tps->GetYawDeg());
	}
}
