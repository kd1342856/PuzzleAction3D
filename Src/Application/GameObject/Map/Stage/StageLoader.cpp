#include "StageLoader.h"
#include "../../../Engine/System/Thread/LoadedEntityQueue.h"
#include "../../../Engine/System/Thread/MainThreadTask.h"
#include "../../../Engine/System/TaskManager.h"
#include "../../../Engine/Entity/Entity/Entity.h"
#include "../../../Engine/Entity/Entity/EntityFactory/EntityFactory.h"
#include "../../../Engine/Entity/Component/Render/RenderComponent.h"
#include "../../../Engine/Entity/Component/Trans/TransformComponent.h"
#include "../../../Engine/Data/ObjData.h" 
#include "../../../Scene/GameScene/GameScene.h"
#include "../../../Engine/Entity/Component/Collider/ColliderComponent.h"

using namespace EntityFactory;

namespace 
{
	// 必要ならここで spawn.json を読む（ObjData 1件だけを期待）
	inline bool TryLoadSpawn(const std::string& path, Math::Vector3& outPos, Math::Vector3& outRot)
	{
		if (path.empty()) return false;
		ObjectData io;
		auto list = io.LoadJson(path);
		for (auto& d : list) 
		{
			if (d.name == "PlayerSpawn") 
			{ 
				outPos = d.pos; outRot = d.rot; return true;
			}
		}
		return false;
	}

	struct PlayerState 
	{
		Math::Vector3 pos{ 0,0,0 }, rot{ 0,0,0 }, scale{ 1,1,1 };
	};
	inline bool TryLoadPlayerState(const std::string& path, PlayerState& out)
	{
		if (path.empty()) return false;
		std::ifstream ifs(path);
		if (!ifs) return false;
		nlohmann::json j; ifs >> j;
		auto p = j.value("pos", std::vector<float>{0, 0, 0});
		auto r = j.value("rot", std::vector<float>{0, 0, 0});
		auto s = j.value("scale", std::vector<float>{1, 1, 1});
		out.pos = { p[0],p[1],p[2] }; out.rot = { r[0],r[1],r[2] }; out.scale = { s[0],s[1],s[2] };
		return true;
	}
}

void StageLoader::LoadStageAsync(const GameStageSpec& spec, GameScene* scene)
{
	EngineCore::TaskManager::Submit([spec, scene]()
		{
			// 1) Map
			if (!spec.mapModelPath.empty()) 
			{
				auto map = EntityFactory::CreateModelEntity("Map", spec.mapModelPath);
				LoadedEntityQueue::Instance().Push(map);
			}

			// 2) （任意）ObjDataレイアウトからシーンオブジェクトを復元したい場合
			if (!spec.objDataPath.empty()) 
			{
				ObjectData io;
				auto entities = io.LoadEntityList(spec.objDataPath);
				for (auto& e : entities) 
				{
					LoadedEntityQueue::Instance().Push(e);
				}
			}

			if (!spec.objDataPath.empty()) {
				ObjectData io;
				auto entities = io.LoadEntityList(spec.objDataPath);
				for (auto& e : entities) {
					if (e && e->GetName() == "Block") {
						if (!e->HasComponent<ColliderComponent>()) {
							auto cc = std::make_shared<ColliderComponent>();
							cc->Init();
							e->AddComponent<ColliderComponent>(cc);

							auto& rc = e->GetComponent<RenderComponent>();
							if (auto md = rc.GetModelData())
								cc->RegisterModel("block", md, KdCollider::TypeGround | KdCollider::TypeBump);
							else if (auto mw = rc.GetModelWork())
								cc->RegisterModel("block", mw, KdCollider::TypeGround | KdCollider::TypeBump);
						}
					}
					LoadedEntityQueue::Instance().Push(e);
				}
			}

			// 3) Player（必ず1体）
			auto player = EntityFactory::CreatePlayer();
			player->SetName("Player");
			if (!spec.playerModelPath.empty()) 
			{
				auto& rc = player->GetComponent<RenderComponent>();
				if (spec.playerDynamic) rc.SetModelWork(spec.playerModelPath);
				else                    rc.SetModelData(spec.playerModelPath);
			}
			LoadedEntityQueue::Instance().Push(player);

			// 4) Spawn / PlayerState をメインスレッドで適用（Transformは安全側で）
			Math::Vector3 spawnPos{ 0,0,0 }, spawnRot{ 0,0,0 };
			const bool hasSpawn = TryLoadSpawn(spec.spawnJsonPath, spawnPos, spawnRot);

			PlayerState state;
			const bool hasState = TryLoadPlayerState(spec.playerStatePath, state);

			if (scene) 
			{
				MainThreadTask::Instance().Enqueue([scene, hasSpawn, spawnPos, spawnRot, hasState, state]()
					{
						if (hasSpawn) scene->ApplyPlayerSpawn(spawnPos, spawnRot);
						if (hasState) 
						{
							// ステートで上書き（必要ならスポーンより後に）
							auto* ent = scene->m_playerEnt.get(); // friend or public APIで
							if (ent && ent->HasComponent<TransformComponent>()) 
							{
								auto& tf = ent->GetComponent<TransformComponent>();
								tf.SetPos(state.pos); tf.SetRotation(state.rot); tf.SetScale(state.scale);
							}
						}
					});
			}
		});
}