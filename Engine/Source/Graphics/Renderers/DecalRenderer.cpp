#include "stdafx.h"
#include "Engine/Source/Graphics/ModelData.h"
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/Renderers/DecalRenderer.h"

#include <d3d11.h>

#include "Engine/Source/Graphics/Graphics.h"

namespace WRL = Microsoft::WRL;
void KE::DecalRenderer::Init(Graphics* aGraphics)
{
	myGraphics = aGraphics;
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.ByteWidth = sizeof(DecalRenderBufferData);
		bufferDesc.StructureByteStride = 0u;

		myRenderingBuffer.Init(aGraphics->GetDevice(), &bufferDesc);
	}

	myGraphics = aGraphics;
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.ByteWidth = sizeof(DecalRenderBufferData);
		bufferDesc.StructureByteStride = 0u;

		myDecalBuffer.Init(aGraphics->GetDevice(), &bufferDesc);
	}

	myDecalMesh = &aGraphics->GetModelLoader().Load("Data/EngineAssets/Cube.fbx");
}

void KE::DecalRenderer::RenderDecal(const DecalRenderInput& aInput, const Decal& aDecal)
{
	const auto& mesh = myDecalMesh->myMeshes[0];
	//
	myGraphics->BindMaterial(aDecal.myMaterial, 8u);

	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0u;

	myGraphics->GetContext()->IASetVertexBuffers(
		0u,
		1u,
		mesh.myVertexBuffer.GetAddressOf(),
		&stride,
		&offset
	);

	myGraphics->GetContext()->IASetIndexBuffer(mesh.myIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);

	DecalObjectBufferData bufferData;
	bufferData.objectToWorld = aDecal.myTransform.GetMatrix();
	bufferData.worldToObject = DirectX::XMMatrixInverse(nullptr, bufferData.objectToWorld);
	bufferData.textureIntensities = aDecal.myTextureIntensities;

	myDecalBuffer.MapBuffer(&bufferData, sizeof(bufferData), myGraphics->GetContext().Get());
	myDecalBuffer.BindForVS(1u, myGraphics->GetContext().Get());
	myDecalBuffer.BindForPS(1u, myGraphics->GetContext().Get());

	myGraphics->DrawIndexed(mesh.GetIndexCount(), 0u, 0u);
}

void KE::DecalRenderer::Render(const DecalRenderInput& aInput)
{
	const auto& decals = myGraphics->GetDecalManager().GetDecals();
	const auto& graphicsContext = myGraphics->GetContext();
	auto& shaderLoader = myGraphics->GetShaderLoader();

	graphicsContext->PSSetShader(shaderLoader.GetPixelShader(SHADER_LOAD_PATH "Decal_Deferred_PS.cso")->GetShader(), nullptr, 0u);
	graphicsContext->VSSetShader(shaderLoader.GetVertexShader(SHADER_LOAD_PATH "Decal_Deferred_VS.cso")->GetShader(), nullptr, 0u);
	graphicsContext->IASetInputLayout(shaderLoader.GetVertexShader(SHADER_LOAD_PATH "Decal_Deferred_VS.cso")->GetInputLayout());
	graphicsContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DecalRenderBufferData bufferData;
	bufferData.projectionMatrix = aInput.projectionMatrix;
	bufferData.viewMatrix = aInput.viewMatrix;

	myRenderingBuffer.MapBuffer(&bufferData, sizeof(bufferData), graphicsContext.Get());
	myRenderingBuffer.BindForVS(11u, graphicsContext.Get());
	myRenderingBuffer.BindForPS(11u, graphicsContext.Get());

	const auto& decalIndices = *aInput.decalIndices;
	for (const size_t& index : decalIndices)
	{
		RenderDecal(aInput, decals[index]);
	}
}

void KE::DecalRenderer::AddDecalIndex(size_t aDecalIndex)
{
	myDecalIndices.push_back(aDecalIndex);
}
