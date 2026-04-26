#include "stdafx.h"
#include "SSAO.h"
#include "Engine/Source/Graphics/Graphics.h"
#include "External/Include/imgui/imgui.h"

KE::SSAO::SSAO() : myRandomFloats(0.0f, 1.0f)
{
}

HRESULT KE::SSAO::Init(Graphics* aGraphics)
{
	HRESULT result = S_OK;
	if (!myFullscreenAsset.Init(aGraphics))
	{
		result = E_FAIL;
		return result;
	}

	int height = aGraphics->GetRenderHeight();
	int width = aGraphics->GetRenderWidth();


	myFullscreenAsset.myPixelShader = aGraphics->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH"SSAO_PS.cso");

	myFullscreenAsset.myVertexShader = aGraphics->GetShaderLoader().GetVertexShader(SHADER_LOAD_PATH"SSAO_VS.cso");

	myBlurShader = aGraphics->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH"BlurFullscreen_PS.cso");

	myBlurRenderTarget.Init(aGraphics->GetDevice().Get(), aGraphics->GetContext().Get(), width , height);
	mySSAORenderTarget.Init(aGraphics->GetDevice().Get(), aGraphics->GetContext().Get(), width , height);

	// Buffer Setup
	{


		D3D11_BUFFER_DESC bd;

		ZeroMemory(&bd, sizeof(bd)); /*  Fills a block of memory with zeros.*/

		/*	Identify how the buffer is expected to be read from and written to.
			Frequency of update is a key factor. The most common value is typically D3D11_USAGE_DEFAULT	*/
		bd.Usage = D3D11_USAGE_DYNAMIC;

		/*	If CONSTANT_BUFFER you must set the ByteWidth value in multiples of 16, and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT*/
		bd.ByteWidth = sizeof(SSAOData);

		/*  Identify how the buffer will be bound to the pipeline */
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		result = myBuffer.Init(aGraphics->GetDevice(), &bd);
		if (FAILED(result))
		{
			return result;
		}
	}

	// Noise Texture
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = NUM_SSAO_NOISE / 4;
		desc.Height = NUM_SSAO_NOISE / 4;
		desc.MipLevels = desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;

		result = aGraphics->GetDevice()->CreateTexture2D(&desc, NULL, myNoiseTexture.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			return result;
		}

		result = aGraphics->GetDevice()->CreateShaderResourceView(myNoiseTexture.Get(), NULL, myNoiseSRV.ReleaseAndGetAddressOf());
		if (FAILED(result))
		{
			return result;
		}
	}

	GatherSamples();
	GenerateNoise(aGraphics);

	myBufferData.numOfSamples = NUM_SSAO_SAMPLES / 2;

	ADD_LAMBDA_WINDOW("SSAO Settings", std::bind(&KE::SSAO::DebugDisplayImgui, this));

	return result;
}

void KE::SSAO::Render(Graphics* aGraphics)
{
	//DebugDisplayImgui();

	if (!shouldRenderSSAO) return;

	BindNoise(aGraphics, 10);

	myBuffer.MapBuffer(&myBufferData, sizeof(myBufferData), aGraphics->GetContext().Get());
	myBuffer.BindForPS(5, aGraphics->GetContext().Get());

	constexpr float colour[] = { 0.0f,0.0f,0.0f,0.0f };
	mySSAORenderTarget.Clear((float*)colour);
	mySSAORenderTarget.MakeActive();

	myFullscreenAsset.RenderWithoutSRV(aGraphics);

	aGraphics->GetContext().Get()->OMSetRenderTargets(0, NULL, NULL);

	Bind(aGraphics->GetContext().Get());

	myBlurRenderTarget.Clear((float*)colour);
	myBlurRenderTarget.MakeActive();

	myFullscreenAsset.RenderWithoutSRV(aGraphics, nullptr, myBlurShader);

	aGraphics->GetContext().Get()->OMSetRenderTargets(0, NULL, NULL);

	myBlurRenderTarget.SetAsShaderResource(7);
}

void KE::SSAO::Bind(ID3D11DeviceContext* aContext)
{
	if (!shouldRenderSSAO) return;

	mySSAORenderTarget.SetAsShaderResource(7);
}

void KE::SSAO::Resize(Graphics* aGraphics, const DirectX::XMINT2 aSize)
{
	//myBlurRenderTarget.Init(aGraphics->GetDevice().Get(), aGraphics->GetContext().Get(), aSize.x, aSize.y);
	//mySSAORenderTarget.Init(aGraphics->GetDevice().Get(), aGraphics->GetContext().Get(), aSize.x, aSize.y);

	myBlurRenderTarget.Resize(aSize.x, aSize.y);
	mySSAORenderTarget.Resize(aSize.x, aSize.y);
}

void KE::SSAO::Unbind(ID3D11DeviceContext* aContext)
{
	if (!shouldRenderSSAO) return;

	ID3D11ShaderResourceView* const nullSRV[1] = { NULL };
	aContext->PSSetShaderResources(7, 1, nullSRV);
}

void KE::SSAO::GatherSamples()
{
	for (int i = 0; i < NUM_SSAO_SAMPLES; i++)
	{
		// Generate Random Position in the hemisphere
		myBufferData.samples[i] =
		{
			myRandomFloats(myGenerator) * 2.0f - 1.0f,
			myRandomFloats(myGenerator) * 2.0f - 1.0f,
			myRandomFloats(myGenerator),
			1.0f
		};

		myBufferData.samples[i].Normalize();
		myBufferData.samples[i] *= myRandomFloats(myGenerator);

		// Scale the position closer to the point of origin
		float scale = (float)i / 64.0f;
		scale = lerp(0.1f, 1.0f, scale * scale);
		myBufferData.samples[i] *= scale;
	}
}

void KE::SSAO::GenerateNoise(Graphics* aGraphics)
{
	myNoise.resize(NUM_SSAO_NOISE);
	for (int i = 0; i < NUM_SSAO_NOISE; i++)
	{
		myNoise[i] = {
			myRandomFloats(myGenerator) * 2.0f - 1.0f,
			myRandomFloats(myGenerator) * 2.0f - 1.0f,
			0.0f,
			1.0f
		};

	}
}

void KE::SSAO::BindNoise(Graphics* aGraphics, const int aSlot)
{
	// Map noise data to texture
	Vector2i nRes(4, 4);

	aGraphics->GetContext()->UpdateSubresource(myNoiseTexture.Get(), 0, NULL, (void*)myNoise.data(), nRes.x * 4, nRes.y * 4);

	aGraphics->GetContext()->PSSetShaderResources(SSAO_NOISE_TEXTURE_SLOT, 1, myNoiseSRV.GetAddressOf());
}

void KE::SSAO::DebugDisplayImgui()
{
	//ImGui::Begin("SSAO Settings");
	ImGui::Checkbox("SSAO Active", &shouldRenderSSAO);
	ImGui::DragFloat("Radius##SSAORadius", &myBufferData.radius, 0.01f, 0.1f, 15.0f);
	ImGui::DragInt("Number Of Samples##SSAOSamples", &myBufferData.numOfSamples, 1 ,1, NUM_SSAO_SAMPLES);

	//ImGui::End();
}
