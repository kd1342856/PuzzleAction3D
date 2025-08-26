#include "ColliderComponent.h"
#include "../../Entity/Entity.h"
#include "../Trans/TransformComponent.h"

void ColliderComponent::Init()
{
	if (!m_collider)
	{
		m_collider = std::make_unique<KdCollider>();
	}
}

void ColliderComponent::Update()
{
}

void ColliderComponent::RegisterSphere(std::string_view name, float radius, UINT type)
{
	auto owner = GetOwner();
	if (!owner || !owner->HasComponent<TransformComponent>())return;

	Math::Vector3 pos = { 0,0,0 };
	m_collider->RegisterCollisionShape(name, pos, radius, type);
}

void ColliderComponent::RegisterAABB(std::string_view name, const Math::Vector3& offset, const Math::Vector3& size, UINT type)
{
	auto owner = GetOwner();
	if (!owner || !owner->HasComponent<TransformComponent>()) return;

	const auto& transform = owner->GetComponent<TransformComponent>();
	Math::Matrix matrix = transform.GetMatrix();
	auto shape = std::make_unique<KdBoxCollision>(type, matrix, offset, size, false);
	m_collider->RegisterCollisionShape(name, std::move(shape));
}

void ColliderComponent::RegisterOBB(std::string_view name, const Math::Vector3& offset, const Math::Vector3& size, UINT type)
{
	auto owner = GetOwner();
	if (!owner || !owner->HasComponent<TransformComponent>()) return;

	const auto& transform = owner->GetComponent<TransformComponent>();
	Math::Matrix matrix = transform.GetMatrix();
	auto shape = std::make_unique<KdBoxCollision>(type, matrix, offset, size, true);
	m_collider->RegisterCollisionShape(name,std::move(shape));
}

void ColliderComponent::RegisterModel(std::string_view name, const std::shared_ptr<KdModelData>& model, UINT type)
{
	m_collider->RegisterCollisionShape(name, model, type);
}

void ColliderComponent::RegisterModel(std::string_view name, const std::shared_ptr<KdModelWork>& model, UINT type)
{
	m_collider->RegisterCollisionShape(name, model, type);
}

void ColliderComponent::RegisterPolygon(std::string_view name, const std::shared_ptr<KdPolygon>& polygon, UINT type)
{
	m_collider->RegisterCollisionShape(name, polygon, type);
}

void ColliderComponent::SetEnable(std::string_view name, bool flag)
{
	m_collider->SetEnable(name, flag);
}

void ColliderComponent::SetEnableByType(int type, bool flag)
{
	m_collider->SetEnable(type, flag);
}

void ColliderComponent::SetEnableAll(bool flag)
{
	m_collider->SetEnableAll(flag);
}

bool ColliderComponent::Intersects(const KdCollider::SphereInfo& shape, std::list<KdCollider::CollisionResult>* pResults) const
{
	auto owner = GetOwner();
	if (!owner || !owner->HasComponent<TransformComponent>())return false;

	const auto& transform = owner->GetComponent<TransformComponent>();
	return m_collider->Intersects(shape, transform.GetMatrix(), pResults);
}

bool ColliderComponent::Intersects(const KdCollider::BoxInfo& shape, std::list<KdCollider::CollisionResult>* pResults) const
{
	auto owner = GetOwner();
	if (!owner || !owner->HasComponent<TransformComponent>())return false;

	const auto& transform = owner->GetComponent<TransformComponent>();
	return m_collider->Intersects(shape, transform.GetMatrix(), pResults);
}

bool ColliderComponent::Intersects(const KdCollider::RayInfo& shape, std::list<KdCollider::CollisionResult>* pResults) const
{
	auto owner = GetOwner();
	if (!owner || !owner->HasComponent<TransformComponent>())return false;

	const auto& transform = owner->GetComponent<TransformComponent>();
	return m_collider->Intersects(shape, transform.GetMatrix(), pResults);
}

