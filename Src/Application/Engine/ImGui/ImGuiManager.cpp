#include "ImGuiManager.h"
#include "../../main.h"
#include "../Entity/Entity/Entity.h"
#include "../Entity/Component/Trans/TransformComponent.h"
#include "../Entity/Component/Render/RenderComponent.h"
#include "../Engine.h"
#include "../../Scene/SceneManager.h"
#include "../Data/ObjData.h"
#include "Editor/EditorScene/EditorScene.h"
#include "Editor/EditorManager.h"

void ImGuiManager::GuiInit()
{
	m_editor = std::make_shared<EditorManager>();
	m_editor->Init();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	// Setup Dear ImGui style
	ImGui::StyleColorsClassic();
	// Setup Platform/Renderer bindings
	ImGui_ImplWin32_Init(Application::Instance().GetWindowHandle());
	ImGui_ImplDX11_Init(
		KdDirect3D::Instance().WorkDev(), KdDirect3D::Instance().WorkDevContext());

#include "imgui/ja_glyph_ranges.h"
	ImGuiIO& io = ImGui::GetIO();
	ImFontConfig config;
	config.MergeMode = true;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	io.Fonts->AddFontDefault();
	// 日本語対応
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msgothic.ttc", 13.0f, &config, glyphRangesJapanese);
}

void ImGuiManager::GuiProcess()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (m_editor->GetMode() == EditorMode::Editor)
	{
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		
		//===========================================================
		// 以下にImGui描画処理を記述
		//==========================================================

		DrawMainMenu();
		m_editor->Draw();
		if (m_editor->IsEditorMode())
		{
			GameScreen();
		}
		//===========================================================
		// ここより上にImGuiの描画はする事
		//===========================================================
	}
	else if (m_editor->GetMode() == EditorMode::Game)
	{
		DrawGame();
	}
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();
}

void ImGuiManager::SetMode(EditorMode m)
{
	if (m_editor) m_editor->SetMode(m);
}

void ImGuiManager::ToggleMode()
{
	if (!m_editor) return;
	m_editor->SetMode(m_editor->IsEditorMode() ? EditorMode::Game : EditorMode::Editor);
}

void ImGuiManager::GuiRelease()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiManager::GameScreen()
{
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

	// 画像にぴったり合わすためにパディングを0に
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("GameScreen", nullptr, flags);

	if (auto rt = EngineCore::Engine::Instance().m_gameViewRT)
	{
		ImVec2 avail = ImGui::GetContentRegionAvail();
		int w = (int)std::max(1.0f, avail.x);
		int h = (int)std::max(1.0f, avail.y);

		// ★ パネルサイズにRTを合わせる
		EngineCore::Engine::Instance().EnsureGameRTSize(w, h);

		if (ID3D11ShaderResourceView* srv = rt->GetSRView())
		{
			ImGui::Image((ImTextureID)srv, ImVec2((float)w, (float)h));

			bool hovered = ImGui::IsItemHovered();
			bool lmbDown = ImGui::IsMouseDown(0);
			EngineCore::Engine::Instance().m_isCameraControlActive = (hovered && lmbDown);
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void ImGuiManager::DrawMainMenu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			auto objData = std::make_shared<ObjectData>();
			if (ImGui::MenuItem("Save"))
			{
				auto entityList = m_editor->GetEntityList();
				auto objList = objData->ConvertToDataList(entityList);
				objData->SaveObj(objList, "Asset/Data/ObjData/ObjData/ObjData.json");
			}
			if (ImGui::MenuItem("Load"))
			{
				auto newEntities = objData->LoadEntityList("Asset/Data/ObjData/ObjData/ObjData.json");
				m_editor->SetEntityList(newEntities);
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Mode"))
		{
			if (ImGui::MenuItem("Editor Mode"))
			{
				m_editor->SetMode(EditorMode::Editor);
			}
			if (ImGui::MenuItem("Game Mode"))
			{
				m_editor->SetMode(EditorMode::Game);
			}
			ImGui::EndMenu();
		}
	}
	ImGui::EndMainMenuBar();
}

void ImGuiManager::DrawGame()
{
	if (!EngineCore::Engine::Instance().m_gameViewRT) return;

	ImGuiViewport* vp = ImGui::GetMainViewport();
	ImGuiIO& io = ImGui::GetIO();

	// 実ピクセルに変換（DPI考慮）
	const float sx = io.DisplayFramebufferScale.x;
	const float sy = io.DisplayFramebufferScale.y;
	const int rtW = (int)std::round(vp->Size.x * sx);
	const int rtH = (int)std::round(vp->Size.y * sy);

	// ★ RT は実ピクセル、描画サイズは UI 座標
	EngineCore::Engine::Instance().EnsureGameRTSize(rtW, rtH);

	// 余白ゼロ
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

	ImGui::SetNextWindowPos(vp->Pos);
	ImGui::SetNextWindowSize(vp->Size);
	ImGui::SetNextWindowViewport(vp->ID);

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoBackground;

	ImGui::Begin("##GameFullscreen", nullptr, flags);

	if (auto srv = EngineCore::Engine::Instance().m_gameViewRT->GetSRView())
	{
		// 画像の描画サイズは UI 座標（= vp->Size）でOK
		ImGui::Image((ImTextureID)srv, vp->Size);
	}

	ImGui::End();
	ImGui::PopStyleVar(3);
}
