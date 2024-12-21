#pragma once
#include <d3d11.h>

#include "Renderer.h"
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/Decals/DecalManager.h"

namespace KE
{
	class PixelShader;
	class VertexShader;
	class Graphics;
	class CBuffer;
	struct RenderResources;
	//
	struct DecalRenderBufferData
	{
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;
	};

	struct DecalObjectBufferData
	{
		DirectX::XMMATRIX objectToWorld;
		DirectX::XMMATRIX worldToObject;
		Vector4f textureIntensities;
	};

	struct DecalRenderInput
	{
		std::vector<size_t>* decalIndices;
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;

		VertexShader* overrideVertexShader = nullptr;
		PixelShader* overridePixelShader = nullptr;
	};

	class DecalRenderer : public Renderer
	{
	private:
		CBuffer myRenderingBuffer{};
		CBuffer myDecalBuffer{};

		std::vector<size_t> myDecalIndices{};
		MeshList* myDecalMesh = nullptr;

	public:
		void Init(Graphics* aGraphics);

		void RenderDecal(const DecalRenderInput& aInput, const Decal& aDecal);

		void Render(const DecalRenderInput& aInput);
		void AddDecalIndex(size_t aDecalIndex);
	};

}
