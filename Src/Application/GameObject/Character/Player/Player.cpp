#include "Player.h"
#include "../../../Engine/Entity/Component/Render/RenderComponent.h"
void Player::Init()
{
	CharacterBase::Init();
	m_render->SetModelWork("Asset/Models/Character/Robot/Robot.gltf");
}
