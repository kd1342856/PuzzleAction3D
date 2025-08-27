#pragma once
#include "../../../Component.h"

class RenderComponent;

class AnimatorComponent : public Component
{
public:
	void Init() override;
	void Update() override;

	// アニメ再生API
	bool Play(const std::string& name, bool loop = true, float rate = 1.0f);

	// 補助
	bool IsPlaying(const std::string& name) const { return (m_current == name); }
	void SetRate(float r) { m_rate = r; }

private:
	std::shared_ptr<KdModelWork> m_model;
	std::shared_ptr<KdAnimator> m_anim;

	std::string m_current;
	bool   m_loop = true;
	float  m_rate = 1.0f;
};