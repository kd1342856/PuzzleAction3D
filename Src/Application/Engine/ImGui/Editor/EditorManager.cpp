#include "EditorManager.h"
#include "EditorScene/EditorScene.h"
#include "EditorUI/EditorUI.h"
#include "../../../Scene/SceneManager.h"
#include "EditorCamera/EditorCamera.h"
#include "../../Engine.h"
#include "../../Entity/Entity/Entity.h"

void EditorManager::Init()
{
	m_scene = std::make_shared<EditorScene>();
	m_ui = std::make_shared<EditorUI>();
}

void EditorManager::Update()
{
	m_scene->Update();
}

void EditorManager::Draw()
{
	if (!m_scene)return;
	m_scene->Draw();
	if (m_ui)
	{
		m_ui->Update(*this);
	}
}


void EditorManager::SetUseTPSCamera(bool v)
{
	if (m_useTPSCamera != v)
	{
		m_useTPSCamera = v;
		ApplyCameraState();
	}
}

EditorMode EditorManager::GetMode() const
{
	return m_scene->GetMode();
}

void EditorManager::SetMode(EditorMode mode)
{
	m_scene->SetMode(mode);
	ApplyCameraState();
}

int EditorManager::FindByName(const std::string& name) const
{
	const auto& v = m_entityListRef ? *m_entityListRef : m_fallbackList;
	for (int i = 0; i < (int)v.size(); ++i) { if (v[i] && v[i]->GetName() == name) return i; }
	return -1;
}

void EditorManager::PushEntity(const std::shared_ptr<Entity>& e)
{
	GetEntityList().push_back(e);
}

bool EditorManager::IsEditorMode() const
{
	return m_scene && m_scene->IsEditorMode();
}

void EditorManager::SetCameras(const std::shared_ptr<CameraBase>& tpsCam, const std::shared_ptr<CameraBase>& overheadCam)
{
	m_wpTPSCam = tpsCam;
	m_wpOverheadCam = overheadCam;
	ApplyCameraState();
}

void EditorManager::SetActiveCamView(CamView v)
{
	if (m_camView == v) return;
	m_camView = v;
	ApplyCameraState();
}

void EditorManager::DrawCameraPanel()
{
	ImGui::Begin("Overhead Camera");

	bool prev = m_useTPSCamera;
	if (ImGui::Checkbox("Use TPS camera", &m_useTPSCamera))
	{
		if (m_useTPSCamera != prev)
		{
			ApplyCameraState();
		}
	}

	// ★ エディタモード時は常にカーソル表示（掴まない）
	if (IsEditorMode())
	{
		EngineCore::Engine::Instance().SetMouseGrabbed(false);
	}

	auto overhead = m_wpOverheadCam.lock();

	if (overhead)
	{
		if (m_useTPSCamera) ImGui::BeginDisabled(); // TPS中は俯瞰のUIだけ無効化

		Math::Vector3 pos = overhead->GetPosition();
		Math::Vector3 rot = overhead->GetEulerDeg();
		if (ImGui::DragFloat3("Position", &pos.x, 0.1f))   overhead->SetPosition(pos);
		if (ImGui::DragFloat3("Rotation (deg)", &rot.x, 1))overhead->SetEulerDeg(rot);

		if (m_useTPSCamera) ImGui::EndDisabled();
	}
	else
	{
		ImGui::TextUnformatted("Overhead camera not set.");
	}

	ImGui::Separator();
	ImGui::Text("Active: %s", m_useTPSCamera ? "TPS" : "Overhead");

	ImGui::End();
}

void EditorManager::ApplyCameraState()
{
	auto overhead = m_wpOverheadCam.lock();
	auto tps = m_wpTPSCam.lock();
	const bool inEditor = (m_scene && m_scene->IsEditorMode());

	if (inEditor)
	{
		// Editor＝俯瞰ON / TPS OFF、マウス解放
		if (overhead) overhead->SetActive(true);
		if (tps)      tps->SetActive(false);
		EngineCore::Engine::Instance().SetMouseGrabbed(false);
	}
	else
	{
		// Game では Editor 側は何もしない（GameModeManager が制御）
		return;
	}
}
