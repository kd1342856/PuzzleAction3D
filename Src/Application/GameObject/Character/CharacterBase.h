#pragma once
#include "../../Engine/Entity/Entity/Entity.h"
enum class DrawPass;
class RenderComponent;
class CharacterBase: public Entity
{
public:
	CharacterBase() {}
	virtual ~CharacterBase() {}

	void Init()override;
	void Draw()override;

protected:
	float m_speed = 0.0f;
	std::shared_ptr<RenderComponent> m_render;
};