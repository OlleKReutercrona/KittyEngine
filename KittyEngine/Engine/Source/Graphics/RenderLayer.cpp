#include "stdafx.h"
#include "ModelData.h"
#include "RenderLayer.h"

#include "Camera.h"
#include "Graphics.h"

void KE::RenderLayer::Init(Graphics* aGraphics)
{
	myGraphics = aGraphics;

	myProcessedRender.Init(
		aGraphics->GetDevice().Get(),
		aGraphics->GetContext().Get(),
		aGraphics->GetRenderWidth(),
		aGraphics->GetRenderHeight()
	);
}

void KE::RenderLayer::ApplySettings()
{
	if (mySettings.depthStencilState != eDepthStencilStates::Count)
	{
		myGraphics->SetDepthStencilState(mySettings.depthStencilState);
	}
	if (mySettings.rasterizerState != eRasterizerStates::Count)
	{
		myGraphics->SetRasterizerState(mySettings.rasterizerState);
	}
	if (mySettings.blendState != eBlendStates::Count)
	{
		myGraphics->SetBlendState(mySettings.blendState);
	}
}

void KE::RenderLayer::SetActive()
{
	//myGBuffer.SetAsActiveTarget(myGraphics->GetContext().Get(), myGBuffer.GetDepthStencilView());
}

void KE::RenderLayer::Render(KE::Camera* aCamera, KE::VertexShader* aVSOverride, KE::PixelShader* aPSOverride)
{
	if (!(flags & RenderLayerFlags_Active)) { return; }


	//ApplySettings();
	//SetActive();

	const DirectX::XMMATRIX& viewMatrix = aCamera->GetViewMatrix();
	const DirectX::XMMATRIX& projectionMatrix = aCamera->GetProjectionMatrix();

	myInstancedRenderer->Render({
		&myInstancedRenderPackages,
		viewMatrix,
		projectionMatrix,
		aVSOverride,
		aPSOverride,
		});


	myBasicRenderer->Render({
		&myRegularModelIndices,
		viewMatrix,
		projectionMatrix,
		aVSOverride,
		aPSOverride,
	});
	
	mySkeletalRenderer->Render({
		&mySkeletalModelDataIndices,
		viewMatrix,
		projectionMatrix,
		aVSOverride,
		aPSOverride,
	});
}

void KE::RenderLayer::RenderDecals(Camera* aCamera, GBuffer* aMainGBuffer, GBuffer* aCopyGBuffer, DecalManager* aDecalManager)
{
	if (!(flags & RenderLayerFlags_Active)) { return; }

	myGraphics->SetDepthStencilState(KE::eDepthStencilStates::ReadOnlyLess);
	auto* context = myGraphics->GetContext().Get();

	aDecalManager->PrepareDecalRendering(myGraphics, aMainGBuffer, aCopyGBuffer);

	aMainGBuffer->SetAsActiveTarget(context, aMainGBuffer->GetDepthStencilView());

	myDecalRenderer->Render({
		&myDecalIndices,
		aCamera->GetViewMatrix(),
		aCamera->GetProjectionMatrix(),
	});

	aMainGBuffer->UnbindTarget(context);
	aMainGBuffer->SetAllAsResources(context, 0u);
	myGraphics->SetDepthStencilState(KE::eDepthStencilStates::Write);
}

void KE::RenderLayer::AddModelDataIndex(size_t aModelIndex)
{
	myModelDataIndices.push_back(aModelIndex);
}

void KE::RenderLayer::AddSkeletalModelDataIndex(size_t aModelIndex)
{
	mySkeletalModelDataIndices.push_back(aModelIndex);
}

void KE::RenderLayer::AddDecalIndex(size_t aDecalIndex)
{
	myDecalIndices.push_back(aDecalIndex);
}

void KE::RenderLayer::GenerateInstancingData(const ModelDataList& aModelDataList)
{
	myInstancedRenderPackages.clear();
	myInstancedRenderer->GenerateInstancingData(myModelDataIndices, aModelDataList, &myInstancedRenderPackages);
	for (size_t i : myModelDataIndices)
	{
		if (!aModelDataList[i].myIsInstanced)
		{
			myRegularModelIndices.push_back(i);
		}
	}

}

void KE::RenderLayer::NewFrame()
{
	float clearColour[4] = { 0.0f,0.0f,0.0f,0.0f };
	myProcessedRender.Clear(clearColour, myGraphics->GetContext().Get());
}

void KE::RenderLayer::Reset()
{
	myModelDataIndices.clear();
	myRegularModelIndices.clear();
	mySkeletalModelDataIndices.clear();
	myInstancedRenderPackages.clear();
	myDecalIndices.clear();
}

void KE::RenderLayer::Resize(Vector2i aSize)
{
	myProcessedRender.Resize(aSize.x, aSize.y);
}
