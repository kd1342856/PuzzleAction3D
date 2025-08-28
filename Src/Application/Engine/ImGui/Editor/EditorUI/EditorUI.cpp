#include "EditorUI.h"
#include "../../../Entity/Component/Trans/TransformComponent.h"
#include "../../../Entity/Component/Render/RenderComponent.h"
#include "../../../Entity/Entity/Entity.h"
#include "../EditorManager.h"
#include "../../../Entity/Entity/EntityFactory/EntityFactory.h"
#include "../../../../Scene/SceneManager.h"
#include "../../../System/Thread/MainThreadTask.h"
#include "../../../../GameObject/Camera/CameraBase.h"

void EditorUI::Update(EditorManager& editor)
{
	auto& entityList = editor.GetEntityList();
	
	DrawEditorUI(entityList, editor);
	DrawEntityInspector(entityList, editor);

}
void EditorUI::DrawEditorUI(std::vector<std::shared_ptr<Entity>>& list, EditorManager& editor)
{
	//	Entity List
	ImGui::Begin("Hierarchy");
	ImGui::SeparatorText("Entities");

	if (ImGui::BeginMenu("Add Object"))
	{
		// Player は“選択”メニューに変更
		if (ImGui::MenuItem("Select Player"))
		{
			int idx = editor.FindByName("Player");
			if (idx >= 0) 
			{
				editor.SelectIndex(idx);                 // ヒエラルキー選択をプレイヤーに
			}
		}

		// 他の “Enemy / Object” は必要なら生成OK
		if (ImGui::MenuItem("Block")) 
		{
			auto object = EntityFactory::CreateBlock();
			SceneManager::Instance().AddObject(object);
			editor.PushEntity(object);
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

void EditorUI::DrawEntityInspector(std::vector<std::shared_ptr<Entity>>& list, EditorManager& editor)
{
	// ★ ここから“通常エンティティ”用
	if (m_selectedEntityIndex >= 0 && m_selectedEntityIndex < static_cast<int>(list.size()))
	{
		auto ent = list[m_selectedEntityIndex];
		ImGui::Begin("Inspector");

		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ent->HasComponent<TransformComponent>())
			{
				auto& tf = ent->GetComponent<TransformComponent>();
				Math::Vector3 pos = tf.GetPos();
				Math::Vector3 rot = tf.GetRotation();
				Math::Vector3 scale = tf.GetScale();

				if (ImGui::DragFloat3("Position", &pos.x, 0.1f)) tf.SetPos(pos);
				if (ImGui::DragFloat3("Rotation", &rot.x, 1.0f)) tf.SetRotation(rot);
				if (ImGui::DragFloat3("Scale", &scale.x, 1.0f)) tf.SetScale(scale);
			}
		}

		if (ent->HasComponent<RenderComponent>())
		{
			if (ImGui::CollapsingHeader("Render Setting", ImGuiTreeNodeFlags_DefaultOpen))
			{
				using VF = Entity::VisibilityFlags;
				bool lit = ent->IsVisible(VF::Lit);
				bool unlit = ent->IsVisible(VF::UnLit);
				bool bright = ent->IsVisible(VF::Bright);
				bool shadow = ent->IsVisible(VF::Shadow);

				if (ImGui::Checkbox("DrawLit", &lit))      ent->SetVisibility(VF::Lit, lit);
				if (ImGui::Checkbox("DrawUnLit", &unlit))  ent->SetVisibility(VF::UnLit, unlit);
				if (ImGui::Checkbox("DrawBright", &bright))ent->SetVisibility(VF::Bright, bright);
				if (ImGui::Checkbox("GenerateDepthMapFromLight", &shadow))
					ent->SetVisibility(VF::Shadow, shadow);
			}
		}
		ImGui::End();
	}
}
