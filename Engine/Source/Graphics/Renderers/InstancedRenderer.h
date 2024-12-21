#pragma once
#include <d3d11.h>

#include "Renderer.h"
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/ModelData.h"

namespace KE
{

	constexpr size_t INSTANCING_BUFFER_SIZE = 1024;

	class Graphics;
	class CBuffer;
	struct RenderResources;
	struct InstancedRenderPackage;

	typedef std::vector<InstancedRenderPackage> InstancedRenderPackageList;

	struct InstancedRenderPackage
	{
		std::vector<size_t> modelDataIndices;
		RenderResourceList renderResources;
		float distanceToCamera = 0.0f;

		bool TryAddModelData(size_t aModelDataIndex, const RenderResourceList& aRenderResourceSet)
		{
			if (aRenderResourceSet.size() != renderResources.size())
			{
				return false;
			}

			for (size_t i = 0; i < aRenderResourceSet.size(); ++i)
			{
				if (aRenderResourceSet[i].myVertexShader != renderResources[i].myVertexShader ||
					aRenderResourceSet[i].myPixelShader != renderResources[i].myPixelShader ||
					aRenderResourceSet[i].myMaterial != renderResources[i].myMaterial)
				{
					return false;
				}
			}

			modelDataIndices.push_back(aModelDataIndex);
			return true;
		}
	};

	//

	struct InstancedRenderData
	{
		DirectX::XMMATRIX myTransform;
		Vector4f attributes;

		InstancedRenderData(const DirectX::XMMATRIX& aTransform) : myTransform(aTransform), attributes({0,0,1,1}) {}
	};

	//

	struct InstanceRenderInput
	{
		InstancedRenderPackageList* renderPackages;

		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;

		VertexShader* overrideVertexShader = nullptr;
		PixelShader* overridePixelShader = nullptr;
	};

	struct InstanceRenderBufferData
	{
		DirectX::XMMATRIX worldToClip;
	};

	//

	class InstancedRenderer : public Renderer
	{
	private:
		CBuffer myRenderingBuffer{};
		CBuffer myInstanceBuffer{};

		void RenderPackage(const InstancedRenderPackage& aPackage, const InstanceRenderInput& aInput);
	public:
		void Init(Graphics* aGraphics);

		void SortRenderPackages(InstancedRenderPackageList* aPackageList);
		void GenerateInstancingData(const std::vector<size_t>& aModelDataIndices, const ModelDataList& aModelDatas, InstancedRenderPackageList* aOutRenderPackages);
		void Render(const InstanceRenderInput& aInput);

	};

}
