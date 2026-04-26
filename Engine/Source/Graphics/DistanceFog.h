#pragma once
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/RenderTarget.h"

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace KE
{
	class FullscreenAsset;
	class PixelShader;
	class VertexShader;

	struct DistFogData
	{
		Vector4f colour =
		{
			140.0f / 255.0f,
			196.0f / 255.0f,
			220.0f / 255.0f,
			0.0f
		};
		float maxDistance = 90.0f;
		float waterheight = -4.9f;
		float fadeDistance = 100.0f;

		int power = 5;

	};

	class DistanceFog
	{
	public:
		DistanceFog() = default;
		~DistanceFog() = default;

		void Init(ID3D11Device* aDevice, ID3D11DeviceContext* aContext, const int aWidth, const int aHeight, PixelShader* aPS, VertexShader* aVS);

		void Resize(const int aWidth, const int aHeight);

		void BindResources(RenderTarget* aRTToCopy, ID3D11DeviceContext* aContext);

		void Render(KE::Graphics* aGraphics, KE::FullscreenAsset& aFullscreenAssetToRender);

		void DrawImGUI();
	private:
		RenderTarget myRenderTarget;

		DistFogData myDistFogData;
		CBuffer myDistFogBuffer;

		PixelShader* myPixelShader = nullptr;
		VertexShader* myVertexShader = nullptr;
	};
}

