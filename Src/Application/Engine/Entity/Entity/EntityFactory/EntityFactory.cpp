#include "EntityFactory.h"
#include "../Entity.h"
#include "../../Component/Trans/TransformComponent.h"
#include "../../Component/Render/RenderComponent.h"
#include "../../Component/Collider/ColliderComponent.h"
#include "../../Component/Controller/Player/PlayerCtrlComp.h"

std::shared_ptr<Entity> EntityFactory::CreatePlayer()
{
	auto entity = std::make_shared<Entity>();
	entity->AddComponent<TransformComponent>(std::make_shared<TransformComponent>());

	auto rc = std::make_shared<RenderComponent>();
	rc->SetModelWork("Asset/Models/Character/Robot/Robot.gltf");
	entity->AddComponent<RenderComponent>(rc);

	auto col = std::make_shared<ColliderComponent>();
	entity->AddComponent<ColliderComponent>(col);

	auto pc = std::make_shared<PlayerCtrlComp>();
	pc->SetEnabled(false); // 初期は無効（エディタモードのため）
	entity->AddComponent<PlayerCtrlComp>(pc);

	entity->SetName("Player");
	entity->Init();

	return entity;
}

std::shared_ptr<Entity> EntityFactory::CreateEnemy()
{
	auto entity = std::make_shared<Entity>();
	entity->AddComponent<TransformComponent>(std::make_shared<TransformComponent>());
	entity->AddComponent<RenderComponent>(std::make_shared<RenderComponent>());
	entity->SetName("Enemy");
	return entity;
}

std::shared_ptr<Entity> EntityFactory::CreateObject()
{
	auto entity = std::make_shared<Entity>();
	entity->AddComponent<TransformComponent>(std::make_shared<TransformComponent>());
	entity->AddComponent<RenderComponent>(std::make_shared<RenderComponent>());
	entity->SetName("Object");
	return entity;
}

std::shared_ptr<Entity> EntityFactory::CreateModelEntity(
	const std::string& name,
	const std::string& modelPath,
	const Math::Vector3& pos,
	const Math::Vector3& rot,
	const Math::Vector3& scl,
	bool dynamic)
{
	auto e = std::make_shared<Entity>();

	// 1) AddComponent
	auto tf = std::make_shared<TransformComponent>();
	tf->SetPos(pos); tf->SetRotation(rot); tf->SetScale(scl);
	e->AddComponent<TransformComponent>(tf);

	auto rc = std::make_shared<RenderComponent>();
	if (!modelPath.empty()) {
		if (dynamic) rc->SetModelWork(modelPath);
		else         rc->SetModelData(modelPath);
	}
	e->AddComponent<RenderComponent>(rc);

	e->SetName(name);

	using VF = Entity::VisibilityFlags;
	e->SetVisibility(VF::Lit, true);   // ライティングありで描く
	e->SetVisibility(VF::UnLit, true);   // ← とりあえず両方ONにしておくと見逃しがない
	e->SetVisibility(VF::Bright, false);
	e->SetVisibility(VF::Shadow, true);

	// 2) 最後に Init（コンポーネントが揃ってから）
	e->Init();

	return e;
}