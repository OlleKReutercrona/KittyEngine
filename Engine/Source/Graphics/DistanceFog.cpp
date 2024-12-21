#include "stdafx.h"
#include "DistanceFog.h"
#include "Engine/Source/Graphics/FullscreenAsset.h"
#include "Engine/Source/Graphics/Shader.h"
#include "Engine/Source/Graphics/Graphics.h"

#include "External\Include\imgui\imgui.h"

void KE::DistanceFog::Init(ID3D11Device* aDevice, ID3D11DeviceContext* aContext, const int aWidth, const int aHeight, PixelShader* aPS, VertexShader* aVS)
{
	myRenderTarget.Init(aDevice, aContext, aWidth, aHeight);

	myPixelShader = aPS;
	myVertexShader = aVS;

	// Buffer setup
	{
		D3D11_BUFFER_DESC bd;

		ZeroMemory(&bd, sizeof(bd)); /*  Fills a block of memory with zeros.*/

		/*	Identify how the buffer is expected to be read from and written to.
			Frequency of update is a key factor. The most common value is typically D3D11_USAGE_DEFAULT	*/
		bd.Usage = D3D11_USAGE_DYNAMIC;

		/*	If CONSTANT_BUFFER you must set the ByteWidth value in multiples of 16, and less than or equal to D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT*/
		bd.ByteWidth = sizeof(DistFogData);

		/*  Identify how the buffer will be bound to the pipeline */
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = myDistFogBuffer.Init(aDevice, &bd);
		if (FAILED(hr))
		{
			return;
		}
	}

	ADD_LAMBDA_WINDOW("Distance Fog", std::bind(&KE::DistanceFog::DrawImGUI, this));
}

void KE::DistanceFog::Resize(const int aWidth, const int aHeight)
{
	myRenderTarget.Resize(aWidth, aHeight);
}

void KE::DistanceFog::BindResources(RenderTarget* aRTToCopy, ID3D11DeviceContext* aContext)
{
	myRenderTarget.CopyFrom(aRTToCopy);

	myRenderTarget.SetAsShaderResource(11);

	myDistFogBuffer.MapBuffer(&myDistFogData, sizeof(DistFogData), aContext);
	myDistFogBuffer.BindForPS(1, aContext);
}

void KE::DistanceFog::Render(KE::Graphics* aGraphics, KE::FullscreenAsset& aFullscreenAssetToRender)
{
	//DrawImGUI();

	aFullscreenAssetToRender.RenderWithoutSRV(aGraphics, myVertexShader, myPixelShader);

	ID3D11ShaderResourceView* const nullSRV[1] = { NULL };
	aGraphics->GetContext()->PSSetShaderResources(1, 1, nullSRV);
	aGraphics->GetContext()->PSSetShaderResources(11, 1, nullSRV);
}

void KE::DistanceFog::DrawImGUI()
{
	//ImGui::Begin("Distance Fog");
	ImGui::DragFloat("Max Distance", &myDistFogData.maxDistance, 0.1f, 0.0f, 100.0f);
	ImGui::DragFloat("Water Height", &myDistFogData.waterheight, 0.1f, -25.0f, 25.0f);
	ImGui::DragFloat("Fog Distance", &myDistFogData.fadeDistance, 0.1f, 0.0f, 100.0f);
	ImGui::SliderInt("Power (1 = linear)", &myDistFogData.power, 1, 6);
	ImGui::ColorPicker4("Fog Colour", &myDistFogData.colour.x);
	//ImGui::End();
}
