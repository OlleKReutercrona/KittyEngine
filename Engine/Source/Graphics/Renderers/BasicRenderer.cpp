#include "stdafx.h"
#include "Engine/Source/Graphics/ModelData.h"
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/Renderers/BasicRenderer.h"

#include <d3d11.h>

#include "Engine/Source/Graphics/Graphics.h"

namespace WRL = Microsoft::WRL;
void KE::BasicRenderer::Init(Graphics* aGraphics)
{
	myGraphics = aGraphics;
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.ByteWidth = sizeof(ObjectBufferData);
		bufferDesc.StructureByteStride = 0u;

		myRenderingBuffer.Init(aGraphics->GetDevice(), &bufferDesc);
	}
}

void KE::BasicRenderer::RenderModel(const BasicRenderInput& aInput, const ModelData& aModelData)
{
	namespace WRL = Microsoft::WRL;

	const std::vector<RenderResources> renderResources = aModelData.myRenderResources;
	MeshList* meshList = aModelData.myMeshList;
	auto& shaderLoader = myGraphics->GetShaderLoader();
	const auto& graphicsContext = myGraphics->GetContext();

	/// IN FROM MODEL

	for (int i = 0; i < static_cast<int>(meshList->myMeshes.size()); ++i)
	{
		Mesh& mesh = meshList->myMeshes[i];

		int renderResourceIndex = 0;
		if (renderResources.size() > 1)
		{
			renderResourceIndex = i;
		}

		const auto* vertexShader = aInput.overrideVertexShader ?
			aInput.overrideVertexShader : //use overriden shader if it exists
			renderResources[renderResourceIndex].myVertexShader;

		const auto* pixelShader = aInput.overridePixelShader ?
			aInput.overridePixelShader : //use overriden shader if it exists
			renderResources[renderResourceIndex].myPixelShader;

		if (renderResources[renderResourceIndex].myCBuffer)
		{
			if (renderResources[renderResourceIndex].myCBufferVSSlot >= 0)
			{
				renderResources[renderResourceIndex].myCBuffer->BindForVS(
					renderResources[renderResourceIndex].myCBufferVSSlot,
					graphicsContext.Get()
				);
			}

			if (renderResources[renderResourceIndex].myCBufferPSSlot >= 0)
			{
				renderResources[renderResourceIndex].myCBuffer->BindForPS(
					renderResources[renderResourceIndex].myCBufferPSSlot,
					graphicsContext.Get()
				);
			}
		}


		constexpr UINT stride = sizeof(Vertex);
		constexpr UINT offset = 0u;

		const Material* material = renderResources[renderResourceIndex].myMaterial;

		myGraphics->BindMaterial(material, 0u);


		graphicsContext->IASetVertexBuffers(0u, 1u, mesh.myVertexBuffer.GetAddressOf(), &stride, &offset);
		graphicsContext->IASetIndexBuffer(mesh.myIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);

		ObjectBufferData objectData;
		objectData.objectToWorld = *aModelData.myTransform;
		objectData.objectToClip = objectData.objectToWorld * aInput.viewMatrix * aInput.projectionMatrix;
		
		myRenderingBuffer.MapBuffer(&objectData, sizeof(objectData), graphicsContext.Get());
		myRenderingBuffer.BindForVS(1, graphicsContext.Get());
		myRenderingBuffer.BindForPS(1, graphicsContext.Get());

		graphicsContext->PSSetShader(pixelShader->GetShader(), nullptr, 0u);
		graphicsContext->VSSetShader(vertexShader->GetShader(), nullptr, 0u);
		graphicsContext->IASetInputLayout(vertexShader->GetInputLayout());

		graphicsContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		myGraphics->DrawIndexed(mesh.GetIndexCount(), 0u, 0u);
	}
}

void KE::BasicRenderer::Render(const BasicRenderInput& aInput)
{
	const auto& indices = *aInput.modelDataIndices;
	const ModelDataList& modelData = myGraphics->GetModelData();
	for (const size_t& index : indices)
	{
		RenderModel(aInput, modelData[index]);
	}
}

void KE::BasicRenderer::AddModelDataIndex(size_t aModelIndex)
{
	myModelDataIndices.push_back(aModelIndex);
}
