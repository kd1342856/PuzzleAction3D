#pragma once
class Entity;
enum class DrawPass
{
	Lit,
	UnLit,
	Bright,
	Shadow,
	Effect,
	Sprite
};
class Component
{
public:

	virtual ~Component(){}
	virtual void Init(){}
	virtual void Update(){}
	virtual void Draw(){}

	void SetOwner(std::shared_ptr<Entity>owner) { m_owner = owner; }
protected:
	std::shared_ptr<Entity> GetOwner()const { return m_owner.lock(); }
	std::weak_ptr<Entity> m_owner;
};
