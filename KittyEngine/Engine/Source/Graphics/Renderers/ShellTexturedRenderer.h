#pragma once
#include <d3d11.h>

#include "Renderer.h"
#include "Engine/Source/Graphics/ModelData.h"
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/Decals/DecalManager.h"
#include "Engine/Source/Utility/KittyArray.h"
#include "Engine/Source/Graphics/Camera.h"

namespace KE
{
	struct Texture;
	class PixelShader;
	class VertexShader;
	class Graphics;
	class CBuffer;
	struct RenderResources;
	class RenderTarget;

	//

	struct ShellTexturingAttributes
	{
		float totalHeight = 0.5f;
		int shellCount = 32;
		float thickness = 2.0f;
		float density = 512.0f;

		float noiseMin = 0.0f;
		float noiseMax = 1.0f;
		float aoExp = 0.0f;

		float _padding;

		float bottomColour[3] = {53.0f  / 255.0f, 133.0f / 255.0f, 29.0f / 255.0f};
		int shellIndexOffset = 0;
		float topColour[3] =    {164.0f / 255.0f, 228.0f / 255.0f, 48.0f / 255.0f};
		int shellCountPerDrawCall = 32;

		Vector2f textureRegionMin;
		Vector2f textureRegionMax;
	};


	struct ShellTexturingBufferData
	{
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;
		DirectX::XMMATRIX modelToWorld;

		ShellTexturingAttributes attributes;

	
	};

	struct ShellModelData
	{
		ShellTexturingAttributes attributes;
		ModelData modelData;
		RenderTarget* effectsRT = nullptr;
		Texture* displacementTexture = nullptr;
	};

	struct ShellTexturingRenderInput
	{
		DirectX::XMMATRIX viewMatrix;
		DirectX::XMMATRIX projectionMatrix;

		ShellTexturingAttributes attributes;

		VertexShader* overrideVertexShader = nullptr;
		PixelShader* overridePixelShader = nullptr;
	};

	struct ShellTextureDisplacement
	{
		Vector3f position;
		Vector3f scale;
	};

	struct ShellTextureDisplacementBufferData
	{
		int channelIndex = 0;
		float smoothFactor = 1.0f;
		float padding[2];
	};

	class ShellTexturedRenderer : public Renderer
	{
	private:
		CBuffer myRenderingBuffer{};
		CBuffer myInstanceBuffer{};
		CBuffer myDisplacementBuffer{};
		
		KittyArray<ShellModelData> myShellModels;

		ModelData myDisplacementModelData;
		Transform myDisplacementTransform;

	public:
		void Init(Graphics* aGraphics);
		void Render(const ShellTexturingRenderInput& someInput, const ModelData& aModel);

		void ClearDisplacement(LaserPtr<ShellModelData> aModel);
		void FadeDisplacement(LaserPtr<ShellModelData> aModel, float aFadeSpeed, float aMaxFade);

		void RenderDisplacement(
			LaserPtr<ShellModelData> aModel,
			const std::vector<ShellTextureDisplacement>& someDisplacements,
			const Vector3f& boundsMinimum,
			const Vector3f& boundsMaximum,
			Camera* aOriginalCamera = nullptr,
			int aChannelIndex = 0
		);
		void ResetDisplacement(LaserPtr<ShellModelData> aModel);

		LaserPtr<ShellModelData> AddShellModel(const ShellModelData& aShellModelData) { return myShellModels.Append(aShellModelData);  }
		LaserPtr<ShellModelData> GetShellModel(const size_t aIndex) { return myShellModels.At(aIndex); }

		void RemoveShellModel(LaserPtr<ShellModelData> aShellModel) { myShellModels.Erase(aShellModel); }
		void RemoveShellModel(const size_t aIndex) { myShellModels.Erase(aIndex); }

		KittyArray<ShellModelData>& GetShellModels() { return myShellModels; }

		void Reset() { myShellModels.Clear(); }
	};

}
