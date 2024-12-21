#pragma once
#include <d3d11.h>

#include "Renderer.h"
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/ModelData.h"

namespace KE
{
	class Graphics;
	class CBuffer;
	struct RenderResources;	//

	struct ObjectBufferData
	{
		DirectX::XMMATRIX objectToWorld;
		DirectX::XMMATRIX objectToClip;
	};

	struct BasicRenderInput
	{
		std::vector<size_t>* modelDataIndices;

		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;

		VertexShader* overrideVertexShader = nullptr;
		PixelShader* overridePixelShader = nullptr;
	};

	class BasicRenderer : public Renderer
	{
	private:
		CBuffer myRenderingBuffer{};
		std::vector<size_t> myModelDataIndices{};

	public:
		void Init(Graphics* aGraphics);
		void RenderModel(const BasicRenderInput& aInput, const ModelData& aModelData);

		void Render(const BasicRenderInput& aInput);
		void AddModelDataIndex(size_t aModelIndex);
	};

}
