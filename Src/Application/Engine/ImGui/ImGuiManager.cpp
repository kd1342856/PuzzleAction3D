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
#include "../Entity/Component/Controller/Player/PlayerCtrlComp.h"
#include "../../Scene/GameScene/GameScene.h"
#include "../../Scene/SceneManager.h"
#include "../../GameObject/Camera/CameraBase.h"

void ImGuiManager::GuiInit()
{
	m_editor = std::make_shared<EditorManager>();
	m_editor->Init();
	if (m_editor) m_editor->SetMode(EditorMode::Game);

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

		m_editor->DrawCameraPanel();

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

void ImGuiManager::SetCameras(const std::shared_ptr<CameraBase>& tps, const std::shared_ptr<CameraBase>& overhead)
{
	if (m_editor) m_editor->SetCameras(tps, overhead);
}

bool ImGuiManager::GetGameViewUVFromMouse(float& u, float& v) const
{
	if (!m_gameImgValid) return false;

	ImVec2 mouse = ImGui::GetMousePos();
	if (mouse.x < m_gameImgMin.x || mouse.y < m_gameImgMin.y ||
	+mouse.x > m_gameImgMax.x || mouse.y > m_gameImgMax.y) return false;

	float w = m_gameImgMax.x - m_gameImgMin.x;
	float h = m_gameImgMax.y - m_gameImgMin.y;

	if (w <= 1 || h <= 1) return false;
	u = (mouse.x - m_gameImgMin.x) / w;
	v = (mouse.y - m_gameImgMin.y) / h;

	return true;
}

bool ImGuiManager::IsMouseOverGameView() const
{
	if (!m_gameImgValid) return false;

	ImVec2 mouse = ImGui::GetMousePos();

	return (mouse.x >= m_gameImgMin.x && mouse.y >= m_gameImgMin.y &&
		+mouse.x <= m_gameImgMax.x && mouse.y <= m_gameImgMax.y);
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

			ImVec2 min = ImGui::GetItemRectMin();
			ImVec2 max = ImGui::GetItemRectMax();
			m_gameImgMin = min;
			m_gameImgMax = max;
			m_gameImgValid = true;

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
	
			if (ImGui::MenuItem("Save"))
			{
				ObjectData io;

				// 既存：エンティティ配列（Player は ConvertToDataList 内で除外）
				auto& list = m_editor->GetEntityList();
				auto objects = io.ConvertToDataList(list);

				// 追加：Overhead カメラを特別レコードで保存
				if (m_editor)
				{
					if (auto oh = m_editor->GetOverheadCamera())   // ← EditorManager に getter がある想定
					{
						ObjData cam;
						cam.name = "__OverheadCamera";
						cam.modelPath = "";
						cam.isDynamic = false;
						cam.scale = { 1,1,1 };

						// CameraBase の API に合わせる（GetPos/GetRotation ではなく）
						cam.pos = oh->GetPosition();
						cam.rot = oh->GetEulerDeg();

						objects.push_back(cam);                    // ← 変数名は objects
					}
				}

				io.SaveObj(objects, "Asset/Data/ObjData/ObjData/ObjData.json");
			}

			if (ImGui::MenuItem("Load"))
			{
				auto io = std::make_shared<ObjectData>();
				auto all = io->LoadJson("Asset/Data/ObjData/ObjData/ObjData.json");

				// Overhead カメラの抽出＆反映
				std::vector<ObjData> entitiesOnly;
				for (const auto& d : all)
				{
					if (d.name == "__OverheadCamera")
					{
						if (m_editor)
						{
							if (auto oh = m_editor->GetOverheadCamera())
							{
								// CameraBase の API に合わせる
								oh->SetPosition(d.pos);
								oh->SetEulerDeg(d.rot);
							}
						}
						// エンティティ化しない
					}
					else
					{
						entitiesOnly.push_back(d);
					}
				}

				// 残りを通常どおり生成して差し替え
				std::vector<std::shared_ptr<Entity>> newEntities;
				newEntities.reserve(entitiesOnly.size());

				for (auto& data : entitiesOnly)
				{
					auto ent = std::make_shared<Entity>();

					auto tf = std::make_shared<TransformComponent>();
					tf->SetPos(data.pos);
					tf->SetRotation(data.rot);
					tf->SetScale(data.scale);
					ent->AddComponent<TransformComponent>(tf);

					auto rc = std::make_shared<RenderComponent>();
					if (!data.modelPath.empty())
					{
						if (data.isDynamic) rc->SetModelWork(data.modelPath);
						else                rc->SetModelData(data.modelPath);
					}
					ent->AddComponent<RenderComponent>(rc);

					ent->SetName(data.name);

					using VF = Entity::VisibilityFlags;
					ent->SetVisibility(VF::Lit, data.isLit);
					ent->SetVisibility(VF::UnLit, data.isUnLit);
					ent->SetVisibility(VF::Bright, data.isBright);
					ent->SetVisibility(VF::Shadow, data.isShadow);

					ent->Init();
					newEntities.push_back(std::move(ent));
				}

				auto& bound = m_editor->GetEntityList();
				bound.clear();
				for (auto& e : newEntities)
				{
					SceneManager::Instance().AddObject(e);
					bound.push_back(e);
				}
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

		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		m_gameImgMin = min;
		m_gameImgMax = max;
		m_gameImgValid = true;
	}

	ImGui::End();
	ImGui::PopStyleVar(3);
}
