#pragma once
#include "FullscreenAsset.h"
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/RenderTarget.h"
#include <random>

// DirectX 11
struct ID3D11Device;
struct ID3D11DeviceContext;

namespace KE
{

#define NUM_SSAO_SAMPLES 32
#define NUM_SSAO_NOISE 16
#define SSAO_NOISE_TEXTURE_SLOT 10

	struct SSAOData
	{
		Vector4f samples[NUM_SSAO_SAMPLES];
		float radius = 1.5f;
		int numOfSamples = NUM_SSAO_SAMPLES;
		Vector2f PADDING;
	};

	class RenderTarget;
	class Graphics;

	class SSAO
	{
	public:
		SSAO();
		~SSAO() = default;

		HRESULT Init(Graphics* aGraphics);

		void Render(Graphics* aGraphics);

		inline  ID3D11ShaderResourceView* GetSSAOSRV() { return mySSAORenderTarget.GetShaderResourceView(); };

		inline  ID3D11ShaderResourceView* GetBlurSRV() { return myBlurRenderTarget.GetShaderResourceView(); };

		inline  ID3D11ShaderResourceView* GetNoiseRSV() { return myNoiseSRV.Get(); };

		void Resize(Graphics* aGraphics, const DirectX::XMINT2 aSize);

		void Unbind(ID3D11DeviceContext* aContext);
	private:
		void Bind(ID3D11DeviceContext* aContext);
		void GatherSamples();
		void GenerateNoise(Graphics* aGraphics);
		void BindNoise(Graphics* aGraphics,const int aSlot);

		void DebugDisplayImgui();

		FullscreenAsset myFullscreenAsset;

		CBuffer myBuffer;
		ComPtr<ID3D11ShaderResourceView> myNoiseSRV;
		ComPtr<ID3D11Texture2D> myNoiseTexture;
		RenderTarget mySSAORenderTarget;

		RenderTarget myBlurRenderTarget;
		PixelShader* myBlurShader;

		SSAOData myBufferData;
		//Texture myTexture;

		std::vector<Vector4f> myNoise;

		std::uniform_real_distribution<float> myRandomFloats;
		std::default_random_engine myGenerator;

		bool shouldRenderSSAO = true;
	};
}

