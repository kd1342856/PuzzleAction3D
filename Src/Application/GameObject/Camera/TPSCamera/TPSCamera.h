#pragma once
#include "../CameraBase.h"
class Entity;

class TPSCamera : public CameraBase
{
public:
	TPSCamera()							{}
	~TPSCamera()			override	{}

	void Init()				override;
	void PostUpdate()		override;

	float GetYawDeg() const { return m_DegAng.y; }

	void SetTargetEntity(const std::shared_ptr<Entity>& e) { m_wpTargetEnt = e; }
private:
	std::weak_ptr<Entity> m_wpTargetEnt;

	float m_minPitch = -30.0f;
	float m_maxPitch = 60.0f;
	Math::Vector3 m_camPos{ 0,3,-10 };
};