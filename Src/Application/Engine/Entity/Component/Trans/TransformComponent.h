#pragma once
#include "../Component.h"
class TransformComponent : public Component
{
public:
	TransformComponent()
		: m_pos(0, 0, 0), m_rotation(0, 0, 0), m_scale(1, 1, 1), m_dirty(true) {
	}

	void Update()override;

	void SetPos(const Math::Vector3& pos) { m_pos = pos; m_dirty = true; }
	void SetRotation(const Math::Vector3& rot) { m_rotation = rot; m_dirty = true; }
	void SetScale(const Math::Vector3& scale) { m_scale = scale;  m_dirty = true; }

	const Math::Vector3& GetPos() const { return m_pos; }
	const Math::Vector3& GetRotation() const { return m_rotation; }
	const Math::Vector3& GetScale() const { return m_scale; }

	// ワールド行列を必要なときだけ再計算
	const Math::Matrix& GetMatrix()const;

private:
	Math::Vector3 m_pos;
	Math::Vector3 m_rotation;
	Math::Vector3 m_scale;

	mutable Math::Matrix m_matrix; // キャッシュ
	mutable bool m_dirty;          // 再計算フラグ
};
