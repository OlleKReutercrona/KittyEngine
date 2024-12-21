#include "stdafx.h"
#include "PostProcessing.h"
#include "Shader.h"
#include "Texture\Texture.h"
#include "Graphics.h"
#include "GraphicsConstants.h"

#include "Editor/Source/ImGui/ImGuiHandler.h"

KE::PostProcessing::PostProcessing() 
{
	myAttributes.CARedOffset = { 0.0f, 0.0f };
	myAttributes.CAGreenOffset = { 0.0f,0.0f };
	myAttributes.CABlueOffset = { 0.0f,0.0f };
	myAttributes.CAMultiplier = 0.0f;
	
	myAttributes.colourCorrecting = { 1.0f, 1.0f, 1.0f };
	
	myAttributes.bloomSampleTreshold = { 1.0f, 1.0f, 1.0f };
	myAttributes.bloomBlending = 0.0f;
	myAttributes.bloomTreshold = 0.25f;

	myAttributes.saturation = 1.5f;
	myAttributes.exposure = 0.0f;
	myAttributes.contrast = 1.0f;
	myAttributes.tint = { 1.0f, 1.0f, 1.0f, 1.0f };;
	myAttributes.blackPoint = { 0.0f, 0.0f, 0.0f, 0.0f };


	myAttributes.gaussianDirection = 25.0f;
	myAttributes.gaussianQuality = 25.0f;
	myAttributes.gaussianSize = 0.0f;
	myAttributes.gaussianTreshold = 0.0f;

	myAttributes.vignetteSize = 0.8f;
	myAttributes.vignetteFeatherThickness = 0.5f;
	myAttributes.vignetteIntensity = 0.0f;
	myAttributes.vignetteShowMask = 0;

	myAttributes.toneMapIntensity = 1.0f;
}

KE::PostProcessing::~PostProcessing() {}

void KE::PostProcessing::SetPSShader(PixelShader* aPS)
{
	myFullscreenAsset.myPixelShader = aPS;
}

void KE::PostProcessing::SetVSShader(VertexShader* aVS)
{
	myFullscreenAsset.myVertexShader = aVS;
}

void KE::PostProcessing::AssignTexture(Texture* aTexture)
{
	myFullscreenAsset.AssignTexture(aTexture);
}

bool KE::PostProcessing::ConfigureDownSampleRTs(Graphics* aGraphics, const int aWidth, const int aHeight)
{
	for (size_t i = 0; i < myDSRenderTargets.size(); i++)
	{
		myDSRenderTargets[i].Reset();
		myDSShaderResources[i].Reset();

		aGraphics->GetContext()->Flush();
	}
	myDSRenderTargets.clear();
	myDSShaderResources.clear();
	mySizeSteps.clear();


	// ||+----------------------------------+||
	// ||	SETUP DOWNSAMPLE RTS AND SRV	+||
	// ||+----------------------------------+||
	{
		//	0	1	2	 3	 4	  5	    6 
		// 1/2 1/4 1/8 1/16 1/32 1/64 1/128

		int step = 2;

		for (int i = 1; i < MAX_NUMBER_OF_DOWNSAMPLES + 1; i++)
		{
			ComPtr<ID3D11Texture2D> dsTexture;

			//////////////////////////////////
			// Texture setup
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = aWidth;
			desc.Height = aHeight;
			desc.Width /= step;
			desc.Height /= step;

			mySizeSteps.push_back(step);
			//mySizeSteps[i - 1] = step;
			step *= 2;


			desc.MipLevels = 0;
			desc.ArraySize = 1;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescDS = {};
			shaderResourceViewDescDS.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDescDS.Texture2D.MostDetailedMip = 0;
			shaderResourceViewDescDS.Texture2D.MipLevels = 1;

			shaderResourceViewDescDS.Format = desc.Format;

			// Create texture // 
			HRESULT hr = aGraphics->GetDevice()->CreateTexture2D(
				&desc,
				nullptr,
				&dsTexture
			);
			if (FAILED(hr))
			{
				return false;
			}

			myDSRenderTargets.emplace_back();

			hr = aGraphics->GetDevice()->CreateRenderTargetView(
				dsTexture.Get(),
				nullptr,
				myDSRenderTargets.back().ReleaseAndGetAddressOf()
			);
			if (FAILED(hr))
			{
				return false;
			}

			myDSShaderResources.emplace_back();

			hr = aGraphics->GetDevice()->CreateShaderResourceView(
				dsTexture.Get(),
				&shaderResourceViewDescDS,
				myDSShaderResources.back().ReleaseAndGetAddressOf()
			);
			if (FAILED(hr))
			{
				return false;
			}
		}


		// MAYBE SHOULD DO THIS WE DONT KNOW ¯\_(ツ)_/¯
		//dsTexture->Release();
	}

	//also resize myRenderTarget and myBlurTarget
	myRenderTarget.Resize(aWidth, aHeight);
	myBlurTarget.Resize(aWidth, aHeight);
	myPrePostProcessSample.Resize(aWidth, aHeight);

	return true;
}

void KE::PostProcessing::PreProcessBloom(Graphics* aGraphics, RenderTarget* aRenderTarget)
{
	BindBuffer(*aGraphics->GetContext().Get(), 1);

	myPrePostProcessSample.MakeActive(false);
	myFullscreenAsset.Render(
		aGraphics,
		aRenderTarget->GetShaderResourceView(),
		myUDSVertexShader,
		myPreProcessPixelShader
	);

	//unset the shader resource view
	ID3D11ShaderResourceView* const nullSRV[1] = { NULL };
	aGraphics->GetContext().Get()->PSSetShaderResources(0, 1, nullSRV);
}

void KE::PostProcessing::MockPreProcessBloom(/*Graphics* aGraphics, */RenderTarget* aRenderTarget)
{
	myPrePostProcessSample.CopyFrom(aRenderTarget);
}

void KE::PostProcessing::SetAttributes(const KE::PostProcessAttributes& someAttributes)
{
	myAttributes = someAttributes;
}

void KE::PostProcessing::EnableGaussianBlur(const bool aValue)
{
	myGaussianActive = aValue;
}

bool KE::PostProcessing::Init(
	ID3D11Device* aDevice,
	Graphics* aGraphics,
	VertexShader* aUDSVertexShader,
	PixelShader* aDSPS,
	PixelShader* aUSPS,
	PixelShader* aGuassianShader)
{
	myFullscreenAsset.Init(aGraphics);

	myRenderTarget.Init(aGraphics->GetDevice().Get(), aGraphics->GetContext().Get(), aGraphics->GetRenderWidth(),
		aGraphics->GetRenderHeight());
	myBlurTarget.Init(aGraphics->GetDevice().Get(), aGraphics->GetContext().Get(), aGraphics->GetRenderWidth(),
		aGraphics->GetRenderHeight());
	myPrePostProcessSample.Init(aGraphics->GetDevice().Get(), aGraphics->GetContext().Get(), aGraphics->GetRenderWidth(),
		aGraphics->GetRenderHeight());

	myUDSVertexShader = aUDSVertexShader;
	myDSPixelShader = aDSPS;
	myUSPixelShader = aUSPS;

	myPreProcessPixelShader = aGraphics->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH "BloomPreProcess_PS.cso");

	myGaussianPixelShader = aGuassianShader;

	//______BUFFER SETUP_____//
	// TODO This should be moved somewhere else
	// Preferably inside a method or as an object
	{
		// PostProcessData

		D3D11_BUFFER_DESC bd;

		ZeroMemory(&bd, sizeof(bd)); /*  Fills a block of memory with zeros.*/

		/*	Identify how the buffer is expected to be read from and written to.
			Frequency of update is a key factor. The most common value is typically D3D11_USAGE_DEFAULT	*/
		bd.Usage = D3D11_USAGE_DYNAMIC;

		/*	If CONSTANT_BUFFER you must set the ByteWidth value in multiples of 16, and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT*/
		bd.ByteWidth = sizeof(PostProcessData);

		/*  Identify how the buffer will be bound to the pipeline */
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT result = aDevice->CreateBuffer(&bd, nullptr, &myBuffer);
		if (FAILED(result))
		{
			return false;
		}
	}
	{
		// BloomData

		D3D11_BUFFER_DESC bd;

		ZeroMemory(&bd, sizeof(bd)); /*  Fills a block of memory with zeros.*/

		/*	Identify how the buffer is expected to be read from and written to.
			Frequency of update is a key factor. The most common value is typically D3D11_USAGE_DEFAULT	*/
		bd.Usage = D3D11_USAGE_DYNAMIC;

		/*	If CONSTANT_BUFFER you must set the ByteWidth value in multiples of 16, and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT*/
		bd.ByteWidth = sizeof(BloomBufferData);

		/*  Identify how the buffer will be bound to the pipeline */
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT result = aDevice->CreateBuffer(&bd, nullptr, &myBloomBuffer);
		if (FAILED(result))
		{
			return false;
		}
	}

	{
		// Gaussian Blur

		D3D11_BUFFER_DESC bd;

		ZeroMemory(&bd, sizeof(bd)); /*  Fills a block of memory with zeros.*/

		/*	Identify how the buffer is expected to be read from and written to.
			Frequency of update is a key factor. The most common value is typically D3D11_USAGE_DEFAULT	*/
		bd.Usage = D3D11_USAGE_DYNAMIC;

		/*	If CONSTANT_BUFFER you must set the ByteWidth value in multiples of 16, and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT*/
		bd.ByteWidth = sizeof(GuassianBufferData);

		/*  Identify how the buffer will be bound to the pipeline */
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT result = aDevice->CreateBuffer(&bd, nullptr, &myGuassianBuffer);
		if (FAILED(result))
		{
			return false;
		}
	}



	return true;
}

void KE::PostProcessing::Render(Graphics* aGraphics, RenderTarget* anActiveRenderTarget)
{
	// Reset
	constexpr float colour[] = { 0.0f,0.0f,0.0f,0.0f };

	ID3D11ShaderResourceView* const nullSRV[1] = { NULL };
	aGraphics->GetContext().Get()->PSSetShaderResources(5, 1, nullSRV);
	// End Reset

	//PreProcessBloom(aGraphics, anActiveRenderTarget);


	if (myAttributes.bloomBlending > 0.0f)
	{
		SampleBloom(aGraphics, &myPrePostProcessSample);
	}


	myPrePostProcessSample.Clear((float*)colour);
	myRenderTarget.Clear((float*)colour);
	myRenderTarget.MakeActive(false);

	if (anActiveRenderTarget != nullptr)
	{
		// Post Process Render call
		myFullscreenAsset.Render(aGraphics, anActiveRenderTarget->GetShaderResourceView());

		if (myGaussianActive)
		{
			BindGuassianBuffer(aGraphics);

			// Set the active cameras RT to be active instead of the local RT
			myBlurTarget.MakeActive(false);

			// Render the blur
			myFullscreenAsset.Render(aGraphics, myRenderTarget.GetShaderResourceView(), myUDSVertexShader,
				myGaussianPixelShader);

			myRenderTarget.CopyFrom(&myBlurTarget);
		}

		anActiveRenderTarget->CopyFrom(&myRenderTarget);

		aGraphics->GetContext().Get()->PSSetShaderResources(0, 1, nullSRV);
	}
	else
	{
		myFullscreenAsset.Render(aGraphics);
	}
}

void KE::PostProcessing::BindBuffer(ID3D11DeviceContext& aContext, const int aSlot)
{
	PostProcessData postProcessData;
	{
		postProcessData.CARedOffsets.x = myAttributes.CARedOffset.x;
		postProcessData.CARedOffsets.y = myAttributes.CARedOffset.y;

		postProcessData.CAGreenOffsets.x = myAttributes.CAGreenOffset.x;
		postProcessData.CAGreenOffsets.y = myAttributes.CAGreenOffset.y;

		postProcessData.CABlueOffsets.x = myAttributes.CABlueOffset.x;
		postProcessData.CABlueOffsets.y = myAttributes.CABlueOffset.y;

		postProcessData.CAMultiplier = myAttributes.CAMultiplier;


		postProcessData.colourCorrecting.x = myAttributes.colourCorrecting.x;
		postProcessData.colourCorrecting.y = myAttributes.colourCorrecting.y;
		postProcessData.colourCorrecting.z = myAttributes.colourCorrecting.z;

		postProcessData.saturation = myAttributes.saturation;
		postProcessData.exposure = myAttributes.exposure;
		postProcessData.contrast = myAttributes.contrast;
		postProcessData.tint = myAttributes.tint;
		postProcessData.blackPoint = myAttributes.blackPoint;

		postProcessData.myBloomSampleTreshold = myAttributes.bloomSampleTreshold;

		postProcessData.vignetteSize = myAttributes.vignetteSize;
		postProcessData.vignetteFeatherThickness = myAttributes.vignetteFeatherThickness;
		postProcessData.vignetteIntensity = myAttributes.vignetteIntensity;
		postProcessData.vignetteShowMask = myAttributes.vignetteShowMask;

		postProcessData.toneMapIntensity = myAttributes.toneMapIntensity;
	}

	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	aContext.Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	memcpy(mappedBuffer.pData, &postProcessData, sizeof(PostProcessData));
	aContext.Unmap(myBuffer.Get(), 0);

	aContext.VSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
	aContext.PSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
}

void KE::PostProcessing::SampleBloom(Graphics* aGraphics, RenderTarget* aFullscreenTexture)
{
	auto context = aGraphics->GetContext();
	ID3D11ShaderResourceView* const nullSRV[1] = { NULL };

	/////////////////////////////////////////////
	// DownSample
	{
		/// Downsample first time with the given fullscreenTexture
		context->OMSetRenderTargets(1, myDSRenderTargets[0].GetAddressOf(), nullptr);
		BindBloomBuffer(aGraphics, mySizeSteps[0]);
		myFullscreenAsset.Render(aGraphics, aFullscreenTexture->GetShaderResourceView(), myUDSVertexShader,
			myDSPixelShader);

		// Loop through the rest of the render targets and downsample
		for (int i = 1; i < myNumberOfDownSamples; i++)
		{
			context->OMSetRenderTargets(1, myDSRenderTargets[i].GetAddressOf(), nullptr);
			BindBloomBuffer(aGraphics, mySizeSteps[i]);
			myFullscreenAsset.Render(aGraphics, myDSShaderResources[i - 1].Get(), myUDSVertexShader, myDSPixelShader);
		}
	}

	// Reset slot 0
	context->PSSetShaderResources(0, 1, nullSRV);

	// Set blend state to alphablend
	aGraphics->SetBlendState(eBlendStates::AlphaBlend);

	////////////////////////////////////////////
	// UpSample
	{
		for (int i = myNumberOfDownSamples - 1; i > 0; i--)
		{
			context->OMSetRenderTargets(1, myDSRenderTargets[i - 1].GetAddressOf(), nullptr);
			BindBloomBuffer(aGraphics, mySizeSteps[i - 1]);
			myFullscreenAsset.Render(aGraphics, myDSShaderResources[i].Get(), myUDSVertexShader, myUSPixelShader);
		}
	}

	// Disable alphablend
	aGraphics->SetBlendState(eBlendStates::Disabled);

	// Reset RenderTarget
	context->OMSetRenderTargets(0, nullptr, nullptr);

	// Set result of bloom down->Upsampling to slot 5 for PS to use
	context->PSSetShaderResources(5, 1, myDSShaderResources[0].GetAddressOf());
}

void KE::PostProcessing::BindBloomBuffer(Graphics* aGraphics, const int aStage, const int aSlot)
{
	BloomBufferData data;

	data.mySampleStage = static_cast<float>(aStage);
	data.myTreshold = myAttributes.bloomTreshold;
	data.myBlending = myAttributes.bloomBlending;

	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	aGraphics->GetContext()->Map(myBloomBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	memcpy(mappedBuffer.pData, &data, sizeof(BloomBufferData));
	aGraphics->GetContext()->Unmap(myBloomBuffer.Get(), 0);

	aGraphics->GetContext()->PSSetConstantBuffers(aSlot, 1, myBloomBuffer.GetAddressOf());
}

void KE::PostProcessing::BindGuassianBuffer(Graphics* aGraphics, const int aSlot)
{
	GuassianBufferData guassianData;

	guassianData.myGuassianDirection = myAttributes.gaussianDirection;
	guassianData.myGuassianQuality = myAttributes.gaussianQuality;
	guassianData.myGuassianSize = myAttributes.gaussianSize;
	guassianData.myGaussianTreshold = myAttributes.gaussianTreshold;

	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	aGraphics->GetContext()->Map(myGuassianBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	memcpy(mappedBuffer.pData, &guassianData, sizeof(BloomBufferData));
	aGraphics->GetContext()->Unmap(myGuassianBuffer.Get(), 0);

	aGraphics->GetContext()->PSSetConstantBuffers(aSlot, 1, myGuassianBuffer.GetAddressOf());
}
