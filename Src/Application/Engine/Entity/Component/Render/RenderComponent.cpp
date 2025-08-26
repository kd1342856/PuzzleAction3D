#include "RenderComponent.h"
#include "../../Entity/Entity.h"
#include "../Trans/TransformComponent.h"
void RenderComponent::Draw()
{
	auto owner = m_owner.lock();
	if (!owner)return;

	if (!owner->HasComponent<TransformComponent>())return;
	const auto& transform = owner->GetComponent<TransformComponent>();
	const Math::Matrix& world = transform.GetMatrix();
	
	auto& shader = KdShaderManager::Instance().m_StandardShader;

	if (m_modelWork && m_modelWork->IsEnable())
	{
		if (owner->IsVisible(Entity::VisibilityFlags::Lit))
		{
			shader.DrawModel(*m_modelWork, world);
		}
		if (owner->IsVisible(Entity::VisibilityFlags::UnLit))
		{
			shader.DrawModel(*m_modelWork, world);
		}
		if (owner->IsVisible(Entity::VisibilityFlags::Bright))
		{
			shader.DrawModel(*m_modelWork, world);
		}
		if (owner->IsVisible(Entity::VisibilityFlags::Shadow))
		{
			shader.DrawModel(*m_modelWork, world);
		}
	}
	else if (m_modelData)
	{
		if (owner->IsVisible(Entity::VisibilityFlags::Lit))
		{
			shader.DrawModel(*m_modelData, world);
		}
		if (owner->IsVisible(Entity::VisibilityFlags::UnLit))
		{
			shader.DrawModel(*m_modelData, world);
		}
		if (owner->IsVisible(Entity::VisibilityFlags::Bright))
		{
			shader.DrawModel(*m_modelData, world);
		}
		if (owner->IsVisible(Entity::VisibilityFlags::Shadow))
		{
			shader.DrawModel(*m_modelData, world);
		}
	}
}

void RenderComponent::SetModelData(const std::string& filePath)
{
	if (!m_modelData) m_modelData = std::make_shared<KdModelData>();
	if (m_modelData->Load(filePath)) 
	{
		EngineCore::Logger::Log("Engine", "Load");
		m_modelType = ModelType::Static;
	}
	else
	{
		EngineCore::Logger::Log("Engine", "Load Error");
	}
}

void RenderComponent::SetModelWork(const std::string& filePath)
{
	if (!m_modelWork) m_modelWork = std::make_shared<KdModelWork>();
	m_modelWork->SetModelData(filePath); // KdAssets 経由で内部ロード
	m_modelType = ModelType::Dynamic;
}
