#pragma once
#include "../Component.h"
class ColliderComponent : public Component
{
public:
	ColliderComponent() = default;
	virtual ~ColliderComponent() = default;

	void Init()override;
	void Update()override;

	void RegisterSphere(std::string_view name, float radius, UINT type);
	void RegisterAABB(std::string_view name, const Math::Vector3& offset, const Math::Vector3& size, UINT type);
	void RegisterOBB(std::string_view name, const Math::Vector3& offset, const Math::Vector3& size, UINT type);
	void RegisterModel(std::string_view name, const std::shared_ptr<KdModelData>& model, UINT type);
	void RegisterModel(std::string_view name, const std::shared_ptr<KdModelWork>& model, UINT type);
	void RegisterPolygon(std::string_view name, const std::shared_ptr<KdPolygon>& polygon, UINT type);

	// 衝突の有効・無効
	void SetEnable(std::string_view name, bool flag);
	void SetEnableByType(int type, bool flag);
	void SetEnableAll(bool flag);

	bool Intersects(const KdCollider::SphereInfo& shape, std::list<KdCollider::CollisionResult>* pResults = nullptr) const;
	bool Intersects(const KdCollider::BoxInfo& shape, std::list<KdCollider::CollisionResult>* pResults = nullptr) const;
	bool Intersects(const KdCollider::RayInfo& shape, std::list<KdCollider::CollisionResult>* pResults = nullptr) const;

private:
	std::unique_ptr<KdCollider> m_collider;
};

