#include "ObjData.h"
#include "Scene/SceneData.h"
#include "../Entity/Entity/Entity.h"
#include "../Entity/Component/Trans/TransformComponent.h"
#include "../Entity/Component/Render/RenderComponent.h"
void ObjectData::SaveScene(const SceneData& scene, const std::string& filePath)
{
	std::ofstream ofs(filePath);
	if (!ofs) return;
	ofs << scene.ToJson().dump(4);
}
SceneData ObjectData::LoadScene(const std::string& filePath)
{
	std::ifstream ifs(filePath);
	if (!ifs) return SceneData{};
	nlohmann::json j; ifs >> j;
	return SceneData::FromJson(j);
}
std::vector<ObjData> ObjectData::ConvertToDataList(const std::vector<std::shared_ptr<Entity>>& entityList)
{
	std::vector<ObjData> result;

	for (const auto& ent : entityList)
	{
		if (!ent) continue;
		if (ent->GetName() == "Player") continue;                 // Player は objects に入れない
		if (!ent->HasComponent<TransformComponent>()) continue;

		const auto& tf = ent->GetComponent<TransformComponent>();

		ObjData data;
		data.name = ent->GetName();
		data.pos = tf.GetPos();
		data.rot = tf.GetRotation();
		data.scale = tf.GetScale();

		using VF = Entity::VisibilityFlags;
		data.isLit = ent->IsVisible(VF::Lit);
		data.isUnLit = ent->IsVisible(VF::UnLit);
		data.isBright = ent->IsVisible(VF::Bright);
		data.isShadow = ent->IsVisible(VF::Shadow);

		if (ent->HasComponent<RenderComponent>())
		{
			auto& rc = ent->GetComponent<RenderComponent>();
			data.modelPath = rc.GetModelPath();   // ★ ここで保存
			data.isDynamic = rc.IsDynamic();
		}

		result.push_back(std::move(data));
	}
	return result;
}

void ObjectData::SaveObj(const std::vector<ObjData>& objects, const std::string& filePath)
{
	json j;
	for (const auto& obj : objects)
	{
		j.push_back(obj.ToJson());
	}
	std::ofstream ofs(filePath);
	if (ofs)
	{
		ofs << j.dump(4);
	}
}

std::vector<ObjData> ObjectData::LoadJson(const std::string& filePath)
{
	std::vector<ObjData> result;
	std::ifstream ifs(filePath);

	if (!ifs)return result;

	json j;
	ifs >> j;

	for (auto& elem : j)
	{
		result.push_back(ObjData::FromJson(elem));
	}
	return result;
}

std::vector<std::shared_ptr<Entity>> ObjectData::LoadEntityList(const std::string& filePath)
{


	std::vector<std::shared_ptr<Entity>> entities;

	std::ifstream ifs(filePath);
	if (!ifs) return entities;

	json j;
	ifs >> j;

	for (auto& elem : j)
	{
		ObjData data = ObjData::FromJson(elem);
		if (data.name == "__OverheadCamera")
		{
			continue; // ← カメラはエンティティにしない
		}
		auto ent = std::make_shared<Entity>();

		// Transform
		auto transform = std::make_shared<TransformComponent>();
		transform->SetPos(data.pos);
		transform->SetRotation(data.rot);
		transform->SetScale(data.scale);
		ent->AddComponent<TransformComponent>(transform);

		// Render
		auto render = std::make_shared<RenderComponent>();
		if (!data.modelPath.empty())
		{
			if (data.isDynamic) render->SetModelWork(data.modelPath);
			else                render->SetModelData(data.modelPath);
		}
		ent->AddComponent<RenderComponent>(render);

		// Meta
		ent->SetName(data.name);
		using VF = Entity::VisibilityFlags;
		ent->SetVisibility(VF::Lit, data.isLit);
		ent->SetVisibility(VF::UnLit, data.isUnLit);
		ent->SetVisibility(VF::Bright, data.isBright);
		ent->SetVisibility(VF::Shadow, data.isShadow);

		ent->Init(); // ★ コンポーネント揃ってからInit

		entities.push_back(std::move(ent));
	}
	return entities;
}
