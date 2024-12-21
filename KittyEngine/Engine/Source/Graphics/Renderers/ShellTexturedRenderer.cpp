#include "stdafx.h"
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/Renderers/ShellTexturedRenderer.h"

#include <d3d11.h>

#include "Engine/Source/Graphics/Graphics.h"

namespace WRL = Microsoft::WRL;
void KE::ShellTexturedRenderer::Init(Graphics* aGraphics)
{
	myGraphics = aGraphics;
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.ByteWidth = sizeof(ShellTexturingBufferData);
		bufferDesc.StructureByteStride = 0u;

		myRenderingBuffer.Init(aGraphics->GetDevice(), &bufferDesc);
	}

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.ByteWidth = sizeof(ShellTextureDisplacementBufferData);
		bufferDesc.StructureByteStride = 0u;

		myDisplacementBuffer.Init(aGraphics->GetDevice(), &bufferDesc);
	}

	{
		ModelLoader& modelLoader     = myGraphics->GetModelLoader();
		TextureLoader& textureLoader = myGraphics->GetTextureLoader();
		ShaderLoader& shaderLoader   = myGraphics->GetShaderLoader();

		myDisplacementModelData.myMeshList = &modelLoader.Load("Data/EngineAssets/Sphere.fbx");
		myDisplacementModelData.myTransform = &myDisplacementTransform.GetMatrix();
		myDisplacementModelData.myRenderResources.emplace_back(
			shaderLoader.GetVertexShader(SHADER_LOAD_PATH "Model_Deferred_VS.cso"),
			shaderLoader.GetPixelShader(SHADER_LOAD_PATH "Displacement_PS.cso"),
			textureLoader.GetDefaultMaterial()
		);
	}
}

void KE::ShellTexturedRenderer::Render(const ShellTexturingRenderInput& someInput, const ModelData& aModel)
{
	if (someInput.attributes.shellCount <= 0) { return;	}

	int shellIndexOffset = 0;
	int shellCount = someInput.attributes.shellCount;
	int shellCountPerDrawCall = someInput.attributes.shellCountPerDrawCall;

	for (int i = 0; i < shellCount; i += shellCountPerDrawCall)
	{
		int shellsToDraw = shellCount - i;
		if (shellsToDraw > shellCountPerDrawCall) { shellsToDraw = shellCountPerDrawCall; }

		const UINT stride = sizeof(Vertex);
		const UINT offset = 0u;

		myGraphics->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		ShellTexturingBufferData bufferData = {};
		bufferData.viewMatrix = someInput.viewMatrix;
		bufferData.projectionMatrix = someInput.projectionMatrix;
		bufferData.modelToWorld = *aModel.myTransform;
		bufferData.attributes = someInput.attributes;
		bufferData.attributes.shellIndexOffset = shellIndexOffset;

		myRenderingBuffer.MapBuffer(&bufferData, sizeof(bufferData), myGraphics->GetContext().Get());
		myRenderingBuffer.BindForVS(7, myGraphics->GetContext().Get());
		myRenderingBuffer.BindForPS(7, myGraphics->GetContext().Get());

		VertexShader* vs = someInput.overrideVertexShader ? someInput.overrideVertexShader : aModel.myRenderResources[0].myVertexShader;
		PixelShader* ps = someInput.overridePixelShader ? someInput.overridePixelShader : aModel.myRenderResources[0].myPixelShader;

		myGraphics->GetContext()->IASetInputLayout(vs->GetInputLayout());
		myGraphics->GetContext()->VSSetShader(vs->GetShader(), nullptr, 0u);
		myGraphics->GetContext()->PSSetShader(ps->GetShader(), nullptr, 0u);

		for (const auto& mesh : aModel.myMeshList->myMeshes)
		{
			myGraphics->GetContext()->IASetVertexBuffers(0u, 1u, mesh.myVertexBuffer.GetAddressOf(), &stride, &offset);
			myGraphics->GetContext()->IASetIndexBuffer(mesh.myIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);

			myGraphics->GetContext()->DrawIndexedInstanced(
				mesh.GetIndexCount(),
				shellsToDraw, 
				0u, 
				0u,
				0u
			);

			myGraphics->AddDrawCall();
		}

		shellIndexOffset += shellsToDraw;
	}

}

void KE::ShellTexturedRenderer::ClearDisplacement(LaserPtr<ShellModelData> aModel)
{
	constexpr float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	aModel->effectsRT->Clear(clearColor);
}

void KE::ShellTexturedRenderer::FadeDisplacement(LaserPtr<ShellModelData> aModel, float aFadeSpeed, float aMaxFade)
{
	auto* modelRT = aModel->effectsRT;
	if (!modelRT) { return; }

	auto* rt11 = myGraphics->GetRenderTarget(11);

	rt11->CopyFrom(modelRT);
	rt11->SetAsShaderResource(4);
	myGraphics->GetContext()->PSSetShaderResources(
		5,
		1, 
		aModel->displacementTexture->myShaderResourceView.GetAddressOf()
	);


	auto& shaderLoader = myGraphics->GetShaderLoader();
	myGraphics->SetBlendState(eBlendStates::Disabled);

	myGraphics->GetFullscreenAsset()->RenderWithoutSRV(
		myGraphics,
		shaderLoader.GetVertexShader(SHADER_LOAD_PATH "FullscreenAsset_VS.cso"),
		shaderLoader.GetPixelShader(SHADER_LOAD_PATH "DisplacementFade_PS.cso")
	);
}

void KE::ShellTexturedRenderer::RenderDisplacement(
	LaserPtr<ShellModelData> aModel,
	const std::vector<ShellTextureDisplacement>& someDisplacements,
	const Vector3f& boundsMinimum,
	const Vector3f& boundsMaximum,
	Camera* aOriginalCamera,
	int aChannelIndex
)
{

	auto* modelRT = aModel->effectsRT;
	if (!modelRT) { return; }
	modelRT->MakeActive(false);
	
	Camera displacementCamera;

	const Vector3f centerPos  = (boundsMinimum + boundsMaximum) / 2.0f;
	const Vector3f boundsSize = boundsMaximum - boundsMinimum;

	displacementCamera.SetOrthographic(boundsSize.x, boundsSize.z, -100.0f, 100.0f);
	displacementCamera.transform.SetPosition(centerPos);

	displacementCamera.transform.RotateLocal({ 90.0f * KE::DegToRadImmediate, 0.0f, 0.0f });

	myGraphics->SetView(displacementCamera.GetViewMatrix());
	myGraphics->SetProjection(displacementCamera.GetProjectionMatrix());
	myGraphics->SetCommonBuffer(displacementCamera);
	
	myGraphics->SetViewport(1024, 1024);

	auto* rt11 = myGraphics->GetRenderTarget(11);
	rt11->CopyFrom(modelRT);
	rt11->SetAsShaderResource(4);

	ShellTextureDisplacementBufferData displacementBufferData;
	displacementBufferData.channelIndex = aChannelIndex;

	myDisplacementBuffer.MapBuffer(
		&displacementBufferData.channelIndex,
		sizeof(displacementBufferData),
		myGraphics->GetContext().Get()
	);
	myDisplacementBuffer.BindForPS(5, myGraphics->GetContext().Get());

	BasicRenderer* basicRenderer = myGraphics->GetDefaultRenderer();
	for (const auto& disp : someDisplacements)
	{
		myDisplacementTransform.SetPosition(disp.position);
		myDisplacementTransform.SetScale(disp.scale);

		BasicRenderInput displacementRenderIn{
			nullptr,
			displacementCamera.GetViewMatrix(),
			displacementCamera.GetProjectionMatrix(),
			nullptr,
			nullptr
		};

		basicRenderer->RenderModel(displacementRenderIn, myDisplacementModelData);
	}

	if (!aOriginalCamera) { return; }
	myGraphics->SetViewport(
		static_cast<int>(aOriginalCamera->GetProjectionData().perspective.width), 
		static_cast<int>(aOriginalCamera->GetProjectionData().perspective.height)
	);

	myGraphics->SetView(aOriginalCamera->GetViewMatrix());
	myGraphics->SetProjection(aOriginalCamera->GetProjectionMatrix());
	myGraphics->SetCommonBuffer(*aOriginalCamera);
}

void KE::ShellTexturedRenderer::ResetDisplacement(LaserPtr<ShellModelData> aModel)
{
	auto* modelRT = aModel->effectsRT;
	if (!modelRT) { return; }

	float colour[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	modelRT->Clear(colour);
}

//				//if (ImGui::IsMouseDown(0))
//				{
//				float colour[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
//					static float scrollLevel = 1.0f;
//					static float scrollLevel2 = 1.0f;
//					const float wheel = ImGui::GetIO().MouseWheel;
//					if (ImGui::GetIO().KeyCtrl)
//					{
//						scrollLevel2 += wheel;	
//					}
//					else
//					{
//						scrollLevel += wheel;
//					}
//					const float sclx = 1.0f + (scrollLevel * 0.1f);
//					const float scly = 1.0f + (scrollLevel2 * 0.1f);
//
//					dt.SetScale({sclx,scly,sclx});
//
//					auto mp = KE_GLOBAL::blackboard.Get<InputHandler>("inputHandler")->GetMousePosition();
//
//					Rayf painterRay = camera->GetRay({(float)mp.x, (float)mp.y});
//
//					//calculate the point of intersection with the plane,
//					//in this case we can just move along the direction until its y is 0
//					Vector3f planeHit = painterRay.GetOrigin() + painterRay.GetDirection()* (-painterRay.GetOrigin().y / painterRay.GetDirection().y);
//
//					if (autoclear) { myRenderTargets[10].Clear(colour); }
//					myRenderTargets[11].CopyFrom(&myRenderTargets[10]);
//					myRenderTargets[11].SetAsShaderResource(4);
//
//					myRenderTargets[10].MakeActive(false);
//
//					Camera displacementCamera;
//					const Vector3f& scale = t.GetScale();
//					displacementCamera.SetOrthographic(scale.x, scale.z, -100.0f, 100.0f);
//
//					//displacementCamera.transform.SetPosition(planeHit * -1.0f);
//					auto* ob = KE_GLOBAL::blackboard.Get<GameObjectManager>("gameObjectManager")->GetGameObject(-9999);
//					if(ob)
//					{
//						dt.SetPosition(
//							ob->myTransform.GetPosition()
//						);
//					}
//
//
//					displacementCamera.transform.RotateLocal({ 90.0f * KE::DegToRadImmediate, 0.0f, 0.0f });
//
//					SetView(displacementCamera.GetViewMatrix());
//					SetProjection(displacementCamera.GetProjectionMatrix());
//					SetCommonBuffer(displacementCamera);
//
//					SetViewport(1024, 1024);
//
//					BasicRenderInput displacementRenderIn{ nullptr, displacementCamera.GetViewMatrix(), displacementCamera.GetProjectionMatrix(), nullptr,nullptr };
//
//					myDefaultRenderer.RenderModel(displacementRenderIn, displacementMD);
//
//					SetViewport(myWidth, myHeight);
//					SetView(camera->GetViewMatrix());
//					SetProjection(camera->GetProjectionMatrix());
//					SetCommonBuffer(*camera);
//				}
//
//			
//
//				myRenderTargets[10].SetAsShaderResource(4);
//
//
//
//				ShellTexturingRenderInput shellIn{
//					camera->GetViewMatrix(),
//					camera->GetProjectionMatrix(),
//					shellAttr
//				};
//				myShellTexturedRenderer.Render(shellIn, modelData);
//
//				//for (int l = 0; l < shellTexBuf.shellCount; l++)
//				//{
//				//	shellTexBuf.shellIndex = l;
//				//	shellBuf.MapBuffer(&shellTexBuf, sizeof(shellTexBuf), myContext.Get());
//				//
//				//	shellBuf.BindForPS(7, myContext.Get());
//				//	shellBuf.BindForVS(7, myContext.Get());
//				//	
//				//	myDefaultRenderer.RenderModel(in, modelData);
//				//}
//
//
//			}
//#endif