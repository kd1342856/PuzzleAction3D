#pragma once
#include "../../Component.h"

class PlayerCtrlComp : public Component {
public:
	void Init() override;
	void Update() override;
	void PostUpdate() override;

	void SetEnabled(bool e) { m_enabled = e; }
	bool IsEnabled() const { return m_enabled; }
	
	//　カメラ
	void SetCameraYawDeg(float y) { m_camYawDeg = y; }

	//	移動
	void SetMoveSpeed(float s) { m_moveSpeed = s; }
	void SetJumpSpeed(float v) { m_jumpSpeed = v; }
	void SetTurnLerp(float a) { m_turnLerp = std::clamp(a, 0.0f, 1.0f); }

	void RegisterHitEntity(const std::shared_ptr<Entity>& e) { m_hitEntities.push_back(e); }

	void SetSprintMultiplier(float m) { m_sprintMultiplier = std::max(1.0f, m); }
	float GetSprintMultiplier() const { return m_sprintMultiplier; }
private:

	std::vector<std::weak_ptr<Entity>> m_hitEntities;

	bool  m_enabled = false;

	//　移動
	float m_moveSpeed = 7.5f;
	float m_turnLerp = 0.9f;
	float m_camYawDeg = 0.0f;
	float m_sprintMultiplier = 1.8f;  // ダッシュ倍率
	float m_currentSpeed = 0.0f;  // 今フレームの実効速度
	// ジャンプ
	float m_jumpSpeed = 8.8f;   // 初速(↑)
	bool  m_prevSpace = false;  // 縁判定用

	// 重力／接地
	float m_gravity = -15.0f;
	float m_vy = 0.0f;
	float m_stepHeight = 0.25f;   // 登れる段差（20cm）
	float m_groundSnap = 0.08f;   // 接地吸着（6cmくらい）
	float probeExtra = 0.50f;   // 余裕（50cm）

	// 体スフィア
	float m_bodyRadius = 0.9f;
	Math::Vector3 m_bodyOffset = { 0, 1.3f, 0 }; // 原点が足元なら腰高に

	// 横押し戻しの安定化
	int   m_bumpIters = 3;          // 反復回数

	// 接地状態
	bool  m_grounded = false;

	Math::Vector3 m_moveDirWorld = { 0,0,0 }; // Updateで作る
	bool m_wantsJump = false;               // Updateで立てる


	// 判定
	bool GroundCheck(const Math::Vector3& feetPos, float& outY);
	void ResolveBump(Math::Vector3& pos);  // ★追加：球オーバーラップ解消


};
