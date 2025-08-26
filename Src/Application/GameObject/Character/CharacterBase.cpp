#include "CharacterBase.h"
#include "../../Engine/Entity/Component/Render/RenderComponent.h"
void CharacterBase::Init()
{
	m_render = std::make_shared<RenderComponent>();
	AddComponent<RenderComponent>(m_render);
}

void CharacterBase::Draw()
{
	if (!m_render)return;
	if (!IsVisible(VisibilityFlags::Lit) &&
		!IsVisible(VisibilityFlags::UnLit) &&
		!IsVisible(VisibilityFlags::Bright) &&
		!IsVisible(VisibilityFlags::Shadow))
	{
		return;
	}

	Entity::Draw();
}
