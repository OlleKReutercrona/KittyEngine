#include "stdafx.h"

#include "ImGuiHandler.h"


#include <External/Include/imgui/imgui.h>
#include <External/Include/imgui/imgui_impl_dx11.h>
#include <External/Include/imgui/imgui_impl_win32.h>

#include <Engine/Source/ComponentSystem/GameObject.h>
#include <Engine/Source/ComponentSystem/GameObjectManager.h>
#include <Engine/Source/Math/Transform.h>
#include <Engine/Source/SceneManagement/SceneManager.h>
#include <Engine/Source/SceneManagement/Scene.h>
#include <Engine/Source/Math/KittyMath.h>

#include <External/Include/imguizmo/ImGuizmo.h>
#include <External/Include/imguizmo/ImSequencer.h>

#include <Engine/Source/Utility/Global.h>
#include <Engine/Source/Utility/StringUtils.h>

#include <Engine/Source/Graphics/Lighting/LightManager.h>
#include <Engine/Source/Graphics/Texture/Texture.h>
#include <Engine/Source/Graphics/Texture/TextureLoader.h>
#include <Engine/Source/Graphics/ModelData.h>
#include <Engine/Source/Graphics/ModelLoader.h>
#include <Engine/Source/Graphics/FX/VFX.h>
#include <Engine/Source/Build/BuildManager.h>

//components!
#include <Engine/Source/ComponentSystem/Components/Component.h>
#include <Engine/Source/ComponentSystem/Components/Graphics/ModelComponent.h>
#include <Engine/Source/ComponentSystem/Components/Graphics/SkeletalModelComponent.h>
#include <Engine/Source/ComponentSystem/Components/Graphics/PostProcessingComponent.h>
#include <Engine/Source/ComponentSystem/Components/LightComponent.h>
#include <Engine/Source/ComponentSystem/Components/Graphics/CameraComponent.h>
#include <Engine/Source/ComponentSystem/Components/Collider/BoxColliderComponent.h>
#include <Engine/Source/Collision/Collider.h>
#include <Engine/Source/ComponentSystem/Components/Collider/SphereColliderComponent.h>
#include <Engine/Source/Graphics/DebugRenderer.h>
//
#include <External/Include/imnodes/imnodes.h>

#include <filesystem>
#include <typeinfo>

#include <string.h>

//#include "Editor/Source/MeowFile.h"
#include "ComponentSystem/Components/TestComponent.h"
#include "ComponentSystem/Components/Graphics/DecalComponent.h"
#include "ComponentSystem/Components/Graphics/ShellTexturingComponent.h"
#include "ComponentSystem/Components/Graphics/VFXComponent.h"
#include "Editor/Source/Editor.h"
#include "Input/InputHandler.h"
#include "Utility/DebugTimeLogger.h"
#include "Utility/Logging.h"

#include "Project/Source/Player/Player.h"
#include "Project/Source/Boomerang/BoomerangComponent.h"
#include "Project/Source/Boomerang/BoomerangPhysxController.h"
#include "Project/Source/Camera/ActionCameraComponent.h"
#include "Project/Source/Camera/CameraSettingsFile.h"
#include "Project/Source/Managers/GameManager.h"
#include "Project/Source/GameSystems/LobbySystem.h"
#include "Project/Source/Powerups/DashThroughWallsPowerup.h"
#include "Project/Source/Powerups/ShieldPowerup.h"
#include "Project/Source/Powerups/SplitBoomerangPowerup.h"
#include "Project/Source/Powerups/TeleportPowerup.h"
#include "Project/Source/Water/WaterComponent.h"

#define coutVec2(v) v.x << ", " << v.y
#define coutVec3(v) v.x << ", " << v.y << ", " << v.z
#define coutVec4(v) v.x << ", " << v.y << ", " << v.z << ", " << v.w

#ifndef KITTYENGINE_NO_EDITOR



bool KE_EDITOR::ImGuiHandler::WillWindowDock(ImGuiID aWindowID)
{
	if (windowDockedMap.find(aWindowID) == windowDockedMap.end())
	{
		windowDockedMap[aWindowID] = false;
	}

	return windowDockedMap[aWindowID];
}

void KE_EDITOR::ImGuiHandler::Init(HWND hwnd, ID3D11Device* aDevice, ID3D11DeviceContext* aDeviceContext, Editor* aEditor)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImNodes::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(aDevice, aDeviceContext);

	editor = aEditor;
	myDebugRenderer = KE_GLOBAL::blackboard.Get<KE::DebugRenderer>("debugRenderer");

	//set custom style stuff:
	{
		//colours
		{
			ImVec4* colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
			colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
			colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.01f, 0.01f, 0.78f);
			colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
			colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
			colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.53f, 0.41f, 0.55f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.08f, 0.53f, 0.41f, 0.29f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.08f, 0.53f, 0.41f, 0.20f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.27f, 0.22f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
			colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.53f, 0.41f, 0.04f);
			colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.08f, 0.53f, 0.41f, 0.55f);
			colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
			colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 1.00f, 0.67f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.02f, 0.52f, 0.39f, 1.00f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.02f, 0.71f, 0.53f, 1.00f);
			colors[ImGuiCol_Button] = ImVec4(0.08f, 0.53f, 0.41f, 0.55f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.08f, 0.53f, 0.41f, 0.29f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.08f, 0.53f, 0.41f, 0.20f);
			colors[ImGuiCol_Header] = ImVec4(0.08f, 0.53f, 0.41f, 0.55f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.08f, 0.53f, 0.41f, 0.29f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.08f, 0.53f, 0.41f, 0.20f);
			colors[ImGuiCol_Separator] = ImVec4(0.08f, 0.53f, 0.41f, 0.20f);
			colors[ImGuiCol_SeparatorHovered] = ImVec4(1.00f, 0.00f, 0.80f, 0.78f);
			colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 0.96f, 0.00f, 1.00f);
			colors[ImGuiCol_ResizeGrip] = ImVec4(0.08f, 0.53f, 0.41f, 0.20f);
			colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.08f, 0.53f, 0.41f, 0.55f);
			colors[ImGuiCol_ResizeGripActive] = ImVec4(0.08f, 0.53f, 0.41f, 0.78f);
			colors[ImGuiCol_Tab] = ImVec4(0.16f, 0.24f, 0.22f, 0.78f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.08f, 0.53f, 0.41f, 0.71f);
			colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.49f, 0.35f, 1.00f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.16f, 0.24f, 0.22f, 0.78f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.08f, 0.53f, 0.45f, 0.51f);
			colors[ImGuiCol_DockingPreview] = ImVec4(0.08f, 0.53f, 0.41f, 0.71f);
			colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
			colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
			colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
			colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
			colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
			colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
			colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
			colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
			colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
			colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
			colors[ImGuiCol_TextSelectedBg] = ImVec4(0.10f, 0.61f, 0.45f, 0.59f);
			colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
			colors[ImGuiCol_NavHighlight] = ImVec4(0.14f, 0.65f, 0.53f, 1.00f);
			colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
			colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
			colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		}

		//style
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 3;
			style.WindowBorderSize = 0;
			style.PopupBorderSize = 0;

			style.WindowRounding = 6;
			style.PopupRounding = 6;
			style.GrabRounding = 4;
			style.TabRounding = 4;
		}

		//font
		{
			//ImGui::GetIO().Fonts->AddFontFromFileTTF("Data/InternalAssets/Katex.ttf", 16.0f);
		}
	}
}

void KE_EDITOR::ImGuiHandler::DisplayProgressBar(float aProgress)
{
	ImVec4 noProgress = { 1.0f, 0.0f, 0.0f, 1.0f };
	ImVec4 midProgress = { 1.0f, 1.0f, 0.0f, 1.0f };
	ImVec4 yesProgress = { 0.0f, 1.0f, 0.0f, 1.0f };

	float toHalf = (std::min)(aProgress / 0.5f, 1.0f);
	float toEnd = (std::max)((std::min)((aProgress - 0.5f) / 0.5f, 1.0f),0.0f);
	ImVec4 lerpedCol = ImLerp(ImLerp(noProgress, midProgress, toHalf), yesProgress, toEnd);

	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, lerpedCol);
	ImGui::ProgressBar(aProgress);
	ImGui::PopStyleColor();
}

void KE_EDITOR::ImGuiHandler::BeginFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuizmo::BeginFrame();
}

void KE_EDITOR::ImGuiHandler::EndFrame()
{
	//
	customHandles.clear();
	//

	ImGui::Render();


	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();

	
}

void KE_EDITOR::ImGuiHandler::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImNodes::DestroyContext();
	ImGui::DestroyContext();
}

bool KE_EDITOR::ImGuiHandler::TrySameLine(float aWidth, float aSpacing)
{
	ImGui::SameLine();
	if (ImGui::GetCursorPosX() + aWidth + aSpacing > ImGui::GetWindowWidth())
	{
		ImGui::NewLine();
		return false;
	}

	return true;
}

bool KE_EDITOR::ImGuiHandler::TryBeginWindow(const char* aString, bool* aCondition, ImGuiWindowFlags aWindowFlags)
{
	if (aCondition == nullptr || *aCondition == true)
	{
		if (ImGui::Begin(aString, aCondition, aWindowFlags))
		{
			return true;
		}

		ImGui::End();
		return false;
	}

	return false;
}

bool KE_EDITOR::ImGuiHandler::PerformFlagsBegin(const char* aString, KittyFlags aFlags, bool* aOpenFlag, int aInputFlags)
{
	if (aFlags & eBeginWindow) { if (!TryBeginWindow(aString, aOpenFlag, aInputFlags)) { return false; } }
	if (aFlags & eSeparatorAbove) { ImGui::SeparatorText(aString); }


	flagStack.push_back(aFlags);
	return true;
}

void KE_EDITOR::ImGuiHandler::PerformFlagsEnd()
{
	if (flagStack.empty()) { return; }

	if (flagStack.back() & eSeparatorBelow) { ImGui::Separator(); }
	if (flagStack.back() & eBeginWindow) { ImGui::End(); }



	flagStack.pop_back();
}

void KE_EDITOR::ImGuiHandler::DisplayMainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::BeginMenu("Levels"))
			{
				auto& allScenes = editor->mySceneManager->GetScenes();

				for (int i = 0; i < allScenes.size(); ++i)
				{
					bool isCurrentLevel = editor->mySceneManager->GetCurrentScene() == &allScenes[i];
					bool isStartupLevel = editor->mySceneManager->GetStartupScene() == &allScenes[i];

					if (ImGui::BeginChild(FormatString("%i sceneChild", i), ImVec2(256.0f, 16.0f)))
					{
						if (ImGui::Selectable(allScenes[i].sceneName.c_str(), &isCurrentLevel))
						{
							customHandles.clear();
							editor->mySceneManager->SwapScene(allScenes[i].sceneID);
							//gameObjectCacheDirty = true;
						}
					}
					ImGui::EndChild();
					ImGui::SameLine();

					if (ImGui::Checkbox(FormatString("Startup##%i sceneActiveStartup", i), &isStartupLevel))
					{
						editor->mySceneManager->ExportStartupScene(&allScenes[i]);
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Settings"))
			{
				if (ImGui::Button("Save Settings"))
				{
					editor->SaveMeowFile();
				}
				if (ImGui::Button("Load Settings"))
				{
					editor->LoadMeowFile();
				}
				if (ImGui::Button("Reset Settings"))
				{
					editor->ResetMeowFile();
				}
				if (ImGui::Button("Save Post Process Settings"))
				{
					editor->SavePostProcessSettings();
				}
				if (ImGui::Button("Load Post Process Settings"))
				{
					editor->LoadPostProcessSettings();
				}
				if (ImGui::BeginMenu("Navmesh"))
				{
					if (ImGui::Button("Save KittyMesh"))
					{
						editor->SaveKittyMeshFile();
					}
					if (ImGui::Button("Load KittyMesh"))
					{
						editor->LoadKittyMeshFile();
					}
					if (ImGui::Button("Clear navmesh"))
					{
						editor->ClearNavmesh();
					}

					ImGui::EndMenu();
				}



				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Assets"))
			{
				if (ImGui::Button("Reload Textures"))
				{
					editor->myGraphics->GetTextureLoader().ReloadAllTextures();
				}

				if (ImGui::Button("Run Asset Loader"))
				{
					editor->myGraphics->GetAssetLoader().Init();
				}

				if (ImGui::BeginMenu("Shaders"))
				{
					DisplayShaderList();
					ImGui::EndMenu();
				}
				//if (ImGui::Button("Create Thumbnails"))
				//{
				//	editor->myAssetBrowser.GenerateThumbnails();
				//}

				ImGui::EndMenu();
			}


			if (ImGui::BeginMenu("Build Settings"))
			{
				std::string appName(WideStringToNarrow(KE::BuildManager::GetApplicationName()));

				ImGui::Text("Application Name: %s", appName.c_str());

				static char inputbuffer[64] = {0};
				ImGui::InputText("New application Name: ", inputbuffer, 64);
				if (ImGui::Button("Save"))
				{
					wchar_t outbuffer[64] = { 0 };
					NarrowStrToWideStr(inputbuffer, outbuffer);
					KE::BuildManager::AssignEXEName(outbuffer);
				}
				if (ImGui::Button("Reset"))
				{
					memset(inputbuffer, 0, 64);
				}

				if (ImGui::Button("Set Build Directory"))
				{
					KE::BuildManager::SetBuildDirectory();
				}
				if (ImGui::Button("Create Build"))
				{
					KE::BuildManager::StartBuild();
				}

				ImGui::SameLine();

				ImVec4 colour = KE::BuildManager::IsBuilding() ? ImVec4(1.0f, 0.0f, 0.0f, 1.0f) : ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

				//ImGui::ColorButton("##isbuildrunning", colour);

				if (KE::BuildManager::IsBuilding())
				{
					ImGui::SetNextItemWidth(75.0f);
					DisplayProgressBar(KE::BuildManager::GetProgress());
					
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::BeginMenu("Debug Rendering"))
			{
				if (ImGui::BeginMenu("Draw Flags"))
				{
					KE::Scene& currentScene = *editor->mySceneManager->GetCurrentScene();

					for (int i = 0; i < 4; i++)
					{
						KE::SceneDrawFlags flag = (KE::SceneDrawFlags)(1 << i);

						bool isSet = currentScene.GetDrawFlag(flag);
						if (ImGui::Checkbox(EnumToString(flag), &isSet))
						{
							currentScene.ToggleDrawDebugFlag(flag);
						}
					}

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Navmesh"))
				{
					DebugRenderLevel& renderLevel = editor->myData.debugRenderData.renderNavmesh;
					if (ImGui::RadioButton("None", renderLevel == DebugRenderLevel::eNone)) { renderLevel = DebugRenderLevel::eNone; }
					if (ImGui::RadioButton("Basic", renderLevel == DebugRenderLevel::eBasic)) { renderLevel = DebugRenderLevel::eBasic; }
					if (ImGui::RadioButton("Full", renderLevel == DebugRenderLevel::eFull)) { renderLevel = DebugRenderLevel::eFull; }
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Skeleton"))
				{
					DebugRenderLevel& renderLevel = editor->myData.debugRenderData.renderSkeleton;
					if (ImGui::RadioButton("None", renderLevel == DebugRenderLevel::eNone)) { renderLevel = DebugRenderLevel::eNone; }
					if (ImGui::RadioButton("Basic", renderLevel == DebugRenderLevel::eBasic)) { renderLevel = DebugRenderLevel::eBasic; }
					if (ImGui::RadioButton("Full", renderLevel == DebugRenderLevel::eFull)) { renderLevel = DebugRenderLevel::eFull; }
					ImGui::EndMenu();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Render Layers"))
			{
				if (ImGui::Button("Regenerate Instancing Data"))
				{
					for (int i = 0; i < static_cast<int>(KE::eRenderLayers::Count); i++)
					{
						editor->myGraphics->GetRenderLayer(static_cast<KE::eRenderLayers>(i))->GenerateInstancingData(editor->myGraphics->GetModelData());
					}
				}

				for (int i = 0; i < static_cast<int>(KE::eRenderLayers::Count); i++)
				{
					ImGui::PushID(i);
					KE::RenderLayer* layer = editor->myGraphics->GetRenderLayer(static_cast<KE::eRenderLayers>(i));
					bool active = layer->flags & KE::RenderLayerFlags_Active;
					if (ImGui::Checkbox(EnumToString(static_cast<KE::eRenderLayers>(i)), &active))
					{
						layer->flags ^= KE::RenderLayerFlags_Active;
					}
					ImGui::SameLine();
					if (ImGui::BeginMenu("Settings"))
					{
						ImGui::SeparatorText("Flags");
						for (int j = 0; j < KE::RenderLayerFlags_Count; j++)
						{
							bool flag = layer->flags & (1 << j);
							if (ImGui::Checkbox(KE::RenderLayerFlagStrings[j], &flag))
							{
								layer->flags ^= (1 << j);
							}
						}
						ImGui::Separator();

						DisplayEnumSlider("DepthStencilState", &layer->mySettings.depthStencilState, KE::eDepthStencilStates::Count);
						DisplayEnumSlider("RasterizerState", &layer->mySettings.rasterizerState, KE::eRasterizerStates::Count);
						DisplayEnumSlider("BlendState", &layer->mySettings.blendState, KE::eBlendStates::Count);

						

						ImGui::EndMenu();
					}
					ImGui::PopID();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Windows"))
		{
			for (const auto& [windowName, entry] : editor->myWindowRegistry)
			{
				if (entry.hidden) { continue; }
				if (ImGui::MenuItem(windowName.c_str()))
				{
					entry.myCreationFunc({});
				}
			}
			ImGui::SeparatorText("LambdaWindows");
			for (const auto& lambdaWindowData : editor->myLambdaWindowRegistry)
			{
				if (ImGui::MenuItem(lambdaWindowData.first.c_str()))
				{

					editor->myWindows.push_back(new LambdaWindow(lambdaWindowData.second));
				}
			}

			ImGui::EndMenu();
		}

		ImGui::SameLine(ImGui::GetWindowWidth() / 2.0f - 147.0f / 2.0f);
		//states: 0: Play, 1: Pause, 2: Stop
		static char state = 1;

		if (ImGui::Button("Play")) 
		{
			KE::SceneEvent event;
			event.mySceneState = KE::eSceneState::ePlayMode;
			ES::EventSystem::GetInstance().SendEvent(event);
			state = 0;
		}
		ImGui::SameLine();
		if (ImGui::Button("Pause")) 
		{
			KE::SceneEvent event;
			event.mySceneState = KE::eSceneState::ePauseMode;

			ES::EventSystem::GetInstance().SendEvent(event);
			state = 1;
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop")) 
		{
			KE::SceneEvent event;
			event.mySceneState = KE::eSceneState::eExitPlayMode;

			ES::EventSystem::GetInstance().SendEvent(event);
			state = 2;
		}
		ImGui::SameLine();
		ImGui::Text(state == 0 ? "State: Play" : state == 1 ? "State: Pause" : "State: Stopped");

		//put text at the end of the menu bar
		ImGui::SameLine(ImGui::GetWindowWidth() - 400);
		ImVec2 currentPos = ImGui::GetCursorPos();
		ImGui::Text("Draw Calls: %i (%i Light)", editor->myGraphics->GetLastDrawCalls(), editor->myGraphics->GetLastLightDrawCalls());

		ImGui::SameLine(ImGui::GetWindowWidth() - 200);
		ImGui::Text("FPS: %i (%f MS)", (int)editor->myData.performance.Fps(), editor->myData.performance.deltaTime * 1000.f);
		ImGui::SameLine();
		ImVec2 deltaPos = ImGui::GetCursorPos() - currentPos;

		ImGui::SetCursorPos(currentPos);
		if (ImGui::BeginMenu("##performanceDropdown"))
		{
			auto logs = KE::DebugTimeLogger::myHeads;

			//if (logs.count("Frame") > 0)
			//{
			//	ImGui::Text("FPS: %i", (int)(1.0f / (logs.at("Frame").myTime / 1000.0f)));
			//}

			for (auto& log : logs)
			{
				editor->ExtractLogChildren(log.second, logs.at("Frame").myTime);
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void KE_EDITOR::ImGuiHandler::BeginFullscreenWindow()
{
	auto dockID = ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_PassthruCentralNode);
}

void KE_EDITOR::ImGuiHandler::DisplayNavmesh(KE::Navmesh* aNavmesh, KE::Camera* aCamera, ImVec4 aViewportBounds)
{
	float viewportWidth = aViewportBounds.z;
	float viewportHeight = aViewportBounds.w;
	ImVec2 upperLeft = ImVec2(aViewportBounds.x, aViewportBounds.y);
	ImVec2 size = ImVec2(aViewportBounds.z, aViewportBounds.w);

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->PushClipRect(upperLeft, upperLeft + size);

	Vector2f viewDims = Vector2f(viewportWidth, viewportHeight);
	ImVec2 windowMousePos = GetMousePosInWindow(ImVec2(0.0f, 0.0f));

	for (KE::NavNode& node : aNavmesh->myNodes)
	{
		int nodeIndex = node.index;

		ImVec2 nodePos[3];

		for (int i = 0; i < 3; i++)
		{
			Vector3f pos = *node.vertices[i];
			pos.z -= 0.1f;
			pos.x -= 0.1f;
			Vector2f clip = { FLT_MIN,FLT_MIN };
			if (!aCamera->WorldToScreenPoint(pos, clip, viewDims)) { continue; }

			ImVec2 imClip = ImVec2(clip.x, clip.y);

			nodePos[i] = upperLeft + imClip;

			drawList->AddText(nodePos[i], ImColor(1.0f, 0.0f, 1.0f, 1.0f), FormatString("%i", node.indices[i]));
		}

		{
			Vector3f pos = node.myCenter;
			Vector2f clip;
			if (aCamera->WorldToScreenPoint(pos, clip, viewDims))
			{
				ImVec2 imClip = ImVec2(clip.x, clip.y);
				drawList->AddText(upperLeft + imClip, ImColor(1.0f, 1.0f, 1.0f, 1.0f), FormatString("%i", nodeIndex));
			}
		}

		for (int i = 0; i < 3; i++)
		{
			CustomHandle& newHandle = customHandles.emplace_back();
			newHandle.myData = (void*)node.vertices[i];
			newHandle.myGizmoType = ArbitraryGizmo::Type::VECTOR3;
			newHandle.myTransform.SetPosition(*node.vertices[i]);
		}
	}

	//if (ImGui::IsKeyPressed(ImGuiKey_M)) //temp: m for merge?
	//{
	//	std::vector<Vector3f*> selectedVertices;
	//	std::vector<KE::Node*> nodesToRemove;

	//	std::vector<int> indicesToMerge;
	//	
	//	for (void* data : arbitraryGizmo.myDatas)
	//	{
	//		int index = 0;
	//		for (Vector3f& vertex : aNavmesh->myVertices)
	//		{
	//			if (data == &vertex)
	//			{
	//				indicesToMerge.push_back(index);
	//			}
	//			index++;
	//		}
	//	}

	//	std::cout << "Merging " << indicesToMerge.size() << " vertices" << std::endl;
	//	
	//	int resolveIndex = indicesToMerge[0];
	//	for (KE::Node& node : aNavmesh->myNodes)
	//	{
	//		int affectedVertexCount = 0;
	//		for (int i = 0; i < indicesToMerge.size(); i++)
	//		{
	//			int index = indicesToMerge[i];
	//			for (int j = 0; j < 3; j++)
	//			{
	//				if (node.indices[j] == index)
	//				{
	//					affectedVertexCount++;
	//					node.indices[j] = resolveIndex;
	//					node.vertices[j] = &aNavmesh->myVertices[resolveIndex];
	//				}
	//			}
	//		}
	//		
	//		if (affectedVertexCount > 1)
	//		{
	//			nodesToRemove.push_back(&node);
	//		}
	//	}

	//	for (int i = 0; i < nodesToRemove.size(); i++)
	//	{
	//		nodesToRemove[i]->myNeighbours.clear();
	//		nodesToRemove[i]->myCenter = Vector3f(FLT_MAX, FLT_MAX, FLT_MAX);
	//	}

	//	for (KE::Node& node : aNavmesh->myNodes)
	//	{
	//		for (int i = 0; i < node.myNeighbours.size(); i++)
	//		{
	//			for (KE::Node* deadNode : nodesToRemove)
	//			{
	//				if (node.myNeighbours.size() > i && node.myNeighbours[i] == deadNode)
	//				{
	//					node.myNeighbours.erase(node.myNeighbours.begin() + i);
	//				}
	//			}
	//		}
	//	}
	//}

	drawList->PopClipRect();
}

void KE_EDITOR::ImGuiHandler::DisplayTransform(Transform* aTransform, KittyFlags aFlags)
{
	size_t transformID = (size_t)aTransform;

	ImGui::PushID((unsigned int)transformID);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(0.12f, 0.12f, 0.12f, 1.0f).Value);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

	ImGui::BeginChild("TransformChild", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);

	if (PerformFlagsBegin("Transform", aFlags))
	{
		Vector3f pos;
		Vector3f rot;
		Vector3f scale;

		ImGuizmo::DecomposeMatrixToComponents((float*)&aTransform->GetMatrix(), &pos.x, &rot.x, &scale.x);

		if (ImGui::DragFloat3("Position", &pos.x)) {}

		if (ImGui::DragFloat3("Rotation", &rot.x)) {}

		if (ImGui::DragFloat3("Scale", &scale.x)) {}

		ImGuizmo::RecomposeMatrixFromComponents(&pos.x, &rot.x, &scale.x, (float*)&aTransform->GetMatrix());


		static bool showMatrix = false;
		//ImGui::Checkbox("Show Matrix", &showMatrix);

		if (showMatrix)
		{
			DirectX::XMMATRIX matrix = aTransform->GetMatrix();
			bool changed = false;
			if (ImGui::DragFloat4("##row1", &matrix.r[0].m128_f32[0])) { changed = true; }
			if (ImGui::DragFloat4("##row2", &matrix.r[1].m128_f32[0])) { changed = true; }
			if (ImGui::DragFloat4("##row3", &matrix.r[2].m128_f32[0])) { changed = true; }
			if (ImGui::DragFloat4("##row4", &matrix.r[3].m128_f32[0])) { changed = true; }
			if (changed)
			{
				aTransform->SetMatrix(matrix);
			}
		}


		//if (false && ImGui::CollapsingHeader("Evaluated Values"))
		//{
		//	Vector3f forward = aTransform->GetForward();
		//	Vector3f up = aTransform->GetUp();
		//	Vector3f right = aTransform->GetRight();

		//	ImGui::InputFloat3("Right", &right.x);
		//	ImGui::InputFloat3("Up", &up.x);
		//	ImGui::InputFloat3("Forward", &forward.x);
		//}

		//

		PerformFlagsEnd();
	}

	ImGui::EndChild();
	ImGui::PopID();

	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

bool KE_EDITOR::ImGuiHandler::DisplayCustomHandle(CustomHandle& aCustomHandle, KE::Camera* aCamera, ImVec4 aViewportBounds)
{
	ImVec2 windowMousePos = GetMousePosInWindow(ImVec2(0.0f, 0.0f));
	ImVec2 upperLeft = ImVec2(aViewportBounds.x, aViewportBounds.y);
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	bool used = false;
	bool mouseClicked = ImGui::IsMouseClicked(ImGuiMouseButton_Left);

	bool isUsingData = false;
	for (void* data : arbitraryGizmo.myDatas)
	{
		if (data == aCustomHandle.myData)
		{
			isUsingData = true;
			break;
		}
	}

	switch (aCustomHandle.myGizmoType)
	{
	case ArbitraryGizmo::Type::VECTOR3:
	{
		//calculate screen position
		Vector3f drawPos = aCustomHandle.myTransform.GetPosition();
		/*Vector3f pos = *(Vector3f*)aCustomHandle.myData;*/
		Vector2f clip;
		if (!aCamera->WorldToScreenPoint(drawPos, clip, Vector2f(aViewportBounds.z, aViewportBounds.w))) { break; }

		ImVec2 imClip = ImVec2(clip.x, clip.y);


		const float nodeRadius = 5.0f;
		const float dist = (Vector2f(windowMousePos.x, windowMousePos.y) - Vector2f(upperLeft.x + imClip.x, upperLeft.y + imClip.y)).LengthSqr();


		if (dist < nodeRadius * nodeRadius || isUsingData)
		{
			drawList->AddCircleFilled(upperLeft + imClip, nodeRadius, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
			if (mouseClicked && !isUsingData)
			{
				arbitraryGizmo.Attach(ImGui::GetIO().KeyCtrl, ArbitraryGizmo::Type::VECTOR3, aCustomHandle.myData, aCustomHandle.myTransform);
				used = true;
			}
		}
		else
		{
			drawList->AddCircle(upperLeft + imClip, nodeRadius, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
		}
		drawList->AddText(upperLeft + imClip, ImColor(1.0f, 1.0f, 1.0f, 1.0f), aCustomHandle.myText.c_str());

		break;
	}
	case ArbitraryGizmo::Type::TRANSFORM:
	{
		Vector3f drawPos = aCustomHandle.myTransform.GetPosition();
		/*Vector3f pos = *(Vector3f*)aCustomHandle.myData;*/
		Vector2f clip;
		if (!aCamera->WorldToScreenPoint(drawPos, clip, Vector2f(aViewportBounds.z, aViewportBounds.w))) { break; }

		ImVec2 imClip = ImVec2(clip.x, clip.y);


		const float nodeRadius = 5.0f;
		const float dist = (Vector2f(windowMousePos.x, windowMousePos.y) - Vector2f(upperLeft.x + imClip.x, upperLeft.y + imClip.y)).LengthSqr();
		if (dist < nodeRadius * nodeRadius || isUsingData)
		{
			drawList->AddCircleFilled(upperLeft + imClip, nodeRadius, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
			if (mouseClicked && !isUsingData)
			{
				arbitraryGizmo.Attach(ImGui::GetIO().KeyCtrl, ArbitraryGizmo::Type::TRANSFORM, aCustomHandle.myData, aCustomHandle.myTransform);
				used = true;
			}
		}
		else
		{
			drawList->AddCircle(upperLeft + imClip, nodeRadius, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
		}
		drawList->AddText(upperLeft + imClip, ImColor(1.0f, 1.0f, 1.0f, 1.0f), aCustomHandle.myText.c_str());

		break;
	}
	default:
	{
		break;
	}
	}
	return used;
}

bool KE_EDITOR::ImGuiHandler::DisplayCustomHandles(KE::Camera* aCamera, ImVec4 aViewportBounds)
{
	bool anyUsed = false;
	for (int i = 0; i < customHandles.size(); i++)
	{
		anyUsed = DisplayCustomHandle(customHandles[i], aCamera, aViewportBounds) ? true : anyUsed;
	}
	return anyUsed;
}

void KE_EDITOR::ImGuiHandler::DisplayBoundingBox(const Vector3f& aWorldMin, const Vector3f& aWorldMax, KE::Camera* aCamera, ImVec4 aViewportBounds) {}

bool KE_EDITOR::ImGuiHandler::DisplayArbitraryGizmo(ArbitraryGizmo* aGizmo, KE::Camera* aCamera, ImVec4 aViewportBounds)
{
	//Vector3f averagePos;
	//for (void* data : aGizmo->myDatas)
	//{
	//	averagePos += *(Vector3f*)data;
	//}
	//averagePos /= (float)aGizmo->myDatas.size();
	//
	//Transform transform;
	//transform.SetPosition(averagePos);
	//
	//DisplayGizmo(&transform, aCamera, aViewportBounds, nullptr);
	//
	//
	//Vector3f delta = transform.GetPosition() - averagePos;
	//
	//for (void* data : aGizmo->myDatas)
	//{
	//	Vector3f* pos = (Vector3f*)data;
	//	*pos += delta;
	//}

	Transform averageTransform;
	averageTransform.SetMatrix(DirectX::XMMATRIX());

	for (Transform& trf : aGizmo->myTransforms)
	{
		averageTransform.SetMatrix(averageTransform.GetCUMatrix() + trf.GetCUMatrix());
	}
	averageTransform.SetMatrix(averageTransform.GetCUMatrix() * (1.0f / (float)aGizmo->myTransforms.size()));


	switch (aGizmo->myType)
	{
	case ArbitraryGizmo::Type::VECTOR3:
	{
		Transform deltaMatrix;
		DisplayGizmo(&averageTransform, aCamera, aViewportBounds, &deltaMatrix);

		for (void* data : aGizmo->myDatas)
		{
			Vector3f* pos = (Vector3f*)data;
			*pos += deltaMatrix.GetPosition();
		}

		for (Transform& trf : aGizmo->myTransforms)
		{
			trf.SetPosition(trf.GetPosition() + deltaMatrix.GetPosition());
		}

		break;
	}
	case ArbitraryGizmo::Type::TRANSFORM:
	{
		Transform deltaMatrix;
		DisplayGizmo(&averageTransform, aCamera, aViewportBounds, &deltaMatrix);

		for (void* data : aGizmo->myDatas)
		{
			Transform* trf = (Transform*)data;
			trf->SetMatrix(trf->GetMatrix() * deltaMatrix.GetMatrix());
		}

		for (Transform& trf : aGizmo->myTransforms)
		{
			trf.SetMatrix(trf.GetMatrix() * deltaMatrix.GetMatrix());
		}

		break;
	}
	}


	if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().KeyCtrl && !ImGuizmo::IsOver())
	{
		//arbitraryGizmo.Attach(false, ArbitraryGizmo::Type::NONE, nullptr);
	}

	return ImGuizmo::IsUsing();
}

void KE_EDITOR::ImGuiHandler::DisplayGizmo(Transform* aTransform, KE::Camera* aCamera, ImVec4 aViewportBounds, void* anOutDeltaMatrix)
{
	ImGuizmo::SetOrthographic(false);
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImGuizmo::SetDrawlist(drawList);

	ImGuizmo::SetRect(aViewportBounds.x, aViewportBounds.y, aViewportBounds.z, aViewportBounds.w);

	float w = 100.0f;
	float h = 140.0f;

	drawList->AddRectFilled(
		ImVec2(aViewportBounds.x, aViewportBounds.y),
		ImVec2(aViewportBounds.x + w, aViewportBounds.y + h),
		ImColor(0.0f, 0.0f, 0.0f, 0.78f),
		10.0f,
		ImDrawFlags_RoundCornersBottomRight
	);

	static ImGuizmo::OPERATION op = ImGuizmo::OPERATION::TRANSLATE;
	static ImGuizmo::MODE md = ImGuizmo::MODE::LOCAL;

	auto opT = ImGuizmo::OPERATION::TRANSLATE;
	auto opR = ImGuizmo::OPERATION::ROTATE;
	auto opS = ImGuizmo::OPERATION::SCALE;

	auto mdL = ImGuizmo::MODE::LOCAL;
	auto mdW = ImGuizmo::MODE::WORLD;

	if (!ImGui::IsMouseDown(ImGuiMouseButton_Right))
	{
		if (ImGui::IsKeyPressed(ImGuiKey_V, false)) { op = opT; }
		if (ImGui::IsKeyPressed(ImGuiKey_R, false)) { op = opR; }
		if (ImGui::IsKeyPressed(ImGuiKey_S, false)) { op = opS; }

		if (ImGui::IsKeyPressed(ImGuiKey_L, false)) { md = mdL; }
		if (ImGui::IsKeyPressed(ImGuiKey_W, false)) { md = mdW; }
	}

	ImGui::SetCursorScreenPos(ImVec2(aViewportBounds.x, aViewportBounds.y));
	if (ImGui::RadioButton("Translate", op == opT)) { op = opT; }
	if (ImGui::RadioButton("Rotate", op == opR)) { op = opR; }
	if (ImGui::RadioButton("Scale", op == opS)) { op = opS; }
	ImGui::NewLine();
	if (ImGui::RadioButton("Local", md == mdL)) { md = mdL; }
	if (ImGui::RadioButton("World", md == mdW)) { md = mdW; }


	//

	DirectX::XMMATRIX transform = aTransform->GetMatrix();

	Matrix4x4f viewMatrix = aCamera->GetViewMatrix();
	Matrix4x4f projectionMatrix = aCamera->GetProjectionMatrix();

	Matrix4x4f deltaMatrix;

	ImGuizmo::Manipulate(
		&viewMatrix(1, 1),
		&projectionMatrix(1, 1),
		op,
		md,
		(float*)&transform,
		(float*)&deltaMatrix
	);

	if (anOutDeltaMatrix != nullptr)
	{
		*(Matrix4x4f*)anOutDeltaMatrix = deltaMatrix;
	}

	static bool gizmoUsing = false;

	static DirectX::XMMATRIX lastTransform;

	if (ImGuizmo::IsUsing())
	{
		if (!gizmoUsing)
		{
			gizmoUsing = true;
		}

		aTransform->SetMatrix(transform);
	}
	else
	{
		if (gizmoUsing)
		{
			gizmoUsing = false;
		}
	}
}

bool KE_EDITOR::ImGuiHandler::BeginDisplayComponent(const std::string& aComponentName, KE::Component* aComponent)
{
	size_t componentID = (size_t)aComponent;
	ImGui::PushID((unsigned int)componentID);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(0.12f, 0.12f, 0.12f, 1.0f).Value);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

	//

	ImGui::BeginChild("ComponentChild", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);

	ImGui::PushID(aComponentName.c_str());
	if (DisplayComponentHeader(aComponentName, aComponent))
	{
		return true;
	}

	EndDisplayComponent();
	return false;
}

void KE_EDITOR::ImGuiHandler::EndDisplayComponent()
{
	ImGui::PopID();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(1);
	ImGui::PopID();
}


//COMPONENTS!

void KE_EDITOR::ImGuiHandler::ComponentDisplayDispatch(KE::Component* aComponent)
{
	KittyFlags componentFlags = eSeparatorAbove;

	//ImGui::PushID((unsigned long long)aComponent);
	//ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(0.12f, 0.12f, 0.12f, 1.0f).Value);
	//ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	////ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {5.0f, 5.0f});

	//ImGui::BeginChild("ComponentChild", ImVec2(0.0f, 0.0f), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);

	if (dynamic_cast<KE::ModelComponent*>(aComponent))
	{
		DisplayModelComponent((KE::ModelComponent*)aComponent, componentFlags);
	}
	else if (dynamic_cast<KE::SkeletalModelComponent*>(aComponent))
	{
		DisplaySkeletalModelComponent((KE::SkeletalModelComponent*)aComponent, componentFlags);
	}
	else if (dynamic_cast<KE::LightComponent*>(aComponent))
	{
		DisplayLightComponent((KE::LightComponent*)aComponent, componentFlags);
	}
	//else if (dynamic_cast<KE::AudioComponent*>(aComponent))
	//{
	//	DisplayAudioComponent((KE::AudioComponent*)aComponent, componentFlags);
	//}
	else if (dynamic_cast<KE::CameraComponent*>(aComponent))
	{
		DisplayCameraComponent((KE::CameraComponent*)aComponent, componentFlags);
	}
	else if (dynamic_cast<KE::BoxColliderComponent*>(aComponent))
	{
		DisplayBoxColliderComponent((KE::BoxColliderComponent*)aComponent, componentFlags);
	}
	else if (dynamic_cast<KE::SphereColliderComponent*>(aComponent))
	{
		DisplaySphereColliderComponent((KE::SphereColliderComponent*)aComponent, componentFlags);
	}
	else if (dynamic_cast<KE::PostProcessingComponent*>(aComponent))
	{
		DisplayPostProcessingComponent((KE::PostProcessingComponent*)aComponent, componentFlags);
	}
	else if (dynamic_cast<KE::VFXComponent*>(aComponent))
	{
		DisplayVFXComponent((KE::VFXComponent*)aComponent, componentFlags);
	}
	else if (dynamic_cast<KE::DecalComponent*>(aComponent))
	{
		DisplayDecalComponent((KE::DecalComponent*)aComponent, componentFlags);
	}
	else if (dynamic_cast<P8::Player*>(aComponent))
	{
		DisplayPlayerComponent((P8::Player*)aComponent, componentFlags);
	}
	else if (dynamic_cast<P8::ActionCameraComponent*>(aComponent))
	{
		DisplayActionCameraComponent((P8::ActionCameraComponent*)aComponent, componentFlags);
	}
	else if (dynamic_cast<P8::GameManager*>(aComponent))
	{
		DisplayGameManager((P8::GameManager*)aComponent, componentFlags);
	}
	else if (dynamic_cast<P8::LobbySystem*>(aComponent))
	{
		DisplayLobbySystem((P8::LobbySystem*)aComponent, componentFlags);
	}
	else if (auto* shellComp = dynamic_cast<KE::ShellTexturingComponent*>(aComponent))
	{
		DisplayShellTexturingComponent(shellComp, componentFlags);
	}
	else if (auto* tester = dynamic_cast<KE::TestComponent*>(aComponent))
	{
		BEGIN_DISPLAY_COMPONENT(aComponent);


		auto ser = KE_SER::Serializer::Serialize(*tester);

		for (auto& member : ser.myData)
		{
			editor->myInspectionSystem.Inspect(tester, member);
		}

		END_DISPLAY_COMPONENT();
	}
	else if (auto* boom = dynamic_cast<P8::BoomerangComponent*>(aComponent))
	{
		BEGIN_DISPLAY_COMPONENT(aComponent);

		float size = boom->GetGameObject().myWorldSpaceTransform.GetScale().x;
		if (ImGui::DragFloat("size", &size, 0.1f, 0.0f, 100.0f))
		{
			boom->SetSize(size);
		}

		END_DISPLAY_COMPONENT();
	}
	else if (auto* water = dynamic_cast<P8::WaterPlane*>(aComponent))
	{
		BEGIN_DISPLAY_COMPONENT(aComponent);

		bool updated = false;
		updated = ImGui::ColorEdit3("Water Fog Colour", &water->GetBufferData()->waterFogColour.x) ? true : updated;
		updated = ImGui::DragFloat("Water Fog Density", &water->GetBufferData()->waterFogDensity) ? true : updated;
		updated = ImGui::ColorEdit3("Caustic Colour", &water->GetBufferData()->causticColour.x) ? true : updated;
		updated = ImGui::DragFloat("Caustic Strength", &water->GetBufferData()->causticStrength) ? true : updated;

		if (updated)
		{
			water->UpdateBuffer();
		}


		END_DISPLAY_COMPONENT();
	}
	else
	{
		//ImGui::Text("Unknown Component");
	}

	//ImGui::EndChild();
	//ImGui::PopStyleColor();
	//ImGui::PopStyleVar(1);
	//ImGui::PopID();
}

void KE_EDITOR::ImGuiHandler::DisplayDecalComponent(KE::DecalComponent* aDecalComponent, KittyFlags aFlags)
{
	//COMPONENT_NAME(aDecalComponent);
	BEGIN_DISPLAY_COMPONENT(aDecalComponent);

	if(auto* decal = aDecalComponent->myDecalManager->GetDecal(aDecalComponent->myDecalIndex))
	{

		ImGui::Text("Decal Index: %i", aDecalComponent->myDecalIndex);
		ImGui::DragFloat4("Intensities", &decal->myTextureIntensities.x, 0.1f, 0.0f, 1.0f);

		DisplayMaterial(decal->myMaterial);
	}

	aDecalComponent->DrawDebug(*myDebugRenderer);

	END_DISPLAY_COMPONENT();
}

void KE_EDITOR::ImGuiHandler::DisplayModelComponent(KE::ModelComponent* aModelComponent, KittyFlags aFlags)
{
	//COMPONENT_NAME(aModelComponent);
	BEGIN_DISPLAY_COMPONENT(aModelComponent);

	KE::ModelData* modelData = aModelComponent->GetModelData();

	DisplayModelData(modelData);


	END_DISPLAY_COMPONENT();
}

bool KE_EDITOR::ImGuiHandler::DisplayComponentHeader(const std::string& aComponentName, KE::Component* aComponent)
{
	bool active = aComponent->IsActive();
	if (ImGui::Checkbox("##Active", &active))
	{
		aComponent->SetActive(active);
	}
	ImGui::SameLine(32);
	bool open = ImGui::TreeNodeEx(aComponentName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
	ImGui::SameLine();
	ImGui::SeparatorText("");
	if (open)
	{
		ImGui::TreePop();
		return true;
	}
	return false;
}



void KE_EDITOR::ImGuiHandler::DisplaySkeletalModelComponent(KE::SkeletalModelComponent* aSkeletalModelComponent, KittyFlags aFlags)
{
	//COMPONENT_NAME(aSkeletalModelComponent);
	BEGIN_DISPLAY_COMPONENT(aSkeletalModelComponent);

	//if (!BeginDisplayComponent("Skeletal Model Component", aSkeletalModelComponent)) { return; }

	KE::SkeletalModelData* modelData = aSkeletalModelComponent->GetModelData();
	DisplaySkeletalModelData(modelData);

	KE::AnimationPlayer& animationPlayer = aSkeletalModelComponent->GetAnimationPlayer();
	ImGui::SeparatorText("Animations");
	for (auto& pair : modelData->myAnimationClipMap)
	{
		bool current = pair.second == animationPlayer.currentAnimation.myClip;
		if (ImGui::RadioButton(pair.first.c_str(), current)) { animationPlayer.PlayAnimation(pair.first); }
		if (current)
		{
			float progress = animationPlayer.currentAnimation.myTime / animationPlayer.currentAnimation.myClip->duration;
			if (ImGui::SliderFloat("##animProgress", &progress, 0.0f, 1.0f, "%.2f"))
			{
				animationPlayer.currentAnimation.myTime = progress * animationPlayer.currentAnimation.myClip->duration;
			}
		}
	}

	DisplaySkeleton(&aSkeletalModelComponent->GetGameObject(), 0);

	END_DISPLAY_COMPONENT();
	//EndDisplayComponent();
}

void KE_EDITOR::ImGuiHandler::DisplayShadowRendering(KE::DeferredLightManager* aLightManager)
{
	ImGui::Separator();
	ImGui::Text("Shadow Settings");
	static Vector2i viewportSize = { 350, 200 };
	static Vector2f nearandfar = { -1000.0f, 500.0f };
	if (ImGui::DragInt2("Shadow Camera viewport size", &viewportSize.x, 1, 10, 8000))
	{
		aLightManager->InitShadowCamera(viewportSize, nearandfar.x, nearandfar.y);
	}
	if (ImGui::DragFloat("Shadow Camera near plane", &nearandfar.x, 5.0f, -2000, 0.0f))
	{
		aLightManager->InitShadowCamera(viewportSize, nearandfar.x, nearandfar.y);
	}
	if (ImGui::DragFloat("Shadow Camera far plane", &nearandfar.y, 5.0f, 1.0f, 2000.0f))
	{
		aLightManager->InitShadowCamera(viewportSize, nearandfar.x, nearandfar.y);
	}


	ImGui::Text("Dir Light Shadow Depth");
	ImGui::Image(aLightManager->myDepthBuffer.GetShaderResourceView(), ImVec2(256, 144));
}

void KE_EDITOR::ImGuiHandler::DisplayLightComponent(KE::LightComponent* aLightComponent, KittyFlags aFlags)
{
	//COMPONENT_NAME(aLightComponent);
	BEGIN_DISPLAY_COMPONENT(aLightComponent);

	KE::LightComponentData* componentData = (KE::LightComponentData*)aLightComponent->GetLightData();
	KE::LightData* lightData = componentData->myLightData;

	if (componentData->myLightType == KE::eLightType::Directional)
	{
		KE::DirectionalLightData* dlData = (KE::DirectionalLightData*)lightData;

		ImGui::Text("Directional Light");

		ImGui::ColorEdit3("Colour", &dlData->myColour.x);
		ImGui::DragFloat("Intensity", &dlData->myDirectionalLightIntensity, 0.1f, 0.0f, 100.0f);
		ImGui::DragFloat("Ambient Intensity", &dlData->myAmbientLightIntensity, 0.1f, 0.0f, 100.0f);

		DisplayShadowRendering(&editor->myGraphics->GetDeferredLightManager());
		//ImGui::Text("Dir Light RT");
		//ImGui::Image(editor->myGraphics->GetRenderTarget(6)->GetShaderResourceView(), ImVec2(256, 144));

	}
	else if (componentData->myLightType == KE::eLightType::Point)
	{
		KE::PointLightData* plData = (KE::PointLightData*)lightData;

		ImGui::Text("Point Light");

		ImGui::ColorEdit3("Colour", &plData->myColour.x);
		ImGui::DragFloat("Intensity", &plData->myIntensity, 0.1f, 0.0f, 1000.0f);
		ImGui::DragFloat("Range", &plData->myRange, 1.0f, 0.0f, 100.0f);
		static bool isPointDBGdraw = false;
		if (ImGui::Checkbox("Debug Draw", &isPointDBGdraw))
		{
			aLightComponent->ToggleDrawDebug();
		}
	}
	else if (componentData->myLightType == KE::eLightType::Spot)
	{
		KE::SpotLightData* slData = (KE::SpotLightData*)lightData;

		ImGui::Text("Spot Light");

		ImGui::ColorEdit3("Colour", &slData->myColour.x);
		ImGui::DragFloat("Intensity", &slData->myIntensity, 0.1f, 0.0f, 1000.0f);
		ImGui::DragFloat("Range", &slData->myRange, 1.0f, 0.0f, 100.0f);

		const float outerMax = slData->myOuterAngle - 0.01f;
		const float innerMax = slData->myInnerAngle + 0.01f;
		ImGui::DragFloat("Inner Angle", &slData->myInnerAngle, 0.1f, 0.0f, outerMax);
		ImGui::DragFloat("Outer Angle", &slData->myOuterAngle, 0.1f, innerMax, 179.0f);
	}


	PerformFlagsEnd();

	END_DISPLAY_COMPONENT();
}

//void KE_EDITOR::ImGuiHandler::DisplayAudioComponent(KE::AudioComponent* aAudioComponent, KittyFlags aFlags)
//{
//	//COMPONENT_NAME(aAudioComponent);
//	BEGIN_DISPLAY_COMPONENT(aAudioComponent);
//	END_DISPLAY_COMPONENT();
//}

void KE_EDITOR::ImGuiHandler::DisplayCameraComponent(KE::CameraComponent* aCameraComponent, KittyFlags aFlags)
{
	//COMPONENT_NAME(aCameraComponent);
	BEGIN_DISPLAY_COMPONENT(aCameraComponent);
	//

	KE::Camera* cam = aCameraComponent->GetCamera();

	DisplayEnumSlider("Control Mode", &aCameraComponent->myMode, KE::CameraComponentMode::Count);

	KE::ProjectionData projData = cam->GetProjectionData();

	auto& camType = cam->myType;
	DisplayEnumSlider("View Mode", &camType, KE::ProjectionType::Count);

	switch (camType)
	{
	case KE::ProjectionType::Perspective:
	{
		ImGui::Text("Perspective Camera");
		bool changed = false;
		float degFov = projData.perspective.fov * KE::RadToDegImmediate;
		if (ImGui::DragFloat("Fov", &degFov, 1.0f, 1.0f, 145.0f)) { changed = true; }
		if (ImGui::DragFloat("Near", &projData.perspective.nearPlane)) { changed = true; }
		if (ImGui::DragFloat("Far", &projData.perspective.farPlane)) { changed = true; }

		if (changed)
		{
			cam->SetPerspective(
				projData.perspective.width,
				projData.perspective.height,
				degFov * KE::DegToRadImmediate,
				projData.perspective.nearPlane,
				projData.perspective.farPlane
			);
		}

		break;
	}
	case KE::ProjectionType::Orthographic:
	{
		ImGui::Text("Orthographic Camera");

		bool changed = false;
		if (ImGui::DragFloat("Width", &projData.orthographic.width)) { changed = true; }
		if (ImGui::DragFloat("Height", &projData.orthographic.height)) { changed = true; }
		if (ImGui::DragFloat("Near", &projData.orthographic.nearPlane)) { changed = true; }
		if (ImGui::DragFloat("Far", &projData.orthographic.farPlane)) { changed = true; }

		if (changed)
		{
			cam->SetOrthographic(
				projData.orthographic.width,
				projData.orthographic.height,
				projData.orthographic.nearPlane,
				projData.orthographic.farPlane
			);
		}

		break;
	}
	}


	editor->myGraphics->AppendRenderTargetQueue(5, cam->myIndex);
	ImGui::Image(
		editor->myGraphics->GetRenderTarget(5)->GetShaderResourceView(),
		GetAspectRegion(
			ImGui::GetContentRegionAvail(),
			(float)editor->myGraphics->GetRenderWidth() / (float)editor->myGraphics->GetRenderHeight()
		)
	);

	END_DISPLAY_COMPONENT();
}

void KE_EDITOR::ImGuiHandler::DisplayBoxColliderComponent(KE::BoxColliderComponent* aBoxColliderComponent, KittyFlags aFlags)
{
	//COMPONENT_NAME(aBoxColliderComponent);
	BEGIN_DISPLAY_COMPONENT(aBoxColliderComponent);


	aBoxColliderComponent->DrawDebug(*myDebugRenderer);

	PerformFlagsEnd();

	END_DISPLAY_COMPONENT();
}

void KE_EDITOR::ImGuiHandler::DisplaySphereColliderComponent(KE::SphereColliderComponent* aSphereColliderComponent, KittyFlags aFlags)
{
	//COMPONENT_NAME(aSphereColliderComponent);
	BEGIN_DISPLAY_COMPONENT(aSphereColliderComponent);

	if (ImGui::DragFloat3("Offset##SphereColOffset", &aSphereColliderComponent->myOffset.x, 0.25f))
	{
		aSphereColliderComponent->myCollider->UpdateOffset(aSphereColliderComponent->myOffset);
	}

	END_DISPLAY_COMPONENT();
}

void KE_EDITOR::ImGuiHandler::DisplayPostProcessingComponent(KE::PostProcessingComponent* aPostProcessingComponent, KittyFlags aFlags)
{
	//COMPONENT_NAME(aPostProcessingComponent);
	BEGIN_DISPLAY_COMPONENT(aPostProcessingComponent);

	KE::PostProcessing* pp = aPostProcessingComponent->GetPostProcessing();

	DisplayPostProcessing(pp);
	PerformFlagsEnd();

	END_DISPLAY_COMPONENT();

}

void KE_EDITOR::ImGuiHandler::DisplayVFXComponent(KE::VFXComponent* aVFXComponent, KittyFlags aFlags)
{
	//COMPONENT_NAME(aVFXComponent);
	BEGIN_DISPLAY_COMPONENT(aVFXComponent);

	ImGui::BeginTable("##sequenceTable", 5);
	//ImGui::SeparatorText("VFXSequences");
	for (int i = 0; i < aVFXComponent->GetVFXSequenceCount(); i++)
	{
		//ImGui::Separator();
		ImGui::PushID(i);
		int vfxSequenceIndex = aVFXComponent->GetVFXSequenceIndex(i);
		KE::VFXSequence* sequence = editor->myGraphics->GetVFXManager().GetVFXSequence(vfxSequenceIndex);
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("%s (%i)", sequence->myName.c_str(), vfxSequenceIndex);
		ImGui::TableNextColumn();
		if (ImGui::Button("Play")) { aVFXComponent->TriggerVFX(i); }
		ImGui::TableNextColumn();
		if (ImGui::Button("Loop")) { aVFXComponent->TriggerVFX(i, true); }
		ImGui::TableNextColumn();
		if (ImGui::Button("Stop")) { aVFXComponent->StopVFX(i); }
		ImGui::TableNextColumn();
		if (ImGui::Button("Edit")) { vfxEditorActiveSequenceIndex = vfxSequenceIndex; }
		ImGui::PopID();
	}
	ImGui::EndTable();

	END_DISPLAY_COMPONENT();

}

void KE_EDITOR::ImGuiHandler::DisplayPlayerComponent(P8::Player* aPlayer, KittyFlags aFlags)
{
	BEGIN_DISPLAY_COMPONENT(aPlayer);

	if (ImGui::Button("Reset"))
	{
		aPlayer->SetPosition({0.0f, 1.0f, 0.0f});
		aPlayer->SetPlayerState<P8::PlayerIdleState>();
	}


	P8::PowerupManager* powerupMGR = aPlayer->myPowerups.manager;

	for (auto* identity : powerupMGR->GetPowerupIdentities())
	{
		P8::Powerup* matchingPowerup = nullptr;
		for (auto* powerup : aPlayer->myPowerups.powerups)
		{
			if (identity->IsType(powerup))
			{
				matchingPowerup = powerup;
				break;
			}
		}

		bool hasPowerup = matchingPowerup != nullptr;
		ImGui::PushID(identity->name.c_str());
		if (ImGui::Checkbox("##box", &hasPowerup))
		{
			if (hasPowerup)
			{
				aPlayer->myPowerups.AddPowerup(identity->CreateInstance());
			}
			else
			{
				aPlayer->myPowerups.RemovePowerup(matchingPowerup);
			}
		}
		ImGui::SameLine();
		ImGui::Image(
			editor->myGraphics->GetTextureLoader().GetTextureFromPath(identity->icon)->myShaderResourceView.Get(),
			{32.0f, 32.0f}
		);
		ImGui::SameLine();
		ImGui::Text(identity->name.c_str());

		ImGui::PopID();
	}

	int powerupMask = aPlayer->myPowerups.GetPowerupMask();
	if (ImGui::InputInt("Powerup Mask", &powerupMask))
	{
		aPlayer->myPowerups.SetPowerupsFromMask(powerupMask);
	}

	int idx = aPlayer->myPlayerIndex;
	if (ImGui::SliderInt("Player Index", &idx, 0, 3))
	{
		aPlayer->myPlayerIndex = static_cast<unsigned int>(idx);
	}

	int charIdx = aPlayer->myCharacterIndex;
	if (ImGui::SliderInt("Character Index", &charIdx, 1, 5))
	{
		aPlayer->SetCharacterIndex(charIdx);
	}


	auto& movementData = aPlayer->GetPhysxController().GetMovementData();

	ImGui::Text("Tweak movement variables");
	ImGui::SliderFloat("MaxSpeed", &movementData.maxSpeed, 0.0f, 50.f, "%.1f");
	ImGui::SliderFloat("MoveSpeed", &movementData.linearAcceleration, 0.0f, 150.f, "%.1f");
	ImGui::SliderFloat("RotateSpeed", &movementData.angularAcceleration, 0.0f, 50.f, "%.1f");
	ImGui::SliderFloat("MaxRotation", &movementData.maxRotation, 0.0f, 50.f, "%.1f");
	ImGui::SliderFloat("Drag", &movementData.decayMagnitude, 0.0f, 50.f, "%.1f");
	ImGui::DragFloat("ThrowCharge", &aPlayer->myThrowCharge);
	ImGui::DragFloat("ThrowStrength", &aPlayer->myThrowStrength);
	ImGui::DragFloat("ThrowBuildup", &aPlayer->myThrowBuildupSpeed);

	ImGui::SeparatorText("Boomerang");
	auto& bp = aPlayer->myBallThrowParams;
	ImGui::DragFloat("Return Delay", &bp.returnDelay, 0.1f, 0.1f, 2.0);
	ImGui::DragFloat("Pickup Range", &bp.pickupRadius, 0.1f, 0.1f, 5.0f);
	ImGui::DragFloat("Max Speed", &bp.maxSpeed, 0.1f, 20.0f, 200.0f);
	ImGui::DragFloat("Slow Radius", &bp.slowRadius, 0.1f, 0.1f, 8.0f);
	ImGui::DragFloat("Dying Threshold", &bp.dyingThreshold, 0.1f, 20.0f, 100.0f);
	ImGui::DragFloat("Min Bounce value ", &bp.minBounceValue, 0.1f, 5.0f, 20.0f);
	ImGui::DragFloat("Max Bounce value ", &bp.maxBounceValue, 0.1f, 5.0f, 50.0f);
	ImGui::DragFloat("Dying DecayStrength", &bp.decayStrength, 0.1f, 1.0f, 20.0f);

	END_DISPLAY_COMPONENT();
}

void KE_EDITOR::ImGuiHandler::DisplayShellTexturingComponent(KE::ShellTexturingComponent* aShellComponent, KittyFlags aFlags)
{
	BEGIN_DISPLAY_COMPONENT(aShellComponent);

	auto& shellAttr = aShellComponent->shellModelData->attributes;
	auto& shellMD = aShellComponent->shellModelData->modelData;

	ImGui::DragInt("Shell Count", &shellAttr.shellCount);
	ImGui::DragFloat("Height", &shellAttr.totalHeight);
	ImGui::DragFloat("Thickness", &shellAttr.thickness);
	ImGui::DragFloat("Density", &shellAttr.density);
	ImGui::DragFloat("noiseMin", &shellAttr.noiseMin);
	ImGui::DragFloat("noiseMax", &shellAttr.noiseMax);
	ImGui::DragFloat("aoExp", &shellAttr.aoExp);

	ImGui::ColorEdit3("Bottom", shellAttr.bottomColour);
	ImGui::ColorEdit3("Top", shellAttr.topColour);

	ImGui::DragInt("ShellCountPerDC", &shellAttr.shellCountPerDrawCall);

	//KE_EDITOR::ImGuiHandler::DisplayTransform(reinterpret_cast<Transform*>(shellMD.myTransform));

	//KE_EDITOR::ImGuiHandler::DisplayModelData(&shellMD);


	static KE::Graphics* gfx = KE_GLOBAL::blackboard.Get<KE::Graphics>();
	if (KE::RenderTarget* rt = aShellComponent->shellModelData.Get()->effectsRT)
	{
		ImGui::PushID("ModificationTex");
		ImGui::Image(rt->GetShaderResourceView(), { 256.0f, 256.0f });
		if (ImGui::Button("Reset")) { aShellComponent->ClearDisplacement(); }
		static bool isEditing = false;
		ImGui::SameLine();
		ImGui::Checkbox("Edit", &isEditing);
		ImGui::SameLine();
		if (ImGui::Button("Save"))
		{
			std::string folder = "Data/InternalAssets/ShellDisplacements/";

			std::string fileName = std::format(
				"Displacement_{}.dds",
				editor->mySceneManager->GetCurrentScene()->sceneName
			);

			std::string path = std::format("{}{}", folder, fileName);
			gfx->GetTextureLoader().SaveTextureToFile(
				path,
				rt->GetShaderResourceView(),
				1024,
				1024
			);
		}

		if (isEditing)
		{
			static float radius = 1.0f;
			static float strength = 1.0f;
			static float falloff = 1.0f;

			if (ImGui::GetIO().KeyCtrl)
			{
				float* toMod = &radius;
				if (ImGui::GetIO().KeyShift)
				{
					toMod = &strength;
				}
							
				*toMod+= ImGui::GetIO().MouseWheel * 0.05f;
				if (*toMod <= 0.0f) { *toMod = 0.05f; }
			
			}

			KE::Camera* viewingCamera = gfx->GetCameraManager().GetHighlightedCamera();

			auto mp= KE_GLOBAL::blackboard.Get<KE::InputHandler>("inputHandler")->GetMousePosition();
			Rayf painterRay = viewingCamera->GetRay({(float)mp.x, (float)mp.y});

			Vector3f planeHit = painterRay.GetOrigin() + painterRay.GetDirection()* (-painterRay.GetOrigin().y / painterRay.GetDirection().y);

			auto& dbg = gfx->GetDebugRenderer();
			Transform painterTransform;
			painterTransform.SetPosition(planeHit);
			painterTransform.SetScale(Vector3f(radius, strength, radius));

			dbg.RenderSphere(painterTransform, radius);

			if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				KE::ShellTextureDisplacement displacement;
				displacement.position = planeHit;
				displacement.scale = Vector3f(radius, strength, radius);
				//std::vector<KE::ShellTextureDisplacement> dispVec;
				//dispVec.push_back(displacement);

				gfx->GetShellTexturedRenderer()->RenderDisplacement(
					aShellComponent->shellModelData,
					{ displacement },
					aShellComponent->sharedBounds.min,
					aShellComponent->sharedBounds.max,
					nullptr,
					(ImGui::GetIO().KeyAlt ? 2 : 1) * (ImGui::GetIO().KeyCtrl ? -1 : 1)
				);
			}
			
		}

		ImGui::PopID();
	}


	END_DISPLAY_COMPONENT();
}
void KE_EDITOR::ImGuiHandler::DisplayGameManager(P8::GameManager* aComponent, KittyFlags aFlags)
{
	BEGIN_DISPLAY_COMPONENT(aComponent);

	ImGui::Text("Old Gamestate: %s", EnumToString(aComponent->myOldGameState).c_str());
	ImGui::Text("Current Gamestate: %s", EnumToString(aComponent->myCurrentGameState).c_str());
	ImGui::Spacing();

	ImGui::SliderInt("Points Per Kill: ", &aComponent->myGameData.pointsPerKill,1, 10);

	ImGui::Spacing();

	for (int i = 0; i < aComponent->myGameData.numberOfPlayers; i++)
	{
		ImGui::Text("Player %i : %i points", i, aComponent->myGameData.scores[i]);
	}

	ImGui::Spacing();
	ImGui::Text("Players Alive:");
	for (int i = 0; i < 4; i++)
	{
		ImGui::Text("Player %i : %s", i, aComponent->myGameData.playersAlive[i] > 0 ? "true" : "false");
	}

	ImGui::Spacing();
	static P8::eGameStates gameState = P8::eGameStates::eUnknown;
	if (ImGui::BeginCombo("GameState to Change", EnumToString(gameState).c_str()))
	{
		for (unsigned int i = 0; i < (int)P8::eGameStates::Count; i++)
		{
			if (ImGui::Selectable(EnumToString((P8::eGameStates)i).c_str()))
			{
				gameState = (P8::eGameStates)i;
				aComponent->ChangeGameState(gameState);
			}
		}

		ImGui::EndCombo();
	}

	if (ImGui::Button("Increment Level"))
	{
		aComponent->DEBUGIncrementLevelTest();
	}


	END_DISPLAY_COMPONENT();
}

void KE_EDITOR::ImGuiHandler::DisplayLobbySystem(P8::LobbySystem* aComponent, KittyFlags aFlags)
{
	BEGIN_DISPLAY_COMPONENT(aComponent);

	ImGui::Text("Cheat Lobby Active? %s", aComponent->isCheatLobbyActive ? "True" : "False");

	END_DISPLAY_COMPONENT();
}


void KE_EDITOR::ImGuiHandler::DisplayActionCameraComponent(P8::ActionCameraComponent* aComponent, KittyFlags aFlags)
{
	BEGIN_DISPLAY_COMPONENT(aComponent);

	ImGui::Checkbox("Is Debug Camera", &aComponent->isDebugCamera);
	ImGui::Checkbox("Use target Pos", &aComponent->useTargetPosition);
	ImGui::DragFloat3("Debug Target Pos", &aComponent->debugTargetPos.x, 0.01f);

	ImGui::DragFloat("Shake Factor", &aComponent->myShakeFactor, 0.01f, 0.0f, 0.1f);


	auto& data = aComponent->mySettings;

	ImGui::DragFloat("Minimum Distance From Ground", &data.minDistanceFromGround, 0.1f, 0.0f, data.maxDistanceFromGround - 1.0f);
	ImGui::DragFloat("Maximum Distance From Ground", &data.maxDistanceFromGround, 0.1f, data.minDistanceFromGround + 1.0f, 100.0f);
	ImGui::DragFloat("Outer border Distance", &data.outerBorderDistancePercentage, 0.5f, 0.0f, 150.0f);
	ImGui::DragFloat("Inner border padding", &data.innerBorderDistancePercentage, 0.5f, 0.0f, 150.0f);
	ImGui::DragFloat("Horizontal movement Speed", &data.horizontalMovementSpeed, 0.1f, 0.0f, 50.0f);
	ImGui::DragFloat("Vertical movement Speed", &data.verticalMovementSpeed, 0.1f, 0.0f, 50.0f);
	if (ImGui::SliderFloat("Camera FOV", &data.cameraFOV, 30.0f, 145.0f))
	{
		aComponent->myCameraComponent->GetCamera()->SetFOV(KE::DegToRadImmediate * data.cameraFOV);
	}
	if (ImGui::DragFloat3("Camera Direction", &data.cameraDirection.x, 1.0f, -180.0f, 180.0f))
	{
		aComponent->myGameObject.myTransform.SetRotation(KE::DegToRadImmediate * data.cameraDirection);
	}

	if (ImGui::Button("Save Settings"))
	{
		P8::CameraSettingsFile::Save(data);
	}
	if (ImGui::Button("Load Settings"))
	{
		P8::CameraSettingsData tempData;
		if (P8::CameraSettingsFile::Load(&tempData))
		{
			data = tempData;
			aComponent->ApplyLoadedSettings();
		}
	}
	if (ImGui::Button("Delete Settings"))
	{
		P8::CameraSettingsFile::Delete();
	}

	END_DISPLAY_COMPONENT();

}

//
void KE_EDITOR::ImGuiHandler::DisplaySkeleton(KE::GameObject* aGameObject, KittyFlags aFlags)
{
	auto& skeletalModel = aGameObject->GetComponent<KE::SkeletalModelComponent>();
	//auto& debugRenderer = aGameObject->GetManager().GetScene()->myDebugRenderer;
	auto& worldSpaceTransforms = skeletalModel.GetModelData()->myWorldSpaceJoints;
	auto& bones = skeletalModel.GetModelData()->mySkeleton->myBones;

	constexpr float size = 0.01f;
	static Vector3f v;

	DirectX::XMMATRIX objTransform = aGameObject->myTransform.GetMatrix();
	DirectX::XMMATRIX scl = DirectX::XMMatrixScaling(size, size, size);

	for (int i = 0; i < bones.size(); i++)
	{
		Transform currentBoneTransform = worldSpaceTransforms[i];

		customHandles.push_back({ nullptr, ArbitraryGizmo::Type::VECTOR3, &v, currentBoneTransform, FormatString("%i - %s", i, bones[i].myName.c_str()) });

		int parentIndex = bones[i].myParentIndex;
		if (parentIndex > -1)
		{
			Transform parentBoneTransform = worldSpaceTransforms[parentIndex];

			myDebugRenderer->RenderLine(currentBoneTransform.GetPosition(), parentBoneTransform.GetPosition());
		}
	}

}

void KE_EDITOR::ImGuiHandler::DisplayParticleEmitter(KE::ParticleEmitter* anEmitter, KittyFlags aFlags)
{
	DisplayTexture(&anEmitter->mySpriteBatch.myData.myTexture, ImVec2(64.0f, 64.0f));

	if (ImGui::RadioButton("Default",
		anEmitter->mySpriteBatch.myData.myMode == KE::SpriteBatchMode::Default))
	{
		anEmitter->mySpriteBatch.myData.myMode = KE::SpriteBatchMode::Default;
	}

	if (ImGui::RadioButton("Billboard",
		anEmitter->mySpriteBatch.myData.myMode == KE::SpriteBatchMode::Billboard))
	{
		anEmitter->mySpriteBatch.myData.myMode = KE::SpriteBatchMode::Billboard;
	}

	if (PerformFlagsBegin("Particle Emitter", aFlags))
	{
		ImGui::DragFloat2("Burst Time", &anEmitter->mySharedParticleAttributes.burstTimeMin);
		ImGui::DragInt2("Burst Count", &anEmitter->mySharedParticleAttributes.burstCountMin);
		ImGui::DragFloat2("Velocity", &anEmitter->mySharedParticleAttributes.velocityMin);
		ImGui::DragFloat2("Acceleration", &anEmitter->mySharedParticleAttributes.accelerationMin);
		ImGui::DragFloat2("Life Time", &anEmitter->mySharedParticleAttributes.lifeTimeMin);
		ImGui::DragFloat2("Angle", &anEmitter->mySharedParticleAttributes.angleMin);
		ImGui::DragFloat2("Velocity Factor", &anEmitter->mySharedParticleAttributes.horizontalVelocityFactor);
		ImGui::Separator();
		ImGui::Text("Particle Count: %d", anEmitter->myParticleCapacity - anEmitter->myFreeParticleIndices.size());
		ImGui::Text("Free Particle Count: %d", anEmitter->myFreeParticleIndices.size());


		if (ImGui::TreeNode("Colour"))
		{
			ImGui::ColorEdit4("Start Colour", &anEmitter->mySharedParticleAttributes.startColor.x);
			ImGui::ColorEdit4("Mid Colour", &anEmitter->mySharedParticleAttributes.midColor.x);
			ImGui::ColorEdit4("End Colour", &anEmitter->mySharedParticleAttributes.endColor.x);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Size"))
		{
			ImGui::DragFloat("Start Size", &anEmitter->mySharedParticleAttributes.startSize);
			ImGui::DragFloat("Mid Size", &anEmitter->mySharedParticleAttributes.midSize);
			ImGui::DragFloat("End Size", &anEmitter->mySharedParticleAttributes.endSize);

			ImGui::TreePop();
		}

		PerformFlagsEnd();
	}
}

void KE_EDITOR::ImGuiHandler::DisplayPostProcessing(KE::PostProcessing* aPostProcessing, KittyFlags aFlags)
{
	if (PerformFlagsBegin("Post Processing", aFlags))
	{
		if (ImGui::TreeNode("Gaussian Blur settings"))
		{
			ImGui::Checkbox("Activate Gaussian Blur", &aPostProcessing->myGaussianActive);
			ImGui::SliderFloat("Direction##GaussianDirection", &aPostProcessing->myAttributes.gaussianDirection, 10.0f, 25.0f);
			ImGui::SliderFloat("Quality##GaussianQuality", &aPostProcessing->myAttributes.gaussianQuality, 2.5f, 25.0f);
			ImGui::SliderFloat("Size##GaussianSize", &aPostProcessing->myAttributes.gaussianSize, 0.0f, 100.0f);
			ImGui::SliderFloat("Treshold##GaussianTreshold", &aPostProcessing->myAttributes.gaussianTreshold, 0.0f, 1.0f);

			ImGui::TreePop();
		}

		ImGui::Separator();

		if (ImGui::TreeNode("Colour Correction Settings"))
		{
			ImGui::SliderFloat("Saturation##CCSaturation", &aPostProcessing->myAttributes.saturation, 0.0f, 5.0f);
			ImGui::SliderFloat("Exposure##CCExposure", &aPostProcessing->myAttributes.exposure, -5.0f, 5.0f);
			ImGui::SliderFloat("Contrast##CCContrast", &aPostProcessing->myAttributes.contrast, -5.0f, 5.0f);
			if (ImGui::TreeNode("Tint Settings"))
			{
				ImGui::ColorPicker4("Tint##CCTint", &aPostProcessing->myAttributes.tint.x);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("BlackPoint Settings"))
			{
				ImGui::ColorPicker4("BlackPoint##CCBlackPoint", &aPostProcessing->myAttributes.blackPoint.x);
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Colour Correction Settings"))
			{
				ImGui::ColorPicker3("Colour Correction", &aPostProcessing->myAttributes.colourCorrecting.x);
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}

		ImGui::Separator();


		if (ImGui::TreeNode("Chromatic Aberration Settings"))
		{
			ImGui::SliderFloat("Strength##CAMultiplier", &aPostProcessing->myAttributes.CAMultiplier, 0.0f, 5.0f);
			ImGui::SliderFloat2("Red Offset##CARedOffset", &aPostProcessing->myAttributes.CARedOffset.x, -5.0f, 5.0f);
			ImGui::SliderFloat2("Green Offset##CAGreenOffset", &aPostProcessing->myAttributes.CAGreenOffset.x, -5.0f, 5.0f);
			ImGui::SliderFloat2("Blue Offset##CABlueOffset", &aPostProcessing->myAttributes.CABlueOffset.x, -5.0f, 5.0f);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Bloom Settings"))
		{
			ImGui::SliderInt("Number of downsamples##bloomDownSamples", &aPostProcessing->myNumberOfDownSamples, 1, MAX_NUMBER_OF_DOWNSAMPLES);
			ImGui::SliderFloat("Blending##bloomBending", &aPostProcessing->myAttributes.bloomBlending, 0.0f, 1.0f);
			ImGui::SliderFloat("Alpha Treshold##bloomTreshold", &aPostProcessing->myAttributes.bloomTreshold, 0.0f, 1.0f);
			ImGui::ColorEdit3("Bloom Sample Pass", &aPostProcessing->myAttributes.bloomSampleTreshold.x);

			ImGui::Spacing();
			if (ImGui::TreeNode("Bloom Sample"))
			{
				static Vector2i bloomWindowSize = { 256, 144 };
				ImGui::DragInt("Window Width##BloomSampleWindowSize", &bloomWindowSize.x, 5, 0, 512);
				bloomWindowSize.y = static_cast<int>((float)bloomWindowSize.x * 0.5625f);
				ImGui::Image(aPostProcessing->myPrePostProcessSample.GetShaderResourceView(), ImVec2((float)bloomWindowSize.x, (float)bloomWindowSize.y));
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Vignette"))
		{
			ImGui::SliderFloat("Size##VignetteSize", &aPostProcessing->myAttributes.vignetteSize, 0.0f, 1.0f);
			ImGui::SliderFloat("Feather Thickness##VignetteFeatherThickness", &aPostProcessing->myAttributes.vignetteFeatherThickness, 0.0f, 1.0f);
			ImGui::SliderFloat("Intensity##VignetteIntensity", &aPostProcessing->myAttributes.vignetteIntensity, 0.0f, 1.0f);
			ImGui::SliderInt("Show Mask##VignetteShowMask", &aPostProcessing->myAttributes.vignetteShowMask, 0, 1);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("ToneMap"))
		{
			ImGui::SliderFloat("ToneMap Intensity", &aPostProcessing->myAttributes.toneMapIntensity, 0.0f, 1.0f);
			ImGui::TreePop();
		}

		PerformFlagsEnd();
	}
}

void KE_EDITOR::ImGuiHandler::DisplayShaderList()
{
	//if (ImGui::Begin("All Shaders"))
	{
		ImGui::SeparatorText("Vertex Shaders");
		for (auto& vs : editor->myShaderLoader->GetVertexShaders())
		{
			KE::Shader* shader = (KE::Shader*)&vs.second;
			DisplayShader(&shader, true, false);
		}

		ImGui::SeparatorText("Pixel Shaders");
		for (auto& ps : editor->myShaderLoader->GetPixelShaders())
		{
			KE::Shader* shader = (KE::Shader*)&ps.second;

			DisplayShader(&shader, true, false);
		}
	}
	//ImGui::End();
}

void KE_EDITOR::ImGuiHandler::DisplayShader(KE::Shader** aShader, bool aDragSource, bool aDragTarget)
{
	KE::Shader*& underlyingShader = *aShader;
	if (underlyingShader->myName.empty()) { return; }

	const size_t lastSlash = underlyingShader->myName.find_last_of("/");
	const std::string beautifiedShaderName = underlyingShader->myName.substr(lastSlash+1, underlyingShader->myName.length() - lastSlash - 5);

	switch(underlyingShader->myShaderType)
	{
	case KE::Shader::ShaderType::eVertexShader:
	{
		ImGui::Text("[VS]"); 
		ImGui::SameLine();
		if (ImGui::Button(beautifiedShaderName.c_str()))
		{
			
		}

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("vertexShaderPayload", underlyingShader->myName.c_str(), strlen(underlyingShader->myName.c_str()) + 1);
			ImGui::Text(beautifiedShaderName.c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("vertexShaderPayload"))
			{
				std::string shaderName = (char*)payload->Data;
				KE::Shader* shader = editor->myShaderLoader->GetVertexShader(shaderName);
				underlyingShader = shader;
			}
			ImGui::EndDragDropTarget();
		}


		break;
	}
	case KE::Shader::ShaderType::ePixelShader:
	{
		ImGui::Text("[PS]");
		ImGui::SameLine();
		if (ImGui::Button(beautifiedShaderName.c_str()))
		{

		}

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("pixelShaderPayload", underlyingShader->myName.c_str(), strlen(underlyingShader->myName.c_str()) + 1);
			ImGui::Text(beautifiedShaderName.c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("pixelShaderPayload"))
			{
				std::string shaderName = (char*)payload->Data;
				KE::Shader* shader = editor->myShaderLoader->GetPixelShader(shaderName);
				*aShader = shader;
			}
			ImGui::EndDragDropTarget();
		}

		break;
	}
	}
}

void KE_EDITOR::ImGuiHandler::DisplayModelData(KE::ModelData* aModelData, KittyFlags aFlags)
{
	if (PerformFlagsBegin("Model Data", aFlags))
	{
		if (ImGui::BeginTabBar("MeshTabBar"))
		{
			if (ImGui::TabItemButton(FormatString("Mesh: %s:", GetFileNameFromPath(aModelData->myMeshList->myFilePath))))
			{
				ImGui::OpenPopup("MeshSelectorPopup");
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("modelPath"))
				{
					std::string meshPath = (char*)payload->Data;
					KE_LOG("Editor", "DragDrop Mesh Path: %s", meshPath.c_str());
					KE::MeshList& meshList = editor->myModelLoader->Load(meshPath);
					aModelData->myMeshList = &meshList;
				}

				ImGui::EndDragDropTarget();
			}

			if (ImGui::BeginPopup("MeshSelectorPopup"))
			{
				for (auto& mesh : editor->myModelLoader->GetMeshes())
				{
					if (ImGui::Selectable(mesh.first.c_str()))
					{
						aModelData->myMeshList = &mesh.second;
					}
				}

				ImGui::EndPopup();
			}

			for (int i = 0; i < aModelData->myMeshList->myMeshes.size(); i++)
			{
				std::string materialName = aModelData->myMeshList->myMaterialNames[i];
				if (ImGui::BeginTabItem(FormatString("%i", i)))
				{
					auto& rrToUse = aModelData->myRenderResources[i >= aModelData->myRenderResources.size() ? 0 : i];

					DisplayMaterial(rrToUse.myMaterial);

					ImGui::SeparatorText("Shaders");
					DisplayShader((KE::Shader**)&rrToUse.myVertexShader, true, true);
					DisplayShader((KE::Shader**)&rrToUse.myPixelShader, true, true);

					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();


		}
		PerformFlagsEnd();
	}
}

void KE_EDITOR::ImGuiHandler::DisplaySkeletalModelData(KE::SkeletalModelData* aModelData, KittyFlags aFlags)
{
	if (PerformFlagsBegin("Skeletal Model Data", aFlags))
	{
		ImGui::Checkbox("outline", &aModelData->myRenderOutline);
		//ImGui::Combo("Render Layer: ", (int*)&aModelData->myRenderLayer, KE::eRenderLayersStrings, static_cast<int>(KE::eRenderLayers::Count));
		if (ImGui::BeginTabBar("MeshTabBar"))
		{
			if (ImGui::TabItemButton(FormatString("Mesh: %s:", GetFileNameFromPath(aModelData->myMeshList->myFilePath))))
			{
				ImGui::OpenPopup("MeshSelectorPopup");
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("skeletalModelPath"))
				{
					std::string meshPath = (char*)payload->Data;
					KE_LOG("Editor", "DragDrop Mesh Path: %s", meshPath.c_str());

					editor->myModelLoader->LoadSkeletalModel(*aModelData, meshPath);

				}

				ImGui::EndDragDropTarget();
			}

			if (ImGui::BeginPopup("MeshSelectorPopup"))
			{
				for (auto& mesh : editor->myModelLoader->GetSkeletalMeshes())
				{
					if (ImGui::Selectable(mesh.first.c_str()))
					{
						aModelData->myMeshList = &mesh.second;
					}
				}

				ImGui::EndPopup();
			}

			for (int i = 0; i < aModelData->myMeshList->myMeshes.size(); i++)
			{
				std::string materialName = aModelData->myMeshList->myMaterialNames[i];
				if (ImGui::BeginTabItem(FormatString("%i", i)))
				{
					ImGui::BeginChild("##modelRenderResourceChild", {0,0}, ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);
					DisplayMaterial(aModelData->myRenderResources[i >= aModelData->myRenderResources.size() ? 0 : i].myMaterial);

					ImGui::SeparatorText("Shaders");
					DisplayShader((KE::Shader**)&aModelData->myRenderResources[i >= aModelData->myRenderResources.size() ? 0 : i].myVertexShader, true, true);
					DisplayShader((KE::Shader**)&aModelData->myRenderResources[i >= aModelData->myRenderResources.size() ? 0 : i].myPixelShader, true, true);
					ImGui::EndChild();
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();

		}
		PerformFlagsEnd();
	}
}

void KE_EDITOR::ImGuiHandler::DisplayTexture(KE::Texture** aTexture, ImVec2 aSize, KittyFlags aFlags)
{
	if (PerformFlagsBegin("Texture", aFlags))
	{
		bool pressed = ImGui::ImageButton(
			KE::TextureTypeStrings[(int)(*aTexture)->myMetadata.myTextureType].c_str(),
			(*aTexture)->myShaderResourceView.Get(),
			aSize
		);

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("texturePath"))
			{
				std::string texturePath = (char*)payload->Data;
				KE_LOG("Editor", "DragDrop Texture Path: %s", texturePath.c_str());
				KE::Texture* tex = editor->myTextureLoader->GetTextureFromPath(texturePath);
				*aTexture = tex;
			}
			ImGui::EndDragDropTarget();
		}

		bool hovered = ImGui::IsItemHovered();

		ImGui::SameLine();
		ImVec2 curPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos(curPos + ImVec2(0.0f, 0.0f));
		ImGui::Text(KE::TextureTypeStrings[(int)(*aTexture)->myMetadata.myTextureType].c_str());
		ImGui::SetCursorPos(curPos + ImVec2(0.0f, 16.0f));

		if (ImGui::BeginCombo("##texture", (*aTexture)->myMetadata.myFileName.c_str()))
		{
			for (auto& pair : editor->myTextureLoader->GetLoadedTextures())
			{
				if (ImGui::Selectable(pair.first.c_str()))
				{
					*aTexture = (KE::Texture*)&pair.second;
				}
			}

			ImGui::EndCombo();
		}

		ImGui::SetCursorPos(curPos + ImVec2(0.0f, 40.0f));

		if ((*aTexture)->myMetadata.failed)
		{
			/*ImGui::TextWrapped(
				"Texture failed to load, using default %s texture.",
				KE::TextureTypeStrings[(int)(*aTexture)->myMetadata.myTextureType].c_str()
			);*/
		}

		ImGui::Dummy(ImVec2(0.0f, 40.0f));

		if (hovered)
		{
			ImGui::BeginTooltip();
			ImGui::Image(
				/*aTexture->myMetadata.myTextureType.c_str(),*/
				(*aTexture)->myShaderResourceView.Get(),
				ImVec2(256, 256)
			);
			ImGui::Text(FormatString("%s\n(%i x %i)", (*aTexture)->myMetadata.myFilePath.c_str(), (*aTexture)->myMetadata.myWidth, (*aTexture)->myMetadata.myHeight));
			ImGui::EndTooltip();
		}

		PerformFlagsEnd();
	}
}

void KE_EDITOR::ImGuiHandler::DisplayMaterial(KE::Material* aMaterial, KittyFlags aFlags)
{
	if (PerformFlagsBegin(FormatString("Material: %s", aMaterial->myName.c_str()), aFlags | KittyFlagTypes::eSeparatorAbove))
	{
		for (int i = 0; i < 4; i++)
		{
			ImGui::PushID(i);

			DisplayTexture(&aMaterial->myTextures[i], ImVec2(64, 64));
			//if (ImGui::BeginDragDropTarget())
			//{
			//	if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Texture"))
			//	{
			//		KE::Texture* texture = *(KE::Texture**)payload->Data;
			//		aMaterial->myTextures[i] = texture;
			//	}
			//	ImGui::EndDragDropTarget();
			//}

			ImGui::PopID();
		}

		PerformFlagsEnd();
	}
}

bool KE_EDITOR::ImGuiHandler::DisplayViewport(const std::string& aViewportName, KE::RenderTarget* aRenderTarget, int anIndex, KE::Camera* aCamera, KittyFlags aFlags)
{
	
	if (PerformFlagsBegin(aViewportName.c_str(), aFlags, nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		

		PerformFlagsEnd();
		ImGui::PopStyleVar();
		return true;
	}
	else
	{
		windowData[anIndex] = { FLT_MIN, FLT_MIN, FLT_MIN,FLT_MIN, -1 };
		ImGui::PopStyleVar();
		return false;
	}
}




void KE_EDITOR::ImGuiHandler::DisplayVFXSequence(KittyFlags aFlags)
{
	if (vfxEditorActiveSequenceIndex == -1) { return; }
	KE::VFXManager& mgr = editor->myGraphics->GetVFXManager();
	KE::VFXSequence* aVFXSequence = mgr.GetVFXSequence(vfxEditorActiveSequenceIndex);
	if (!aVFXSequence)
	{
		KE_LOG("Closed VFX Sequence %i as it was null", vfxEditorActiveSequenceIndex);
		vfxEditorActiveSequenceIndex = -1;
		return;
	}

	bool open = true;
	if (PerformFlagsBegin("VFX Sequence", aFlags, &open))
	{
		static VFXSequenceInterface sqInterface;

		if (sqInterface.mySequence != aVFXSequence)
		{
			sqInterface.Link(aVFXSequence);
		}

		static int selected = 0;
		static int firstTime = 0;
		static int current = 0;
		static float currentFloat = 0.0f;

		static bool playing = false;
		static bool preview = true;
		static Transform previewTransform;
		//previewTransform.SetPosition({ 0.0f, 1.0f, 0.0f });

		if (ImGui::IsKeyPressed(ImGuiKey_Space, false))
		{
			playing = !playing;
		}

		currentFloat += ImGui::IsKeyPressed(ImGuiKey_LeftArrow) ? -1.0f : ImGui::IsKeyPressed(ImGuiKey_RightArrow) ? 1.0f : 0.0f;
		currentFloat += KE_GLOBAL::deltaTime * KE::VFX_SEQUENCE_FRAME_RATE * (float)playing;
		currentFloat = KE::Wrap(currentFloat, (float)sqInterface.GetFrameMin(), (float)sqInterface.GetFrameMax() - 1);
		current = (int)currentFloat;

		static bool expanded = true;

		ImGui::PushItemWidth(130);
		ImGui::InputInt("Duration (VFX Frames)", &sqInterface.mySequence->myDuration);
		ImGui::SameLine();
		float ds = static_cast<float>(sqInterface.mySequence->myDuration) / static_cast<float>(KE::VFX_SEQUENCE_FRAME_RATE);
		if (ImGui::InputFloat("Duration (Seconds)", &ds))
		{
			sqInterface.mySequence->myDuration = static_cast<int>(ds * static_cast<float>(KE::VFX_SEQUENCE_FRAME_RATE));
		}

		ImGui::SameLine();
		ImGui::Checkbox("Preview", &preview);

		static int previewLayer = static_cast<int>(KE::eRenderLayers::Main);


		if (ImGui::Button("Preview Settings")) { ImGui::OpenPopup("vfxPreviewAttr"); }
		if (ImGui::BeginPopup("vfxPreviewAttr"))
		{
			ImGui::Checkbox("Preview", &preview);
			ImGui::DragFloat3("Position", &previewTransform.GetPositionRef().x, 0.1f);
			ImGui::SliderInt(
				"Layer",
				&previewLayer,
				static_cast<int>(KE::eRenderLayers::Back),
				static_cast<int>(KE::eRenderLayers::Count) - 1,
				EnumToString(static_cast<KE::eRenderLayers>(previewLayer))
			);
			ImGui::EndPopup();
		}

		if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) { mgr.SaveVFXSequence(aVFXSequence); }
		ImGui::SameLine();
		if (ImGui::Button("Save")) { mgr.SaveVFXSequence(aVFXSequence); }
		ImGui::SameLine();
		if (ImGui::Button("Load"))
		{
			int index = vfxEditorActiveSequenceIndex;
			mgr.LoadVFXSequence(index, aVFXSequence->myName);
			aVFXSequence = mgr.GetVFXSequence(index);
			sqInterface.Link(aVFXSequence);
		}

		ImGui::PopItemWidth();

		//begin a table
		if (ImGui::BeginTable("VFX Sequence Table", 3, ImGuiTableFlags_Resizable))
		{
			ImGui::TableNextColumn();
			if (ImGui::BeginChild("Sequencer Child"))
			{
				const int currentCache = current;
				ImSequencer::Sequencer(&sqInterface, &current, 0 /*&expanded*/, &selected, &firstTime,
					ImSequencer::SEQUENCER_EDIT_STARTEND |
					ImSequencer::SEQUENCER_ADD |
					//ImSequencer::SEQUENCER_DEL | 
					ImSequencer::SEQUENCER_CHANGE_FRAME
				);
				if (currentCache != current) { currentFloat = (float)current; }
			}
			ImGui::EndChild();
			//

			ImGui::TableNextColumn();
			if (ImGui::BeginChild("Attribute Child") && sqInterface.mySequence->myTimestamps.size() > selected)
			{
				const KE::VFXTimeStamp& selectedStamp = sqInterface.mySequence->myTimestamps[selected];
				ImGui::Text("Selected: %d", selected);

				switch (selectedStamp.myType)
				{
				case KE::VFXType::VFXMeshInstance:
				{
					KE::ModelData* modelData = sqInterface.mySequence->myVFXMeshes[selectedStamp.myEffectIndex].GetModelData();
					DisplayModelData(modelData);
					break;
				}
				case KE::VFXType::ParticleEmitter:
				{
					KE::ParticleEmitter* emitter = &sqInterface.mySequence->myParticleEmitters[selectedStamp.myEffectIndex].myEmitter;
					DisplayParticleEmitter(emitter);
					break;
				}
				default:
					break;
				}
			}
			ImGui::EndChild();

			ImGui::TableNextColumn();

			DisplayPostProcessing(&mgr.myVFXPostProcessing);

			//
			ImGui::EndTable();
		}

		if (preview)
		{
			if (playing)
			{
				for (auto& em : sqInterface.mySequence->myParticleEmitters)
				{
					em.myEmitter.Update(previewTransform);
				}

			}
			mgr.RenderInspectedVFX(vfxEditorActiveSequenceIndex, current, &previewTransform, static_cast<KE::eRenderLayers>(previewLayer));
		}

		PerformFlagsEnd();
	}

	if (!open)
	{
		vfxEditorActiveSequenceIndex = -1;
	}
}

std::string KE_EDITOR::ImGuiHandler::BeautifyComponentName(const std::string& aUGLYName)
{
	if (beautifiedComponentNameMap.contains(aUGLYName)) { return beautifiedComponentNameMap[aUGLYName]; }

	size_t lastNamespaceMarker = aUGLYName.find_last_of("::");
	size_t componentMarker = aUGLYName.find("Component");
	if (componentMarker == std::string::npos) { componentMarker = aUGLYName.length(); }


	//get the substring between the last namespace marker and the component marker, so that "KE::TransformComponent" becomes "Transform"
	std::string beautifiedName = aUGLYName.substr(
		lastNamespaceMarker + 1,
		componentMarker - lastNamespaceMarker - 1
	);

	std::string tempName = "";

	const size_t length = beautifiedName.length();
	for (size_t i = 0; i < length; i++)
	{
		char current = beautifiedName[i];
		char next = i + 1 < length ? beautifiedName[i + 1] : '\0';

		tempName += current;
		if (IsCharLowerA(current) && IsCharUpperA(next))
		{
			tempName += " ";
		}
	}

	beautifiedName = tempName;

	beautifiedComponentNameMap[aUGLYName] = beautifiedName;
	return beautifiedName;
}
#endif

