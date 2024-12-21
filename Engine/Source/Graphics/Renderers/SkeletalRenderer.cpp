#include "stdafx.h"
#include "Engine/Source/Graphics/ModelData.h"
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/Renderers/SkeletalRenderer.h"

#include <d3d11.h>

#include "Engine/Source/Graphics/Graphics.h"

void KE::SkeletalRenderer::Init(Graphics* aGraphics)
{
	Renderer::Init(aGraphics);

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.ByteWidth = sizeof(SkeletalObjectBufferData);
		bufferDesc.StructureByteStride = 0u;

		myObjectBuffer.Init(aGraphics->GetDevice(), &bufferDesc);
	}
	//
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.ByteWidth = sizeof(SkeletalAnimationBufferData);
		bufferDesc.StructureByteStride = 0u;

		myAnimationBuffer.Init(aGraphics->GetDevice(), &bufferDesc);
	}
}

void KE::SkeletalRenderer::Render(const SkeletalRenderInput& aInput)
{
	const auto& indices = *aInput.skeletalModelDataIndices;
	const SkeletalModelDataList& modelDatas = myGraphics->GetSkeletalModelData();
	for (const size_t& index : indices)
	{
		if (!modelDatas[index].myActiveStatus) { continue; }
		RenderSkeletalModel(aInput, modelDatas[index]);
	}
}

void KE::SkeletalRenderer::RenderSkeletalModel(const SkeletalRenderInput& aInput, const SkeletalModelData& aModelData)
{
	const RenderResourceList& renderResources = aModelData.myRenderResources;
	SkeletalMeshList* meshList = aModelData.myMeshList;

	const auto& graphicsContext = myGraphics->GetContext();
	const auto& graphicsDevice = myGraphics->GetDevice();

	for (unsigned int i = 0; i < (unsigned int)meshList->myMeshes.size(); i++)
	{
		SkeletalMesh& mesh = meshList->myMeshes[i];


		int renderResourceIndex = 0;
		if (renderResources.size() > 1)
		{
			renderResourceIndex = (int)i;
		}

		auto& resource = renderResources[renderResourceIndex];

		const PixelShader* pixelShader = aInput.overridePixelShader ?
			aInput.overridePixelShader :
			resource.myPixelShader;


		const VertexShader* vertexShader = aInput.overrideVertexShader ?
			aInput.overrideVertexShader :
			resource.myVertexShader;


		const Material* material = resource.myMaterial;

		if (resource.myCBuffer)
		{
			if (resource.myCBufferVSSlot >= 0)
			{
				resource.myCBuffer->BindForVS(
					resource.myCBufferVSSlot,
					graphicsContext.Get()
				);
			}

			if (resource.myCBufferPSSlot >= 0)
			{
				resource.myCBuffer->BindForPS(
					resource.myCBufferPSSlot,
					graphicsContext.Get()
				);
			}
		}



		constexpr unsigned int stride = sizeof(BoneVertex);
		constexpr unsigned int offset = 0;

		myGraphics->BindMaterial(material, 0u);

		graphicsContext->IASetVertexBuffers(0u, 1u, mesh.myVertexBuffer.GetAddressOf(), &stride, &offset);
		graphicsContext->IASetIndexBuffer(mesh.myIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);

		SkeletalObjectBufferData objectData = {};

		objectData.objectToWorld = *aModelData.myTransform;
		objectData.objectToClip = objectData.objectToWorld * aInput.viewMatrix * aInput.projectionMatrix;
		

		myObjectBuffer.MapBuffer(
			(void*)&objectData,
			sizeof(objectData),
			graphicsContext.Get()
		);
		myObjectBuffer.BindForVS(1, graphicsContext.Get());


		myAnimationBuffer.MapBuffer(
			(void*)aModelData.myFinalTransforms.data(),
			sizeof(*aModelData.myFinalTransforms.data()) * static_cast<int>(aModelData.myFinalTransforms.size()),
			graphicsContext.Get()
		);

		myAnimationBuffer.BindForVS(4, graphicsContext.Get());

		//
		graphicsContext->PSSetShader(pixelShader->GetShader(), nullptr, 0u);
		graphicsContext->VSSetShader(vertexShader->GetShader(), nullptr, 0u);
		graphicsContext->IASetInputLayout(vertexShader->GetInputLayout());
		graphicsContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//
		graphicsContext->DrawIndexed(mesh.GetIndexCount(), 0u, 0u);
	}
}
