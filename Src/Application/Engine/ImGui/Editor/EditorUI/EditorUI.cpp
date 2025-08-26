#include "EditorUI.h"
#include "../../../Entity/Component/Trans/TransformComponent.h"
#include "../../../Entity/Component/Render/RenderComponent.h"
#include "../../../Entity/Entity/Entity.h"
#include "../EditorManager.h"
#include "../../../Entity/Entity/EntityFactory/EntityFactory.h"
#include "../../../../Scene/SceneManager.h"
#include "../../../System/Thread/MainThreadTask.h"

void EditorUI::Update(EditorManager& editor)
{
	auto& entityList = editor.GetEntityList();
	
	DrawEditorUI(entityList);
	DrawEntityInspector(entityList);

}
void EditorUI::DrawEditorUI(std::vector<std::shared_ptr<Entity>>& list)
{
	//	Entity List
	ImGui::Begin("Hierarchy");
	if (ImGui::BeginMenu("Add Object"))
	{
		if (ImGui::MenuItem("Player"))
		{
			// 1) 軽量プレイヤー（モデル未設定）を先に出す
			auto player = EntityFactory::CreatePlayer();      // Transform + RenderComponent あり
			player->SetName("Player (Loading...)");
			SceneManager::Instance().AddObject(player);
			list.push_back(player);

			// 2) 裏でモデルロード（重い処理）
			std::string path = "Asset/Models/Character/Robot/Robot.gltf";
			EngineCore::TaskManager::Submit([wp = std::weak_ptr<Entity>(player), path]() {
				// a) ファイル読み込み
				auto md = std::make_shared<KdModelData>();
				if (!md->Load(path)) return;  // 失敗なら何もしない

				// 3) メインスレッドで安全に差し替える
				MainThreadTask::Instance().Enqueue([wp, md]() {
					if (auto sp = wp.lock()) {
						// 可視フラグ（初期ONにしておくと確実）
						using VF = Entity::VisibilityFlags;
						sp->SetVisibility(VF::Lit, true);
						sp->SetVisibility(VF::UnLit, true);
						sp->SetVisibility(VF::Shadow, true);

						auto& rc = sp->GetComponent<RenderComponent>();
						// 静的なら：
						//rc.SetModelData(md);
						// アニメ等の動的にしたいなら：
						rc.SetModelWork(md);

						sp->SetName("Player");
					}
					});
				});
		}
		if (ImGui::MenuItem("Enemy"))
		{
			auto enemy = EntityFactory::CreateEnemy();
			list.push_back(enemy);
		}
		if (ImGui::MenuItem("Object"))
		{
			auto object = EntityFactory::CreateObject();
			list.push_back(object);
		}
		ImGui::EndMenu();
	}
	for (size_t i = 0; i < list.size(); ++i)
	{
		std::string label = list[i]->GetName() + "##" + std::to_string(i);
		if (ImGui::Selectable(label.c_str(), m_selectedEntityIndex == static_cast<int>(i)))
		{
			m_selectedEntityIndex = static_cast<int>(i);
		}
	}
	ImGui::End();
}

void EditorUI::DrawEntityInspector(std::vector<std::shared_ptr<Entity>>& list)
{
	if (m_selectedEntityIndex >= 0 && m_selectedEntityIndex < static_cast<int>(list.size()))
	{
		auto ent = (list)[m_selectedEntityIndex];
		ImGui::Begin("Inspector");
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{//	Transform編集
			if (ent->HasComponent<TransformComponent>())
			{
				auto& tf = ent->GetComponent<TransformComponent>();
				Math::Vector3 pos	 = tf.GetPos();
				Math::Vector3 rot	 = tf.GetRotation();
				Math::Vector3 scale	 = tf.GetScale();

				if (ImGui::DragFloat3("Position",	&pos.x, 0.1f)) tf.SetPos(pos);
				if (ImGui::DragFloat3("Rotation",	&rot.x, 1.0f)) tf.SetRotation(rot);
				if (ImGui::DragFloat3("Scale",		&scale.x, 1.0f)) tf.SetScale(scale);
			}
		}

		// 表示切り替え
		if (ent->HasComponent<RenderComponent>())
		{
			if (ImGui::CollapsingHeader("Render Setting", ImGuiTreeNodeFlags_DefaultOpen))
			{
				using VF = Entity::VisibilityFlags;

				bool lit	= ent->IsVisible(VF::Lit);
				bool unlit	= ent->IsVisible(VF::UnLit);
				bool bright = ent->IsVisible(VF::Bright);
				bool shadow = ent->IsVisible(VF::Shadow);

				if (ImGui::Checkbox("DrawLit", &lit))							ent->SetVisibility(VF::Lit, lit);
				if (ImGui::Checkbox("DrawUnLit", &unlit))						ent->SetVisibility(VF::UnLit, unlit);
				if (ImGui::Checkbox("DrawBright", &bright))						ent->SetVisibility(VF::Bright, bright);
				if (ImGui::Checkbox("GenerateDepthMapFromLight", &shadow))		ent->SetVisibility(VF::Shadow, shadow);
			}
		}
		ImGui::End();
	}
}
