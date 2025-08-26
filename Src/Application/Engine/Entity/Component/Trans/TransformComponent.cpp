#include "TransformComponent.h"

void TransformComponent::Update()
{
}

const Math::Matrix& TransformComponent::GetMatrix() const
{
	if (m_dirty)
	{
		using namespace DirectX;
		auto scale = Math::Matrix::CreateScale(m_scale);

		// yaw=y, pitch=x, roll=z（度→ラジアン）
		float yaw = XMConvertToRadians(m_rotation.y);
		float pitch = XMConvertToRadians(m_rotation.x);
		float roll = XMConvertToRadians(m_rotation.z);

		auto rot = Math::Matrix::CreateFromYawPitchRoll(yaw, pitch, roll);
		auto trans = Math::Matrix::CreateTranslation(m_pos);

		m_matrix = scale * rot * trans;
		m_dirty = false;
	}
	return m_matrix;
}
