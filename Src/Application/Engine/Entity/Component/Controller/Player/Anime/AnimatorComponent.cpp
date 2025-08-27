#include "AnimatorComponent.h"
#include "../../../Render/RenderComponent.h"
#include "../../../../Entity/Entity.h"

void AnimatorComponent::Init()
{
	auto owner = m_owner.lock();
	if (!owner || !owner->HasComponent<RenderComponent>()) return;

	auto& rc = owner->GetComponent<RenderComponent>();
	m_model = rc.GetModelWork();             // ※動的モデル（スキン）前提
	if (!m_model) return;

	if (!m_anim) m_anim = std::make_shared<KdAnimator>();

	// 初期アニメ（存在すれば）
	//auto idle = m_model->GetAnimation("Walk");
	//if (idle) 
	{
		//m_anim->SetAnimation(idle, true);
		m_current = "";
		m_loop = true;
		m_rate = 1.0f;
	}
}

bool AnimatorComponent::Play(const std::string& name, bool loop, float rate)
{
	if (!m_model) return false;

	auto clip = m_model->GetAnimation(name.c_str());

	if (!clip) return false;

	// 同一状態ならスキップ（速度だけ変える場合はここ調整）
	if (m_current == name && m_loop == loop) 
	{
		m_rate = rate;
		return true;
	}

	if (!m_anim) m_anim = std::make_shared<KdAnimator>();
	m_anim->SetAnimation(clip, loop);
	m_current = name;
	m_loop = loop;
	m_rate = rate;
	return true;
} 

void AnimatorComponent::Update() 
{
	if (!m_model || !m_anim) return;

	// KdAnimator::AdvanceTime は「フレーム量」を加算する設計。

	float dt = EngineCore::Time::DeltaTime();
	if (dt <= 0.0f || dt > 0.1f) dt = 1.0f / 60.0f;

	m_anim->AdvanceTime(m_model->WorkNodes(), m_rate * dt);
	m_model->CalcNodeMatrices();
}