#pragma once
#include "../Component/Component.h"
template<typename T>
inline void Entity::AddComponent(std::shared_ptr<Component> comp)
{
	static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");
	comp->SetOwner(std::static_pointer_cast<Entity>(shared_from_this()));
	m_components[typeid(T)] = comp;
}

template<typename T>
inline bool Entity::HasComponent() const
{
	return m_components.find(typeid(T)) != m_components.end();
}

template<typename T>
inline T& Entity::GetComponent()
{
	auto it = m_components.find(typeid(T));
	if (it == m_components.end()) {
		throw std::runtime_error("Component not found");
	}
	auto compPtr = std::dynamic_pointer_cast<T>(it->second);
	if (!compPtr) {
		throw std::runtime_error("Component type mismatch");
	}
	return *compPtr;
}