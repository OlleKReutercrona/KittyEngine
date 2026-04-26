#pragma once
#include <d3d11.h>

#include "Renderer.h"
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/ModelData.h"

namespace KE
{
	class Graphics;
	class CBuffer;
	struct RenderResources;

	struct SkeletalObjectBufferData
	{
		DirectX::XMMATRIX objectToWorld;
		DirectX::XMMATRIX objectToClip;
	};

	struct SkeletalAnimationBufferData
	{
		DirectX::XMMATRIX boneTransforms[KE_MAX_BONES];
	};

	struct SkeletalRenderInput
	{
		std::vector<size_t>* skeletalModelDataIndices;

		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;

		VertexShader* overrideVertexShader = nullptr;
		PixelShader* overridePixelShader = nullptr;
	};

	class SkeletalRenderer : public Renderer
	{
	private:
		CBuffer myObjectBuffer{};
		CBuffer myAnimationBuffer{};
		
	public:
		void Init(Graphics* aGraphics);
		
		void Render(const SkeletalRenderInput& aInput);
		void RenderSkeletalModel(const SkeletalRenderInput& aInput, const SkeletalModelData& aModelData);
	};

}
