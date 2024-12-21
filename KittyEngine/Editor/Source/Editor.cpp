#include "stdafx.h"

#include "EditorData.h"
#include "MeowFile.h"
#include "ComponentSystem/Components/Graphics/SkeletalModelComponent.h"
#include "EditorWindows/DataMaterialMaker.h"
#include "EditorWindows/GameObjectList.h"
#include "EditorWindows/GUIEditor.h"
#include "EditorWindows/NodeEditor.h"
#include "EditorWindows/SelectedObject.h"
#include "EditorWindows/Viewport.h"
#include "Input/Input.h"
#include "Math/Plane.h"
#ifndef KITTYENGINE_NO_EDITOR
#include "Editor.h"
#include <Engine/Source/Collision/Intersection.h>
#include <Engine/Source/Windows/Window.h>
#include <Engine/Source/Graphics/Graphics.h>
#include <Engine/Source/Utility/Timer.h>
#include <Engine/Source/Utility/Logging.h>
#include <Engine/Source/SceneManagement/SceneManager.h>
#include <Engine/Source/ComponentSystem/GameObjectManager.h>
#include <Engine/Source/ComponentSystem/Components/Graphics/ModelComponent.h>
#include <Engine/Source/Collision/Collider.h>
#include <Engine/Source/ComponentSystem/Components/Collider/BoxColliderComponent.h>
#include <External/Include/imgui/imgui.h>
#include <Engine/Source/AI/Pathfinding/Navmesh.h>
#include <Engine/Source/Utility/Global.h>
#include <Engine/Source/Utility/DebugTimeLogger.h>

#include "Editor/Source/Inspector/ComponentInspector.h"
#include "Editor/Source/EditorUtils.h"
#include "Editor/Source/ImGui/ImGuiHandler.h"
#include <Engine/Source/Files/LeMeowFile.h>
#include <Engine/Source/Files/KittyMesh.h>
#include <Engine/Source/Files/GUIFile.h>

#include <Editor/Source/EditorWindows/GameRuleEditor.h>
#include <Editor/Source/EditorWindows/AssetBrowser.h>
#include <Editor/Source/EditorWindows/DeferredView.h>
#include "Editor/Source/EditorWindows/EditSounds.h"

namespace KE_EDITOR
{
	Editor::Editor() : //Engine/Source/Graphics/Shaders
		myModelLoader(nullptr),
		myTextureLoader(nullptr),
		myShaderLoader(nullptr),
		mySceneManager(nullptr),
		myWindow(nullptr),
		myGraphics(nullptr),
		myTimer(nullptr)
	{
		KE_GLOBAL::editor = this;
	}

	Editor::~Editor()
	{
		for (auto& window : myWindows)
		{
			delete window;
		}
	}

	void Editor::Init(
		KE::Window* aWindow,
		KE::Timer* aTimer,

		KE::ShaderLoader* aShaderLoader,
		KE::TextureLoader* aTextureLoader,
		KE::ModelLoader* aModelLoader,
		KE::SceneManager* aSceneManager
	)
	{
		myWindow = aWindow;
		myGraphics = &aWindow->GetGraphics();
		myTimer = aTimer;

		myShaderLoader = aShaderLoader;
		myTextureLoader = aTextureLoader;
		myModelLoader = aModelLoader;
		mySceneManager = aSceneManager;

		ImGuiHandler::Init(myWindow->myHWnd, myGraphics->GetDevice().Get(), myGraphics->GetContext().Get(), this);

		//call windows function to enable file drop on the HWND
		DragAcceptFiles(myWindow->myHWnd, true);
		myWindow->myEditor = this;

		myGraphics->GetRenderTarget(0)->SetShouldRenderDebug(true);
		myGraphics->GetRenderTarget(0)->SetShouldRenderPostProcessing(true);

		myGraphics->GetRenderTarget(1)->SetShouldRenderDebug(false);
		myGraphics->GetRenderTarget(1)->SetShouldRenderPostProcessing(true);

		REGISTER_EDITOR_WINDOW(EditorAssetBrowser, false);
		REGISTER_EDITOR_WINDOW(EditorConsole, false);
		REGISTER_EDITOR_WINDOW(ModelViewer, false);
		REGISTER_EDITOR_WINDOW(GameObjectList, false);
		REGISTER_EDITOR_WINDOW(SelectedObject, false);
		REGISTER_EDITOR_WINDOW(Viewport, false);
		REGISTER_EDITOR_WINDOW(DeferredView, false);
		REGISTER_EDITOR_WINDOW(NodeEditor, false);
		REGISTER_EDITOR_WINDOW(GUIEditor, false);
		REGISTER_EDITOR_WINDOW(EditSounds, false);
		REGISTER_EDITOR_WINDOW(GameRuleEditor, false);
		REGISTER_EDITOR_WINDOW(MaterialEditor, false);

		REGISTER_EDITOR_WINDOW(LambdaWindow, true);

		//myWindowRegistry["EditorAssetBrowser"]();
		//myWindowRegistry["EditorConsole"]();
		//myWindowRegistry["GameObjectList"]();
		//myWindowRegistry["SelectedObject"]();
		//myWindowRegistry["Viewport"]();
		//myWindowRegistry["Viewport"]();
		//myWindowRegistry["NodeEditor"]();

		for (const auto& lambdaPreload : editorPreload.lambdaWindows)
		{
			myLambdaWindowRegistry[lambdaPreload.name] = lambdaPreload;
		}
		editorPreload.lambdaWindows.clear();

		LoadMeowFile();
	}

	ImGuiDockNode* Editor::ExtendDockTabBar()
	{
		ImGuiDockNode* out = nullptr;
		if (auto* node = ImGui::DockBuilderGetNode(ImGui::GetWindowDockID()))
		{
			if (ImGui::DockNodeBeginAmendTabBar(node))
			{
				if (ImGui::TabItemButton("+", ImGuiTabItemFlags_Trailing))
				{
					out = node;
				}
				ImGui::DockNodeEndAmendTabBar();
			}
		}

		return out;
	}


	void Editor::Update()
	{
		if (ImGui::IsKeyReleased(ImGuiKey_F6))
		{
			myGraphics->IncrementDeferredPass();
		}

		if (ImGui::IsKeyReleased(ImGuiKey_F7))
		{
			myGraphics->ToggleDrawSSAOAsResult();
		}

		if (ImGui::IsKeyReleased(ImGuiKey_F8))
		{
			mySceneManager->GetCurrentScene()->ToggleDrawDebugFlag(KE::SceneDrawFlags::eDrawPhysX);
		}

		//if (myFileWatcher.Update())
		//{
		//	//KE_LOG("file changed: %s", myFileWatcher.GetLastChangedFile().c_str());
		//
		//	std::string file = myFileWatcher.GetLastChangedFile();
		//
		//	if (file.find(".hlsl") != std::string::npos)
		//	{
		//		std::string sub = file.substr(file.find_last_of("\\") + 1);
		//		myShaderLoader->RecompileShader(sub);
		//	}
		//	if (file.find(".png") != std::string::npos)
		//	{
		//		std::string sub = file.substr(file.find_last_of("\\") + 1);
		//		myTextureLoader->ReloadTexture(myTextureLoader->GetTextureFromPath(file));
		//	}
		//}

		if (ImGui::IsKeyPressed(ImGuiKey_F1))
		{
			myData.activeData.editorState = myData.activeData.editorState == 1 ? 0 : 1; //set fullscreen editor
		}
		if (ImGui::IsKeyPressed(ImGuiKey_F2))
		{
			myData.activeData.editorState = myData.activeData.editorState == 2 ? 0 : 2; //set fullscreen editor

			myGraphics->GetRenderTarget(0)->SetShouldRenderDebug(myGraphics->GetRenderTarget(1)->ShouldRenderDebug());
		}

		/*if (ImGui::IsKeyPressed(ImGuiKey_F3, false))
		{
			if (myGraphics->myInstancedRenderPackages.size() > 0)
			{
				myGraphics->myInstancedRenderPackages.clear();
				std::cout << "cleared instanced render packages" << std::endl;
			}
			else
			{
				myGraphics->CreateRenderingQueue(true);
				std::cout << "created instanced render packages" << std::endl;
			}
		}*/

		if (myData.activeData.editorState > 0)
		{
			myGraphics->AppendRenderTargetQueue(-1, myData.activeData.editorState == 1
				                                        ? KE_DEBUG_CAMERA_INDEX
				                                        : KE_MAIN_CAMERA_INDEX);
			if (myData.activeData.editorState == 1)
			{
				//ImGuiHandler::DisplayVFXSequence(nullptr, KittyFlagTypes::eBeginWindow);
				ControlCamera(myGraphics->GetCameraManager().GetCamera(KE_DEBUG_CAMERA_INDEX));

				myGraphics->GetCameraManager().SetHighlightedCamera(KE_DEBUG_CAMERA_INDEX);
			}
			else
			{
				myGraphics->GetCameraManager().SetHighlightedCamera(KE_MAIN_CAMERA_INDEX);
			}

			return;
		}


		ImGuiHandler::DisplayMainMenuBar();
		ImGuiHandler::BeginFullscreenWindow();

		//ImGui::ShowDemoWindow();

		static ImGuiDockNode* pressedNode = nullptr;
		bool openNodePopup = false;

		for (int i = 0; i < myWindows.size(); i++)
		{
			auto window = myWindows[i];
			KE::DebugTimeLogger::BeginLogVar(window->GetWindowName());
			if (window->Begin())
			{
				if (auto* node = ExtendDockTabBar())
				{
					pressedNode = node;
					openNodePopup = true;
				}
				window->Update();
				window->Render();
			}
			window->End();

			KE::DebugTimeLogger::EndLogVar(window->GetWindowName());


			if (window->GetStatus() == WindowStatus::Closed)
			{
				delete myWindows[i];
				myWindows.erase(myWindows.begin() + i);
			}
		}

		if (pressedNode)
		{
			if (openNodePopup) { ImGui::OpenPopup("createWindowPopup"); }

			if (ImGui::BeginPopup("createWindowPopup"))
			{
				for (const auto& [windowName, entry] : myWindowRegistry)
				{
					if (entry.hidden) { continue; }
					if (ImGui::MenuItem(windowName.c_str()))
					{
						entry.myCreationFunc({})->Dock(pressedNode->ID);
					}
				}
				ImGui::SeparatorText("LambdaWindows");
				for (const auto& lambdaWindowData : myLambdaWindowRegistry)
				{
					if (ImGui::MenuItem(lambdaWindowData.first.c_str()))
					{
						myWindows.push_back(new LambdaWindow(lambdaWindowData.second));
						myWindows.back()->Dock(pressedNode->ID);
					}
				}

				ImGui::EndPopup();
			}
			else
			{
				pressedNode = nullptr;
			}
		}

		for (int i = 0; i < myData.dragDropData.files.size(); i++)
		{
			HandleDragDropQueue();
		}

		myData.performance.Register(KE_GLOBAL::deltaTime);

		KE::Scene* scene = ((KE::Scene*)mySceneManager->GetCurrentScene());
		KE::GameObjectManager& gom = scene->gameObjectManager;
		const std::vector<KE::GameObject*>& gameObjects = gom.GetGameObjects();
		const std::vector<KE::GameObject*>& newGameObjects = gom.GetNewGameObjects();


		int camIndex = -1;
		Vector2i vmp = GetViewportMousePos(&camIndex);
		if (camIndex > -1)
		{
			myGraphics->GetCameraManager().SetHighlightedCamera(camIndex);
		}

		Vector2f vmpf = Vector2f((float)vmp.x, (float)vmp.y);

		//

		if (camIndex >= 0)
		{
			myGraphics->GetCameraManager().SetHighlightedCamera(camIndex);

			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsKeyDown(ImGuiKey_B))
			{
				KE::GameObject* hoveredObject	 = GetHoveredGameObject(gameObjects, vmpf);
				hoveredObject = hoveredObject ? hoveredObject : GetHoveredGameObject(newGameObjects, vmpf); //if no regular object exists, check new objects

				if (hoveredObject)
				{
					myData.gameObjectData.selectedGameObjects.clear();
					myData.gameObjectData.selectedGameObjects[hoveredObject->myID] = true;
					myData.gameObjectData.lastSelectedGameObject = hoveredObject->myID;
				}
				else
				{
					myData.gameObjectData.selectedGameObjects.clear();
					myData.gameObjectData.lastSelectedGameObject = INT_MIN;
				}
			}
		}

		if (camIndex == KE_DEBUG_CAMERA_INDEX)
		{
			ControlCamera(myGraphics->GetCameraManager().GetCamera(KE_DEBUG_CAMERA_INDEX));
			if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
			{
				//use windows api to clip the cursor to the window
				RECT rect;
				ImGuiWindowData wd = ImGuiHandler::windowData[0];

				rect.left = (LONG)wd.aTopLeftX;
				rect.top = (LONG)wd.aTopLeftY;
				rect.right = (LONG)wd.aBottomRightX;
				rect.bottom = (LONG)wd.aBottomRightY;

				ClipCursor(&rect);
			}
		}

		//static bool lockToGame = false;
		//if (camIndex == KE_MAIN_CAMERA_INDEX)
		//{
		//	if (ImGui::IsKeyReleased(ImGuiKey_L))
		//	{
		//		lockToGame = !lockToGame;
		//		if(lockToGame)
		//		{
		//			//use windows api to clip the cursor to the window
		//			RECT rect;
		//			ImGuiWindowData wd = ImGuiHandler::windowData[1];

		//			rect.left = (LONG)wd.aTopLeftX;
		//			rect.top = (LONG)wd.aTopLeftY;
		//			rect.right = (LONG)wd.aBottomRightX;
		//			rect.bottom = (LONG)wd.aBottomRightY;

		//			rect.left = static_cast<LONG>(rect.left + (rect.right - rect.left) / 2.0f);
		//			rect.right = rect.left;
		//			rect.top = static_cast<LONG>(rect.top + (rect.bottom - rect.top) / 2.0f);
		//			rect.bottom = rect.top;

		//			KE::InputWrapper& input = myWindow->GetInputWrapper();
		//			myWindow->DisableCursor();
		//			ClipCursor(&rect);
		//			input.EnableRaw();

		//		}
		//		else
		//		{
		//			KE::InputWrapper& input = myWindow->GetInputWrapper();
		//			myWindow->EnableCursor();
		//			ClipCursor(nullptr);
		//			input.DisableRaw();

		//		}
		//	}

		//}

		//ImGuiHandler::DisplayObjectList(gameObjects, KittyFlagTypes::eBeginWindow);
		ImGuiHandler::DisplayVFXSequence(KittyFlagTypes::eBeginWindow);

		if (myData.gameObjectData.lastSelectedGameObject > INT_MIN)
		{
			auto* selected = gom.GetGameObject(myData.gameObjectData.lastSelectedGameObject);
			if (!selected)
			{
				myData.gameObjectData.lastSelectedGameObject = INT_MIN;
			}
			else
			{
				HighlightObject(selected);
			}
		}

		//graphics segment!
		if (myData.debugRenderData.renderNavmesh >= DebugRenderLevel::eBasic)
		{
			scene->myNavmesh.DebugRender(&myWindow->GetGraphics());
			scene->myPathfinder.DebugRender(&myWindow->GetGraphics());
		}
	}

	void Editor::ExtractLogChildren(const KE::LoggedVar& log, const float aFrameTime)
	{
		std::string text(log.myName);
		std::string ms(": ");
		float percent = 0.0f;
		if (aFrameTime > 0.0f)
		{
			percent = (log.myTime / aFrameTime) * 100.0f;
		}
		// Cursed string manipulation
		std::string perc = std::to_string(percent);
		perc.erase(perc.begin() + perc.find('.') + 2, perc.end());

		ms += std::to_string(log.myTime) + "ms " + "(" + perc + "%%)";

		// This is pretty ugly with the else that has the same code as in the if
		// But if text is concantinated with ms ImGui's label system breaks as ms is a 
		// changing variable (it will cause the node to collapse every frame)

		//text += ": " + std::to_string(log.myTime) + "ms" + "##" + log.myName.c_str();

		if (log.myChildren.size() > 0)
		{
			bool open = ImGui::TreeNode(text.c_str());
			ImGui::SameLine();
			ImGui::Text(ms.c_str());
			if (open)
			{
				for (auto& child : log.myChildren)
				{
					ExtractLogChildren(child.second, aFrameTime);
				}
				ImGui::TreePop();
			}
		}
		else
		{
			text.insert(0, "   ");
			ImGui::Text(text.c_str());
			ImGui::SameLine();
			ImGui::Text(ms.c_str());
		}
	}

	void Editor::RegisterLambdaWindow(const std::string& aName, LambdaWindowFunc aLambda)
	{
		myLambdaWindowRegistry[aName] = { aName, aLambda };
	}

	void Editor::BeginFrame()
	{
		if (myData.FullscreenToggleQueued)
		{
			myWindow->SetFullscreen(!myWindow->GetFullscreen());
			myData.FullscreenToggleQueued = false;
		}

		ImGuiHandler::windowData.clear();

		ImGuiHandler::BeginFrame();
	}

	void Editor::EndFrame()
	{
		ImGuiHandler::EndFrame();
	}

	void Editor::Shutdown()
	{
		ImGuiHandler::Shutdown();
	}

	void Editor::ReceiveDragDrop(const std::string& aPath)
	{
		myData.dragDropData.files.push_back(aPath);
		KE_LOG(
			"Received File: %s",
			myData.dragDropData.files.back().c_str()
		);
	}

	bool Editor::IsEnabled() const
	{
		return myData.activeData.editorState == 0;
	}

	Vector2i Editor::GetViewportMousePos(int* anOutCamIndex)
	{
		POINT mousePos;
		GetCursorPos(&mousePos);
		ScreenToClient(myWindow->myHWnd, &mousePos);

		float aX = (float)mousePos.x;
		float aY = (float)mousePos.y;

		for (auto& wData : ImGuiHandler::windowData)
		{
			POINT topLeft = {(LONG)wData.second.aTopLeftX, (LONG)wData.second.aTopLeftY};
			POINT bottomRight = {(LONG)wData.second.aBottomRightX, (LONG)wData.second.aBottomRightY};

			ScreenToClient(myWindow->myHWnd, &topLeft);
			ScreenToClient(myWindow->myHWnd, &bottomRight);

			if (aX >= topLeft.x && aX <= bottomRight.x &&
				aY >= topLeft.y && aY <= bottomRight.y)
			{
				//translate mouse coordinates to 0,0 at top left of window, and 1,1 at bottom right of window
				aX -= topLeft.x;
				aY -= topLeft.y;


				aX /= (bottomRight.x - topLeft.x);
				aY /= (bottomRight.y - topLeft.y);

				aX *= (float)myGraphics->GetRenderWidth();
				aY *= (float)myGraphics->GetRenderHeight();

				if (anOutCamIndex)
				{
					*anOutCamIndex = wData.second.aCameraIndex;
				}

				return Vector2i((int)aX, (int)aY);
			}
		}

		if (anOutCamIndex)
		{
			*anOutCamIndex = -1;
		}
		return Vector2i(-1, -1);
	}

	KE::Box Editor::GetModelBoundsExtremes(const std::vector<const ModelBounds*>& aModelBoundsList, const Transform& aTransform)
	{
		Vector3f totalMin = aModelBoundsList[0]->min;
		Vector3f totalMax = aModelBoundsList[0]->max;

		for (const auto* boundsPtr : aModelBoundsList)
		{
			for (unsigned int i = 0; i < 3; ++i)
			{
				totalMin[i] = (std::min)(totalMin[i], boundsPtr->min[i]);
				totalMax[i] = (std::max)(totalMax[i], boundsPtr->max[i]);
			}
		}

		const Matrix3x3f rot = aTransform.GetCUMatrix(); //truncate to a 3x3 matrix since Box handles positioning, we only want rot and scale!

		KE::Box outBox(totalMin, totalMax, aTransform.GetPosition());

		return outBox;
	}

	KE::GameObject* Editor::GetHoveredGameObject(
		const std::vector<KE::GameObject*>& aGameObjectList,
		const Vector2f& anEditorMousePos
	)
	{
		KE::Camera* cam = myGraphics->GetCameraManager().GetHighlightedCamera();

		Rayf castRay = cam->GetRay(anEditorMousePos);

		struct SelectionHitData
		{
			float distance;
			KE::GameObject* object;
			KE::MeshList* meshList;
			KE::SkeletalMeshList* skeletalMeshList;
		};

		std::vector<SelectionHitData> hitData;

		for (int m = 0; m < aGameObjectList.size(); m++)
		{
			if (!aGameObjectList[m]->IsActive()) { continue; }

			std::vector<const ModelBounds*> objectBounds;

			KE::ModelComponent* objModelComponent;
			KE::MeshList* meshList = nullptr;
			if (aGameObjectList[m]->TryGetComponent<KE::ModelComponent>(objModelComponent))
			{
				const KE::ModelData* modelData = objModelComponent->GetModelData();
				meshList = modelData->myMeshList;
				for (const auto& subMesh : meshList->myMeshes)
				{
					objectBounds.push_back(&subMesh.myBounds);
				}
			}
			KE::SkeletalModelComponent* objSkeletalModelComponent;
			KE::SkeletalMeshList* skeletalMeshList = nullptr;
			if (aGameObjectList[m]->TryGetComponent<KE::SkeletalModelComponent>(objSkeletalModelComponent))
			{
				const KE::SkeletalModelData* modelData = objSkeletalModelComponent->GetModelData();
				for (const auto& subMesh : modelData->myMeshList->myMeshes)
				{
					objectBounds.push_back(&subMesh.myBounds);
				}
			}

			if (objectBounds.size() == 0) { continue; }

			KE::Box boundsExtreme = GetModelBoundsExtremes(objectBounds, aGameObjectList[m]->myWorldSpaceTransform);

			Vector3f hitPoint;
			if (EditorIntersectBox(boundsExtreme, castRay, hitPoint))
			{
				float distance = (
					hitPoint - cam->transform.GetPosition()
				).Length();

				hitData.push_back({ distance, aGameObjectList[m], meshList, skeletalMeshList});
			}
		}


		if (hitData.size() > 0)
		{
			//hitData.erase(
			//	std::remove_if(
			//		hitData.begin(),
			//		hitData.end(),
			//		[castRay](const SelectionHitData& hit)
			//		{
			//			if (hit.meshList)		  { return !RayIntersectsModel(hit.meshList, castRay, hit.object->myWorldSpaceTransform); }
			//			if (hit.skeletalMeshList) { return !RayIntersectsModel(hit.skeletalMeshList, castRay, hit.object->myWorldSpaceTransform); }
			//			return true; //default to removing :)
			//		}
			//	), hitData.end()
			//);

			//if (hitData.empty()) { return nullptr; } //just in case we delete everything

			////sort
			//std::sort(hitData.begin(), hitData.end(), [](const SelectionHitData& a, const SelectionHitData& b)
			//{
			//		return a.distance < b.distance;
			//});

			float workingDistance = FLT_MAX;
			const SelectionHitData* nearestHit = nullptr;

			for (const auto& hit : hitData)
			{
				if (hit.distance > workingDistance) { continue; }
				if (hit.meshList && RayIntersectsModel(hit.meshList, castRay, hit.object->myWorldSpaceTransform) ||
					hit.skeletalMeshList && RayIntersectsModel(hit.skeletalMeshList, castRay, hit.object->myWorldSpaceTransform))
				{
					workingDistance = hit.distance;
					nearestHit = &hit;
				}
			}

			if (!nearestHit) { return nullptr; }

			KE::GameObject* nearestObject = nearestHit->object;
			return nearestObject;
		}
		return nullptr;
	}

	bool Editor::PointInsideTriangle(Vector3f p0, Vector3f p1, Vector3f p2, Vector3f point)
	{
		p0 -= point;
		p1 -= point;
		p2 -= point;

		Vector3f u = p1.Cross(p2);
		Vector3f v = p2.Cross(p0);
		Vector3f w = p0.Cross(p1);

		if (u.Dot(v) < 0.0f) { return false; }
		if (u.Dot(w) < 0.0f) { return false; }

		return true;
	}

	bool Editor::RayIntersectsModel(KE::MeshList* aMeshList, const Rayf& aRay, const Transform& aTransform)
	{
		for (const auto& mesh : aMeshList->myMeshes)
		{
			for (int i = 0; i < mesh.myIndices.size(); i += 3)
			{
				unsigned int index0 = mesh.myIndices[i];
				unsigned int index1 = mesh.myIndices[i+1];
				unsigned int index2 = mesh.myIndices[i+2];

				Vector4f p0 = { mesh.myVertices[index0].x, mesh.myVertices[index0].y, mesh.myVertices[index0].z, 1.0f };
				Vector4f p1 = { mesh.myVertices[index1].x, mesh.myVertices[index1].y, mesh.myVertices[index1].z, 1.0f };
				Vector4f p2 = { mesh.myVertices[index2].x, mesh.myVertices[index2].y, mesh.myVertices[index2].z, 1.0f };

				p0 = p0 * aTransform.GetCUMatrix();
				p1 = p1 * aTransform.GetCUMatrix();
				p2 = p2 * aTransform.GetCUMatrix();

				Plane p(p0.xyz(), p1.xyz(), p2.xyz());
				Vector3f hitPoint;
				if (!IntersectionPlaneRay(p, aRay, hitPoint)) { continue; }
				if (!PointInsideTriangle(p0.xyz(), p1.xyz(), p2.xyz(), hitPoint)) { continue; }
				return true;
			}
		}
		return false;
	}

	bool Editor::RayIntersectsModel(KE::SkeletalMeshList* aMeshList, const Rayf& aRay, const Transform& aTransform)
	{
		for (const auto& mesh : aMeshList->myMeshes)
		{
			for (int i = 0; i < mesh.myIndices.size(); i += 3)
			{
				unsigned int index0 = mesh.myIndices[i];
				unsigned int index1 = mesh.myIndices[i + 1];
				unsigned int index2 = mesh.myIndices[i + 2];

				Vector4f p0 = { mesh.myVertices[index0].x, mesh.myVertices[index0].y, mesh.myVertices[index0].z, 1.0f };
				Vector4f p1 = { mesh.myVertices[index1].x, mesh.myVertices[index1].y, mesh.myVertices[index1].z, 1.0f };
				Vector4f p2 = { mesh.myVertices[index2].x, mesh.myVertices[index2].y, mesh.myVertices[index2].z, 1.0f };

				p0 = p0 * aTransform.GetCUMatrix();
				p1 = p1 * aTransform.GetCUMatrix();
				p2 = p2 * aTransform.GetCUMatrix();

				Plane p(p0.xyz(), p1.xyz(), p2.xyz());
				Vector3f hitPoint;
				if (!IntersectionPlaneRay(p, aRay, hitPoint)) { continue; }
				if (!PointInsideTriangle(p0.xyz(), p1.xyz(), p2.xyz(), hitPoint)) { continue; }
				return true;
			}
		}
		return false;
	}

	bool Editor::HighlightObject(KE::GameObject* aGameObject)
	{
		if (!aGameObject) { return false; }
		KE::ModelComponent* objModelComponent;
		if (aGameObject->TryGetComponent<KE::ModelComponent>(objModelComponent))
		{
			objModelComponent->GetModelData()->myRenderOutline = true;
		}

		return false;
	}

	void Editor::ControlCamera(KE::Camera* aCamera)
	{
		KE::InputWrapper& input = myWindow->GetInputWrapper();
		if (input.IsRMBPressed())
		{
			myWindow->DisableCursor();
			input.EnableRaw();
		}

		if (input.IsRMBReleased())
		{
			myWindow->EnableCursor();
			input.DisableRaw();
		}

		if (!myWindow->IsCursorEnabled())
		{
			float cameraSpeed = 2.0f * KE_GLOBAL::deltaTime;
			float cameraShiftMult = 7.0f;
			float cameraCTRLMult = 0.5f;
			static float cameraScrollSpeedMultiplier = 1.0f;
			short scrollDelta = input.GetScrollDelta();

			if (scrollDelta)
			{
				cameraScrollSpeedMultiplier += scrollDelta > 0 ? 0.1f : -0.1f;
				cameraScrollSpeedMultiplier = std::clamp(cameraScrollSpeedMultiplier, 0.1f, 5.0f);
			}

			if (input.IsKeyHeld(VK_SHIFT))
			{
				cameraSpeed *= cameraShiftMult * cameraScrollSpeedMultiplier;
			}
			else if (input.IsKeyHeld(VK_CONTROL))
			{
				cameraSpeed *= cameraCTRLMult * cameraScrollSpeedMultiplier;
			}


			if (input.IsKeyHeld(KE::KEY_MOV_FW))
			{
				aCamera->transform.TranslateLocal(Vector3f(0.0f, 0.0f, cameraSpeed));
			}

			if (input.IsKeyHeld(KE::KEY_MOV_BW))
			{
				aCamera->transform.TranslateLocal(Vector3f(0.0f, 0.0f, -cameraSpeed));
			}

			if (input.IsKeyHeld(KE::KEY_MOV_LF))
			{
				aCamera->transform.TranslateLocal(Vector3f(-cameraSpeed, 0.0f, 0.0f));
			}

			if (input.IsKeyHeld(KE::KEY_MOV_RT))
			{
				aCamera->transform.TranslateLocal(Vector3f(cameraSpeed, 0.0f, 0.0f));
			}

			if (input.IsKeyHeld(KE::KEY_MOV_UP))
			{
				aCamera->transform.TranslateLocal(Vector3f(0.0f, cameraSpeed, 0.0f));
			}

			if (input.IsKeyHeld(KE::KEY_MOV_DN))
			{
				aCamera->transform.TranslateLocal(Vector3f(0.0f, -cameraSpeed, 0.0f));
			}
			while (const auto delta = input.ReadRawDelta())
			{
				aCamera->Rotate2D((float)delta->myX, (float)delta->myY);
			}
		}
	}

	void Editor::HandleDragDropQueue()
	{
		KE::Scene* scene = ((KE::Scene*)mySceneManager->GetCurrentScene());
		KE::GameObjectManager& gom = scene->gameObjectManager;

		static int objIDCounter = -100;

		std::string& path = myData.dragDropData.files.back();

		if (path.find(".fbx") != std::string::npos || path.find(".obj") != std::string::npos)
		{
			KE::ModelData* data = myGraphics->CreateModelData(path);

			gom.CreateGameObject(objIDCounter, &path);
			KE::GameObject* ob = gom.GetGameObject(objIDCounter);
			data->myTransform = &ob->myTransform.GetMatrix();
			ob->AddComponent<KE::ModelComponent>();
			ob->GetComponent<KE::ModelComponent>().SetModelData(*data);
			objIDCounter--;
			//myGraphics->CreateRenderingQueue(true);
			ImGuiHandler::RegenerateGameObjectCache();
		}

		if (path.find(".dds") != std::string::npos)
		{
			//remove ".dds" from path
			path = path.substr(0, path.find("_c.dds"));

			//remove everything before the last slash
			path = path.substr(path.find_last_of("/") + 1);
			path = path.substr(path.find_last_of("\\") + 1);

			KE::Material* mat = myTextureLoader->GetMaterialFromPath(path);
		}


		myData.dragDropData.files.pop_back();
	}

	void Editor::SaveMeowFile()
	{
		MeowFile aMeowFile;

		//export window rect:
		RECT rect;
		GetWindowRect(myWindow->myHWnd, &rect);
		//adjust rect so we export the client area

		Vector2i windowSize = myWindow->GetWindowSize();

		float x = (float)rect.left;
		float y = (float)rect.top;

		aMeowFile.windowSettings.windowX = x;
		aMeowFile.windowSettings.windowY = y;
		aMeowFile.windowSettings.windowWidth = (float)windowSize.x;
		aMeowFile.windowSettings.windowHeight = (float)windowSize.y;

		aMeowFile.windowSettings.isWindowMaximized = false;

		aMeowFile.windowSettings.isWindowFullscreen = myWindow->GetFullscreen();

		//export console rect:
		HWND console = GetConsoleWindow();
		GetWindowRect(console, &rect);

		aMeowFile.windowSettings.consoleX = (float)rect.left;
		aMeowFile.windowSettings.consoleY = (float)rect.top;
		aMeowFile.windowSettings.consoleWidth = (float)(rect.right - rect.left);
		aMeowFile.windowSettings.consoleHeight = (float)(rect.bottom - rect.top);

		aMeowFile.windowSettings.isConsoleMaximized = false;

		aMeowFile.debugRenderData = myData.debugRenderData;

		//

		aMeowFile.Save("settings.meow");
	}

	void Editor::SavePostProcessSettings()
	{
		KE::LevelSettingsMeowFile meowFile;

		meowFile.myPPData = myGraphics->GetPostProcessing()->myAttributes;

		std::string fileName(mySceneManager->GetCurrentScene()->sceneName);
		fileName.erase(fileName.begin() + fileName.find_last_of("."), fileName.end());
		fileName += ".LeMeow";

		meowFile.Save(fileName);
	}

	void Editor::LoadPostProcessSettings()
	{
		KE::LevelSettingsMeowFile meowFile;

		std::string fileName(mySceneManager->GetCurrentScene()->sceneName);
		fileName.erase(fileName.begin() + fileName.find_last_of("."), fileName.end());
		fileName += ".LeMeow";

		meowFile.Load(fileName);

		myGraphics->GetPostProcessing()->myAttributes = meowFile.myPPData;
	}

	void Editor::SaveKittyMeshFile()
	{
		KE::KittyMesh file;

		std::string fileName(mySceneManager->GetCurrentScene()->sceneName);
		fileName.erase(fileName.begin() + fileName.find_last_of("."), fileName.end());
		fileName += ".KittyMesh";

		mySceneManager->GetCurrentScene()->myNavmesh.SaveKittyMesh(file);

		file.Save(fileName);
	}

	void Editor::LoadKittyMeshFile()
	{
		KE::KittyMesh file;

		std::string fileName(mySceneManager->GetCurrentScene()->sceneName);
		fileName.erase(fileName.begin() + fileName.find_last_of("."), fileName.end());
		fileName += ".KittyMesh";

		file.Load(fileName);

		mySceneManager->GetCurrentScene()->myNavmesh.LoadKittyMesh(file);
	}

	void Editor::ClearNavmesh()
	{
		mySceneManager->GetCurrentScene()->myNavmesh.ClearNavmesh();
	}

	void Editor::SaveGUIFile()
	{
	}

	void Editor::LoadMeowFile()
	{
		MeowFile aMeowFile;

		//if settings file exists, load it
		if (!std::filesystem::exists("settings.meow"))
		{
			return;
		}
		aMeowFile.Load("settings.meow");

		myData.debugRenderData = aMeowFile.debugRenderData;

		myWindow->SetWindowDims(
			(int)aMeowFile.windowSettings.windowX,
			(int)aMeowFile.windowSettings.windowY,
			(int)aMeowFile.windowSettings.windowWidth,
			(int)aMeowFile.windowSettings.windowHeight,
			false
		);

		if (aMeowFile.windowSettings.isWindowMaximized) { }

		if (aMeowFile.windowSettings.isWindowFullscreen)
		{
			myWindow->SetFullscreen(true);
		}

		//apply console rect:
		RECT rect;
		HWND console = GetConsoleWindow();
		rect.left = (LONG)aMeowFile.windowSettings.consoleX;
		rect.top = (LONG)aMeowFile.windowSettings.consoleY;
		rect.right = (LONG)(aMeowFile.windowSettings.consoleX + aMeowFile.windowSettings.consoleWidth);
		rect.bottom = (LONG)(aMeowFile.windowSettings.consoleY + aMeowFile.windowSettings.consoleHeight);

		MoveWindow(console, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, true);
	}

	void Editor::ResetMeowFile()
	{
		//just delete the file
		std::filesystem::remove("settings.meow");
		std::filesystem::copy("defaultSettings.meow", "settings.meow");
		LoadMeowFile();
	}
}
#endif
