#include "stdafx.h"

#ifndef KITTYENGINE_NO_EDITOR

#include "ModelViewer.h"
#include "Editor/Source/Editor.h"

#include <External/Include/imgui/imgui.h>

bool KE_EDITOR::ModelViewer::HoveringOverRender() const
{
	auto   mousePos = ImGui::GetMousePos();
	return mousePos.x > lastRenderRectMin.x && 
		   mousePos.x < lastRenderRectMax.x && 
		   mousePos.y > lastRenderRectMin.y && 
		   mousePos.y < lastRenderRectMax.y;
}

void KE_EDITOR::ModelViewer::Init()
{
	myModelData.myMeshList = &KE_GLOBAL::editor->myModelLoader->Load("Data/Assets/testModel.fbx");
	auto& rr = myModelData.myRenderResources.emplace_back();
	rr.myMaterial = KE_GLOBAL::editor->myTextureLoader->GetDefaultMaterial();
	rr.myPixelShader = KE_GLOBAL::editor->myShaderLoader->GetPixelShader(SHADER_LOAD_PATH "Model_Deferred_PS.cso");
	rr.myVertexShader = KE_GLOBAL::editor->myShaderLoader->GetVertexShader(SHADER_LOAD_PATH "Model_Deferred_VS.cso");
	myModelData.myTransform = &myModelTransform.GetMatrix();

	myModelViewerGround.myMeshList = &KE_GLOBAL::editor->myModelLoader->Load("Data/EditorAssets/modelViewerGroundPlane.fbx");
	auto& grr = myModelViewerGround.myRenderResources.emplace_back();
	grr.myMaterial = KE_GLOBAL::editor->myTextureLoader->GetCustomMaterial("Data/EditorAssets/Prototype/Light/texture_08.png");
	grr.myPixelShader = KE_GLOBAL::editor->myShaderLoader->GetPixelShader(SHADER_LOAD_PATH "ModelViewer_Ground_PS.cso");
	grr.myVertexShader = KE_GLOBAL::editor->myShaderLoader->GetVertexShader(SHADER_LOAD_PATH "Model_Deferred_VS.cso");
	myGroundTransform.SetScale({ 100.0f,100.0f,100.0f });
	myModelViewerGround.myTransform = &myGroundTransform.GetMatrix();

	auto* graphics = KE_GLOBAL::editor->myGraphics;
	myBuffer = &graphics->GetSecondaryGBuffer();
	myRenderTarget = graphics->GetRenderTarget(8);
	myLightManager.Init(graphics->GetDevice().Get(), &graphics->GetShaderLoader(), {graphics->GetRenderWidth(), graphics->GetRenderHeight()});
	myLightManager.AssignCubemap(KE_GLOBAL::editor->myTextureLoader->GetCubemapFromPath("Data/EngineAssets/CubeMap_Skansen.dds"));
	auto* dirLight = (KE::DirectionalLightData*)myLightManager.CreateLightData(KE::eLightType::Directional);

	dirLight->myDirection = Vector3f(1.0f,1.0f,0.0f).GetNormalized();
	dirLight->myColour = { 1.0f,1.0f,1.0f };
	dirLight->myDirectionalLightIntensity = 1.0f;
	dirLight->myAmbientLightIntensity = 1.0f;

	myFullscreenAsset.Init(graphics);
}

void KE_EDITOR::ModelViewer::Update()
{
	auto* graphics = KE_GLOBAL::editor->myGraphics;

	float color[4] = { 0.0f,0.0f,0.0f,1.0f };


	myRenderTarget->Clear(color);
	myBuffer->ClearTextures(graphics->GetContext().Get());
	myLightManager.myDepthBuffer.Clear(graphics->GetContext().Get());

	auto* cam = graphics->GetCameraManager().GetCamera(KE_DEBUG_CAMERA_INDEX - 1);

	static float xAngle = 0.0f;
	static float yAngle = 0.0f;
	static Vector3f pos = { 0.0f,5.0f,0.0f };
	static float distance = -10.0f;

	ImGui::DragFloat("AngleX", &xAngle, 0.01f);
	ImGui::DragFloat("AngleY", &yAngle, 0.01f);
	ImGui::DragFloat3("Position", &pos.x, 0.01f, -100.0f, 100.0f);

	Vector3f gndPos = myGroundTransform.GetPosition();
	ImGui::DragFloat3("groundPos", &gndPos.x);
	Vector3f gndScl = myGroundTransform.GetScale();
	ImGui::DragFloat3("groundScl", &gndScl.x);
	myGroundTransform.SetPosition(gndPos);
	myGroundTransform.SetScale(gndScl);


	if (HoveringOverRender())
	{
		static ImVec2 lastDelta = {};
		const auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);

		if (delta.x == 0 && delta.y == 0)
		{
			lastDelta = {};
		}

		xAngle += delta.y - lastDelta.y;
		yAngle += delta.x - lastDelta.x;

		lastDelta = delta;

		//get the scroll wheel delta
		const auto scroll = ImGui::GetIO().MouseWheel;
		distance += scroll * 0.5f;

		//get the right click delta
		static ImVec2 lastRightClickDelta = {};
		const auto rightClickDelta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
		if (rightClickDelta.x == 0 && rightClickDelta.y == 0)
		{
			lastRightClickDelta = {};
		}

		//pos += transform.GetRight() * -(rightClickDelta.x - lastRightClickDelta.x) * 0.1f;
		pos.y += (rightClickDelta.y - lastRightClickDelta.y) * 0.1f;		

		lastRightClickDelta = rightClickDelta;
	}


	//transform.TranslateLocal({ pos.x, 0.0f, pos.z });
	//transform.TranslateWorld({ 0.0f, pos.y, 0.0f });

	Transform transform;

	transform.GetMatrix() *= Matrix4x4f::CreateRotationAroundX(xAngle * KE::DegToRadImmediate);
	transform.GetMatrix() *= Matrix4x4f::CreateRotationAroundY(yAngle * KE::DegToRadImmediate);

	transform.SetPosition(pos);

	transform.TranslateLocal({ 0.0f, 0.0f, distance });

	cam->transform = transform;
}

void KE_EDITOR::ModelViewer::Render()
{
	if (ImGui::BeginTable("ModelViewerTable", 2, ImGuiTableFlags_Resizable))
	{
		static bool customAspect = false;
		static float customWidth = 16.0f;
		static float customHeight = 9.0f;

		ImVec2 size;

		ImGui::TableNextColumn();
		if (ImGui::BeginChild("imageChild"))
		{

			auto* graphics = KE_GLOBAL::editor->myGraphics;
			size = ImGui::GetContentRegionAvail();

			//make a drag drop target for models here
			auto oldPos = ImGui::GetCursorPos();

			ImGui::InvisibleButton("modelViewerDragDropButton", size);

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("modelPath"))
				{
					myModelData.myMeshList = &KE_GLOBAL::editor->myModelLoader->Load((const char*)payload->Data);
				}

				ImGui::EndDragDropTarget();
			}

			ImGui::SetCursorPos(oldPos);
			


			auto* cam = graphics->GetCameraManager().GetCamera(KE_DEBUG_CAMERA_INDEX - 1);
			cam->SetPerspective(size.x, size.y, 90.0f * KE::DegToRadImmediate, 0.1f, 100.0f);

			switch(myRenderType)
			{
				case RenderType::DeferredModel:
				{
					graphics->SetViewport(static_cast<int>(size.x), static_cast<int>(size.y));

					RenderCustom(
						RenderType::DeferredModel,
						myModelData.myRenderResources[0].myVertexShader,
						myModelData.myRenderResources[0].myPixelShader,
						myModelData.myRenderResources[0].myMaterial,
						{}
					);

					ImGui::Image(myRenderTarget->GetShaderResourceView(), size);
					lastRenderRectMin = ImGui::GetItemRectMin();
					lastRenderRectMax = ImGui::GetItemRectMax();

					break;
				}
				case RenderType::DeferredSkeletalModel:
				{
					break;
				}
				case RenderType::VFXModel:
				{
					//graphics->SetViewport(size.x, size.y);

					RenderCustom(
						RenderType::VFXModel,
						KE_GLOBAL::editor->myShaderLoader->GetVertexShader(SHADER_LOAD_PATH "Model_VFX_VS.cso"),
						myModelData.myRenderResources[0].myPixelShader,
						myModelData.myRenderResources[0].myMaterial,
						{}
					);

					ImGui::Image(myRenderTarget->GetShaderResourceView(), size);
					lastRenderRectMin = ImGui::GetItemRectMin();
					lastRenderRectMax = ImGui::GetItemRectMax();
					break;
				}
				case RenderType::FullscreenQuad:
				{
					RenderCustom(
						RenderType::FullscreenQuad,
						KE_GLOBAL::editor->myShaderLoader->GetVertexShader(SHADER_LOAD_PATH "FullscreenAsset_VS.cso"),
						KE_GLOBAL::editor->myShaderLoader->GetPixelShader(SHADER_LOAD_PATH "SDFRendering_PS.cso"),
						myModelData.myRenderResources[0].myMaterial,
						{}
					);

					float aspect = static_cast<float>(graphics->GetRenderWidth()) / static_cast<float>(graphics->GetRenderHeight());
					if (customAspect)
					{
						aspect = customWidth / customHeight;
					}

					ImGui::Image(myRenderTarget->GetShaderResourceView(), GetAspectRegion(size, aspect));
					lastRenderRectMin = ImGui::GetItemRectMin();
					lastRenderRectMax = ImGui::GetItemRectMax();

					break;
				}
			}

		}
		ImGui::EndChild();
		ImGui::TableNextColumn();

		ImGuiHandler::DisplayEnumSlider("Mode", &myRenderType, RenderType::Count);

		ImGui::Checkbox("Custom Aspect", &customAspect);
		if (customAspect)
		{
			ImGui::DragFloat("Width", &customWidth, 0.1f);
			ImGui::DragFloat("Height", &customHeight, 0.1f);
		}

		if (ImGui::BeginTabBar("assets"))
		{
			if (ImGui::BeginTabItem("model"))
			{
				ImGuiHandler::DisplayModelData(&myModelData);

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("ground"))
			{
				ImGuiHandler::DisplayModelData(&myModelViewerGround);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("dirlight"))
			{
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::EndTable();
	}


}

void KE_EDITOR::ModelViewer::RenderCustom(RenderType aType, KE::VertexShader* aVS, KE::PixelShader* aPS, KE::Material* aMaterial, const Vector2f& aViewportSize)
{
	auto* graphics = KE_GLOBAL::editor->myGraphics;
	auto& shaderLoader = graphics->GetShaderLoader();

	auto* cam = graphics->GetCameraManager().GetCamera(KE_DEBUG_CAMERA_INDEX-1);

	graphics->SetView(cam->GetViewMatrix());
	graphics->SetProjection(cam->GetProjectionMatrix());
	graphics->SetCommonBuffer(*cam);

	switch(aType)
	{
		case RenderType::DeferredModel:
		{
			myBuffer->SetAsActiveTarget(graphics->GetContext().Get(), myBuffer->GetDepthStencilView());

			{
				const KE::BasicRenderInput input(nullptr, cam->GetViewMatrix(), cam->GetProjectionMatrix(), aVS, aPS);
				graphics->GetDefaultRenderer()->RenderModel(input, myModelData);
			}
			{
				const KE::BasicRenderInput input(nullptr, cam->GetViewMatrix(), cam->GetProjectionMatrix());
				graphics->GetDefaultRenderer()->RenderModel(input, myModelViewerGround);
			}

			{
				const KE::BasicRenderInput input(
					nullptr,
					myLightManager.myDirectionalLightCamera.GetViewMatrix(),
					myLightManager.myDirectionalLightCamera.GetProjectionMatrix(),
					aVS,
					shaderLoader.GetPixelShader(SHADER_LOAD_PATH "Model_Shadow_PS.cso")
				);
				
				myLightManager.PrepareShadowPass(graphics);
				
				graphics->SetRasterizerState(KE::eRasterizerStates::FrontfaceCulling);
				graphics->GetDefaultRenderer()->RenderModel(input, myModelData);
				graphics->GetDefaultRenderer()->RenderModel(input, myModelViewerGround);
				graphics->SetRasterizerState(KE::eRasterizerStates::NoCulling);

			}

			myRenderTarget->MakeActive(false);
			myBuffer->SetAllAsResources(graphics->GetContext().Get(), 0u);
			graphics->GetContext()->PSSetShaderResources(15, 1, myLightManager.GetCubemap()->myShaderResourceView.GetAddressOf());
			myLightManager.Render(graphics);

			break;
		}
		case RenderType::VFXModel:
		{
			myBuffer->SetAsActiveTarget(graphics->GetContext().Get(), myBuffer->GetDepthStencilView());

			{
				graphics->SetRasterizerState(KE::eRasterizerStates::NoCulling);
				const KE::BasicRenderInput input(nullptr, cam->GetViewMatrix(), cam->GetProjectionMatrix());
				graphics->GetDefaultRenderer()->RenderModel(input, myModelViewerGround);

			}

			{
				const KE::BasicRenderInput input(
					nullptr,
					myLightManager.myDirectionalLightCamera.GetViewMatrix(),
					myLightManager.myDirectionalLightCamera.GetProjectionMatrix(),
					aVS,
					shaderLoader.GetPixelShader(SHADER_LOAD_PATH "Model_Shadow_PS.cso")
				);

				myLightManager.PrepareShadowPass(graphics);

				graphics->SetRasterizerState(KE::eRasterizerStates::FrontfaceCulling);
				graphics->GetDefaultRenderer()->RenderModel(input, myModelViewerGround);
				graphics->SetRasterizerState(KE::eRasterizerStates::NoCulling);

			}

			myRenderTarget->MakeActive(false);
			myBuffer->SetAllAsResources(graphics->GetContext().Get(), 0u);
			graphics->GetContext()->PSSetShaderResources(15, 1, myLightManager.GetCubemap()->myShaderResourceView.GetAddressOf());
			myLightManager.Render(graphics);

			graphics->SetView(cam->GetViewMatrix());
			graphics->SetProjection(cam->GetProjectionMatrix());
			graphics->SetCommonBuffer(*cam);

			graphics->SetViewport(2560, 1440);

			myBuffer->SetAllAsResources(graphics->GetContext().Get(), 8u);
			myRenderTarget->MakeActive(true, myBuffer->GetDepthStencilView());
			graphics->SetDepthStencilState(KE::eDepthStencilStates::ReadOnlyLess);
			graphics->SetBlendState(KE::eBlendStates::VFXBlend);
			{
				KE::fxBuffer vfxData;
				vfxData.colour = { 1.0f, 1.0f, 1.0f, 1.0f };

				graphics->GetVFXManager().RenderVFX(vfxData, &myModelData);

			}
			graphics->SetDepthStencilState(KE::eDepthStencilStates::Write);
			graphics->SetBlendState(KE::eBlendStates::Disabled);



			break;
		}
		case RenderType::DeferredSkeletalModel:
		{
			
		}
		case RenderType::FullscreenQuad:
		{
			myRenderTarget->MakeActive(false);
			myBuffer->SetAllAsResources(graphics->GetContext().Get(), 4u); // retain 0, 1, 2, 3 for input textures

			graphics->BindMaterial(aMaterial, 0u);

			myFullscreenAsset.RenderWithoutSRV(graphics, aVS, aPS);
		}
	}


	

}

#endif