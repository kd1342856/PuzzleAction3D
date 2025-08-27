#pragma once
#include "../Component.h"

class PlayerCtrlComp;
class BuildModeComponent;

class GameModeManager : public Component 
{
public:
	enum class Mode { Build, Play };

	void Init() override;
	void Update() override;

	void SetMode(Mode m);
	Mode GetMode() const { return m_mode; }

	// 外部から関連を注入（自動検出できない場合に使用）
	void SetPlayerCtrl(const std::weak_ptr<PlayerCtrlComp>& w) { m_wpPlayer = w; }
	void SetBuilder(const std::weak_ptr<BuildModeComponent>& w) { m_wpBuilder = w; }

private:
	Mode m_mode = Mode::Build;

	std::weak_ptr<PlayerCtrlComp>      m_wpPlayer;
	std::weak_ptr<BuildModeComponent>  m_wpBuilder;

	bool m_prevTab = false; // 立ち上がり検出
};