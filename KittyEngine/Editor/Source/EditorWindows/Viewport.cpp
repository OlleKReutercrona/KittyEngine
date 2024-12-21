#include "stdafx.h"

#include "Project/Source/Boomerang/BoomerangComponent.h"
#include "ComponentSystem/Components/Collider/CapsuleColliderComponent.h"
#include "ComponentSystem/Components/Collider/SphereColliderComponent.h"
#include "Project/Source/Player/Player.h"
#ifndef KITTYENGINE_NO_EDITOR
#include "Viewport.h"

#include "ComponentSystem/Components/Collider/BoxColliderComponent.h"
#include "ComponentSystem/Components/Graphics/SkeletalModelComponent.h"
#include "Editor/Source/Editor.h"
#include "Editor/Source/ImGui/ImGuiHandler.h"
#include "SceneManagement/SceneManager.h"
#include "Utility/Global.h"

void KE_EDITOR::Viewport::Init()
{
	SetWindowFlags(GetWindowFlags() | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
}

void KE_EDITOR::Viewport::Update()
{

}

void KE_EDITOR::Viewport::Render()
{
	ImVec2 region = ImGui::GetContentRegionAvail();
	ImVec2 screenCursorPos = ImGui::GetCursorScreenPos();

	KE::Graphics* gfx = KE_GLOBAL::blackboard.Get<KE::Graphics>("graphics");

	gfx->AppendRenderTargetQueue(myRenderTargetIndex, myCameraIndex);

	KE::RenderTarget* renderTarget = gfx->GetRenderTarget(myRenderTargetIndex);
	KE::Camera* camera = gfx->GetCameraManager().GetCamera(myCameraIndex);

	//make region have the same aspect ratio as the render resolution
	region = GetAspectRegion(region, (float)gfx->GetRenderWidth() / (float)gfx->GetRenderHeight());

	ImVec2 vMin = ImGui::GetWindowContentRegionMin();
	ImVec2 vMax = ImVec2(vMin.x + region.x, vMin.y + region.y);
	ImVec2 wPos = ImGui::GetWindowPos();

	ImVec2 vMinWin = wPos + vMin;
	ImVec2 vMaxWin = wPos + vMax;

	ImGuiHandler::windowData[myRenderTargetIndex] = {
		vMinWin.x,
		vMinWin.y,
		vMaxWin.x,
		vMaxWin.y,
		camera->GetIndex()
	};


	ImDrawList* tDrawList = ImGui::GetWindowDrawList();
	//ImGui::Image(renderTarget->GetShaderResourceView(), region);
	tDrawList->AddImage(renderTarget->GetShaderResourceView(), vMinWin, vMaxWin);

	if (auto* console = KE_GLOBAL::editor->GetEditorWindow<EditorConsole>())
	{
		console->RenderPopups(screenCursorPos + ImVec2(4.0f, 2.0f), tDrawList); //temp!
	}


	constexpr float h = 24.0f, w = 182.0f, wo = 5.0f, ho = 0.0f;

	tDrawList->AddRectFilled(
		ImVec2(vMaxWin.x - w, vMinWin.y),
		ImVec2(vMaxWin.x, vMinWin.y + h),
		ImColor(0.0f, 0.0f, 0.0f, 0.78f),
		8.0f,
		ImDrawFlags_RoundCornersBottomLeft
	);

	bool drawDebug = renderTarget->ShouldRenderDebug();
	bool drawPostProcess = renderTarget->ShouldRenderPostProcessing();

	ImGui::SetCursorPos(ImVec2(vMax.x - w + wo, vMin.y + ho));

	if (ImGui::Checkbox("Debug", &drawDebug))
	{
		renderTarget->SetShouldRenderDebug(drawDebug);
	}
	
	ImGui::SameLine();

	if (ImGui::Checkbox("Post Process", &drawPostProcess))
	{
		renderTarget->SetShouldRenderPostProcessing(drawPostProcess);
	}

	if (drawDebug)
	{
		ImVec4 viewportBounds = {
			vMinWin.x, vMinWin.y, vMax.x - 8, vMax.y - 27
		};

		if (KE_GLOBAL::editor->myData.debugRenderData.renderNavmesh == DebugRenderLevel::eFull)
		{
			ImGuiHandler::DisplayNavmesh(
				&KE_GLOBAL::editor->mySceneManager->GetCurrentScene()->GetNavmesh(),
				camera,
				viewportBounds
			);
		}
		
		if (KE_GLOBAL::editor->myData.debugRenderData.renderSkeleton == DebugRenderLevel::eFull)
		{
			for (auto& gameObject : KE_GLOBAL::editor->mySceneManager->GetCurrentScene()->GetGameObjectManager().GetGameObjectsWithComponent<KE::SkeletalModelComponent>())
			{
				ImGuiHandler::DisplaySkeleton(gameObject);
			}
			
		}


		KE::GameObjectManager* gameObjectManager = &KE_GLOBAL::editor->mySceneManager->GetCurrentScene()->GetGameObjectManager();
		int& selectedGameObject = KE_GLOBAL::editor->myData.gameObjectData.lastSelectedGameObject;
		if (selectedGameObject != INT_MIN)
		{


			KE::GameObject* gameObject = gameObjectManager->GetGameObject(selectedGameObject);

			if (!gameObject)
			{
				selectedGameObject = INT_MIN;
			}
			else
			{
				//ugly!
				DirectX::XMMATRIX oldWorldSpaceTransform = gameObject->myWorldSpaceTransform.GetMatrix();
				ImGuiHandler::DisplayGizmo(&gameObject->myWorldSpaceTransform, camera, viewportBounds);
				DirectX::XMMATRIX newWorldSpaceTransform = gameObject->myWorldSpaceTransform.GetMatrix();

				//apply the changes between old and new to gameObject->myTransform
				DirectX::XMMATRIX delta = newWorldSpaceTransform * DirectX::XMMatrixInverse(nullptr, oldWorldSpaceTransform);
				gameObject->myTransform.SetMatrix(delta * gameObject->myTransform.GetMatrix());

				//physx collider updating stuff!
				{
					//components that have physx collision
					KE::BoxColliderComponent* box;
					KE::SphereColliderComponent* sphere;
					KE::CapsuleColliderComponent* capsule;
					P8::Player* player;
					P8::BoomerangComponent* boomerang;

					const auto& pos = gameObject->myTransform.GetPosition();

					if (gameObject->TryGetComponent(box))
					{
						
					}
					else if (gameObject->TryGetComponent(sphere))
					{
						
					}
					else if (gameObject->TryGetComponent(capsule))
					{
						
					}
					else if (gameObject->TryGetComponent(player))
					{
						
					}
					else if (gameObject->TryGetComponent(boomerang))
					{
						
					}
				}

			}
		}

		const bool anyHandleClicked = ImGuiHandler::DisplayCustomHandles(camera, viewportBounds); //returns true if any handle was pressed
		const bool arbitraryGizmoUsed = ImGuiHandler::DisplayArbitraryGizmo(&ImGuiHandler::arbitraryGizmo, camera, viewportBounds);

		if (!arbitraryGizmoUsed && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !anyHandleClicked)
		{
			ImGuiHandler::arbitraryGizmo.Clear();
		}
	}
}

void KE_EDITOR::Viewport::StyleBegin()
{
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
}

void KE_EDITOR::Viewport::StyleEnd()
{
	ImGui::PopStyleVar();
}

void KE_EDITOR::Viewport::Serialize(void* aWorkingData)
{
	auto& data = *(nlohmann::json*)aWorkingData;
	data["renderTargetIndex"] = myRenderTargetIndex;
	data["cameraIndex"] = myCameraIndex;
	data["name"] = myName;
}

void KE_EDITOR::Viewport::Deserialize(void* aWorkingData)
{
	auto& data = *(nlohmann::json*)aWorkingData;
	myRenderTargetIndex = data["renderTargetIndex"];
	myCameraIndex = data["cameraIndex"];
	strcpy_s(myName, data["name"].get<std::string>().c_str());
}

//void KE_EDITOR::Viewport::SetData(const int aRenderTargetIndex, const int aCameraIndex, const char* aName)
//{
//	myRenderTargetIndex = aRenderTargetIndex;
//	myCameraIndex = aCameraIndex;
//	myName = aName;
//}

#endif