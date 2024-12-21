#pragma once
#include "FullscreenAsset.h"
#include "RenderTarget.h"

#include "Engine\Source\Graphics\PostProcessAttributes.h"

#include <d3d11.h>

#include <Editor/Source/EditorInterface.h>

namespace KE
{

#define MAX_NUMBER_OF_DOWNSAMPLES 6

	struct BloomBufferData
	{
		float mySampleStage;
		float myTreshold;
		float myBlending;
		float padding;
	};

	struct GuassianBufferData
	{
		float myGuassianDirection;
		float myGuassianQuality;
		float myGuassianSize;
		float myGaussianTreshold;
	};

	struct PostProcessData
	{
		// 4
		Vector2f CARedOffsets;
		Vector2f CAGreenOffsets;

		// 4
		Vector2f CABlueOffsets;
		float CAMultiplier;
		float saturation;

		// 4
		Vector3f colourCorrecting;
		float contrast;

		// 4
		Vector4f tint;

		// 4
		Vector4f blackPoint;

		// 4
		float exposure;
		Vector3f myBloomSampleTreshold;

		// 4
		float vignetteSize;
		float vignetteFeatherThickness;
		float vignetteIntensity;
		int vignetteShowMask;

		//4
		float toneMapIntensity;
		float padding[3];
	};

	class VertexShader;
	class PixelShader;
	class Graphics;
	class RenderTarget;

	class PostProcessing
	{
		KE_EDITOR_FRIEND
	public:
		PostProcessing();
		~PostProcessing();

		void SetPSShader(PixelShader* aPS);
		void SetVSShader(VertexShader* aVS);
		void AssignTexture(Texture* aTexture);
		bool ConfigureDownSampleRTs(Graphics* aGraphics, const int aWidth, const int aHeight);

		void PreProcessBloom(Graphics* aGraphics, RenderTarget* aRenderTarget);
		void MockPreProcessBloom(/*Graphics* aGraphics, */RenderTarget* aRenderTarget);

		void EnableGaussianBlur(const bool aValue);
		void SetAttributes(const KE::PostProcessAttributes& someAttributes);

		inline KE::PostProcessAttributes& GetAttributes() { return myAttributes; };

		bool Init(
			ID3D11Device* aDevice, 
			Graphics* aGraphics, 
			VertexShader* aUDSVertexShader, 
			PixelShader* aDSPS,
			PixelShader* aUSPS,
			PixelShader* aGuassianShader);
		void Render(Graphics* aGraphics, RenderTarget* anActiveRenderTarget = nullptr);
		void BindBuffer(ID3D11DeviceContext& aContext, const int aSlot);
		
		inline PixelShader** GetPreProcessPS() { return &myPreProcessPixelShader; };
	private:
		FullscreenAsset myFullscreenAsset;
		RenderTarget myRenderTarget;
		RenderTarget myBlurTarget;

		void SampleBloom(Graphics* aGraphics, RenderTarget* aFullscreenTexture);
		void BindBloomBuffer(Graphics* aGraphics, const int aStage, const int aSlot = 8);
		void BindGuassianBuffer(Graphics* aGraphics, const int aSlot = 6);

		// Buffer 
		ComPtr<ID3D11Buffer> myBuffer;
		ComPtr<ID3D11Buffer> myBloomBuffer;
		ComPtr<ID3D11Buffer> myGuassianBuffer;

		RenderTarget myPrePostProcessSample;
		std::vector<ComPtr<ID3D11RenderTargetView>> myDSRenderTargets;
		std::vector<ComPtr<ID3D11ShaderResourceView>> myDSShaderResources;
		std::vector<int> mySizeSteps;

		VertexShader* myUDSVertexShader = nullptr;
		PixelShader* myDSPixelShader = nullptr;
		PixelShader* myUSPixelShader = nullptr;
		PixelShader* myGaussianPixelShader = nullptr;
		PixelShader* myPreProcessPixelShader = nullptr;

		//||+-------------+/|\+--------------+||
		//||     Post Process Variables		 +||
		//||+-------------+\|/+--------------+||
		int myNumberOfDownSamples = 5;
		bool myGaussianActive = false;

		PostProcessAttributes myAttributes;
	};
}