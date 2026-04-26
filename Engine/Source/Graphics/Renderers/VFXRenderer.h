#pragma once
#include <d3d11.h>

#include "Renderer.h"
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/ModelData.h"

namespace KE
{
	//class Graphics;
	//class CBuffer;
	//struct RenderResources;	//
	//
	//struct VFXObjectBufferData
	//{
	//	DirectX::XMMATRIX objectToWorld;
	//	DirectX::XMMATRIX objectToClip;
	//};
	//
	//struct VFXRenderInput
	//{
	//	DirectX::XMMATRIX viewMatrix;
	//	DirectX::XMMATRIX projectionMatrix;
	//
	//	VertexShader* overrideVertexShader = nullptr;
	//	PixelShader* overridePixelShader = nullptr;
	//
	//	DirectX::XMMATRIX* relative = nullptr;
	//};
	//
	//class VFXRenderer : public Renderer
	//{
	//private:
	//	CBuffer myRenderingBuffer{};
	//	std::vector<size_t> myVFXIndices{};
	//
	//public:
	//	void Init(Graphics* aGraphics);
	//	void RenderModel(const VFXRenderInput& aInput, const ModelData& aModelData);
	//
	//	void Render(const VFXRenderInput& aInput);
	//	void AddVFXIndex(size_t aVFXIndex);
	//};

}
