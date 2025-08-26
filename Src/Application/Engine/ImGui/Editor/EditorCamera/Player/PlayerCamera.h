#pragma once
#include "../../../../../GameObject/Camera/CameraBase.h"
class Entity;

class PlayerCamera : public CameraBase 
{
public:
	void Init() override;
	void PostUpdate() override;

	void SetTargetEntity(const std::shared_ptr<Entity>& e) { m_target = e; }
	void SetActive(bool v) { m_active = v; }

	// 調整用パラメータ
	void SetDistance(float d) { m_distance = d; }
	void SetHeight(float h) { m_height = h; }
	void SetFollowLerp(float a) { m_followLerp = std::clamp(a, 0.0f, 1.0f); }

private:
	std::weak_ptr<Entity> m_target;
	bool   m_active = true;

	float  m_distance = 5.0f;   // 背後距離
	float  m_height = 2.0f;   // 目線の高さ
	float  m_minPitch = -30.0f;
	float  m_maxPitch = 60.0f;
	float  m_followLerp = 0.18f;  // 追従のなめらかさ(0..1)

	// 補助：現在のカメラ位置（追従Lerp用）
	Math::Vector3 m_camPos = { 0, 3, -10 };
};