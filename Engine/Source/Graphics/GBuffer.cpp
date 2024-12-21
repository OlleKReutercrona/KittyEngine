#include "stdafx.h"
#include "GBuffer.h"
#include "DirectXMath.h"
#include "GraphicsConstants.h"

namespace KE
{
	GBuffer::GBuffer() {}
	GBuffer::~GBuffer() {}

	GBuffer GBuffer::Create(const DirectX::XMINT2 aSize, ID3D11Device* aDevice)
	{
		GBuffer returnGBuffer;
		returnGBuffer.CreateRTandDSV(aDevice, aSize);

		return returnGBuffer;

#pragma region OldCreation
		//HRESULT hr;
		//constexpr std::array textureFormats =
		//{
		//	DXGI_FORMAT_R32G32B32A32_FLOAT, // Position
		//	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, // Albedo
		//	DXGI_FORMAT_R10G10B10A2_UNORM, // Normal,
		//	DXGI_FORMAT_R8G8B8A8_UNORM, // Material
		//	DXGI_FORMAT_R8G8B8A8_UNORM, // AmbientOcclusionAndCustom
		//	DXGI_FORMAT_R8G8B8A8_UNORM, // Depth
		//};

		//GBuffer returnGBuffer;

		//D3D11_TEXTURE2D_DESC desc = {0};
		//desc.Width = aSize.x;
		//desc.Height = aSize.y;
		//desc.MipLevels = 1;
		//desc.ArraySize = 1;
		//desc.SampleDesc.Count = 1;
		//desc.SampleDesc.Quality = 0;
		//desc.Usage = D3D11_USAGE_DEFAULT;
		//desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		//desc.CPUAccessFlags = 0;
		//desc.MiscFlags = 0;

		//for (unsigned int idx = 0; idx < static_cast<int>(GBufferTexture::Count); idx++)
		//{
		//	desc.Format = textureFormats[idx];
		//	hr = aDevice->CreateTexture2D(&desc, nullptr, &returnGBuffer.myTextures[idx]);
		//	assert(SUCCEEDED(hr));

		//	hr = aDevice->CreateRenderTargetView(
		//		returnGBuffer.myTextures[idx].Get(),
		//		nullptr,
		//		returnGBuffer.myRenderTargetViews[idx].GetAddressOf());
		//	assert(SUCCEEDED(hr));

		//	hr = aDevice->CreateShaderResourceView(
		//		returnGBuffer.myTextures[idx].Get(),
		//		nullptr,
		//		returnGBuffer.myShaderResourceViews[idx].GetAddressOf());
		//	assert(SUCCEEDED(hr));
		//}


		//// Depth Stencil
		//{
		//	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
		//	depthStencilDesc.Width = aSize.x;
		//	depthStencilDesc.Height = aSize.y;
		//	depthStencilDesc.MipLevels = 1;
		//	depthStencilDesc.ArraySize = 1;
		//	depthStencilDesc.Format = DXGI_FORMAT_R32_TYPELESS; //TODO: make this a parameter
		//	depthStencilDesc.SampleDesc.Count = 1;
		//	depthStencilDesc.SampleDesc.Quality = 0;
		//	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT; //TODO: make this a parameter
		//	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		//	depthStencilDesc.CPUAccessFlags = 0;
		//	depthStencilDesc.MiscFlags = 0;


		//	hr = aDevice->CreateTexture2D(&depthStencilDesc, nullptr,
		//	                              returnGBuffer.myDepthStencilTexture.GetAddressOf());
		//	assert(SUCCEEDED(hr));

		//	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		//	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		//	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		//	depthStencilViewDesc.Texture2D.MipSlice = 0;

		//	hr = aDevice->CreateDepthStencilView(
		//		returnGBuffer.myDepthStencilTexture.Get(),
		//		&depthStencilViewDesc,
		//		returnGBuffer.myDepthStencilView.GetAddressOf());
		//	assert(SUCCEEDED(hr));

		//	//create desc for the shader resource view
		//	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		//	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT; //TODO: make this a parameter
		//	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		//	shaderResourceViewDesc.Texture2D.MipLevels = 1;

		//	hr = aDevice->CreateShaderResourceView(returnGBuffer.myDepthStencilTexture.Get(), &shaderResourceViewDesc,
		//	                                       returnGBuffer.myDepthStencilShaderResourceView.GetAddressOf());
		//	assert(SUCCEEDED(hr));
		//}


		//returnGBuffer.myViewport = D3D11_VIEWPORT(
		//	D3D11_VIEWPORT{
		//		0,
		//		0,
		//		static_cast<float>(desc.Width),
		//		static_cast<float>(desc.Height),
		//		0,
		//		1
		//	});

		//return returnGBuffer;
#pragma endregion
	}

	void GBuffer::ClearTextures(ID3D11DeviceContext* aContext, bool aClearDepth) const
	{
		constexpr float colour[] = { 0.0f,0.0f,0.0f,0.0f };

		for (unsigned int idx = 0; idx <
		     static_cast<int>(GBufferTexture::Count); idx++)
		{
			aContext->ClearRenderTargetView(myRenderTargetViews[idx].Get(),
			                                &colour[0]);
		}

		if(aClearDepth)
		{
			aContext->ClearDepthStencilView(myDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		}
	}

	void GBuffer::SetAsActiveTarget(ID3D11DeviceContext* aContext, ID3D11DepthStencilView* aDepth)
	{
		if (aDepth)
		{
			aContext->OMSetRenderTargets(static_cast<int>(GBufferTexture::Count), myRenderTargetViews[0].GetAddressOf(),
				aDepth);
		}
		else
		{
			aContext->OMSetRenderTargets(static_cast<int>(GBufferTexture::Count), myRenderTargetViews[0].GetAddressOf(),
			                             nullptr);
		}
		aContext->RSSetViewports(1, &myViewport);
	}

	void GBuffer::SetAsResourceOnSlot(ID3D11DeviceContext* aContext, GBufferTexture aTexture, const unsigned int aSlot)
	{
		aContext->PSSetShaderResources(aSlot, 1, myShaderResourceViews[static_cast<int>(aTexture)].GetAddressOf());
	}

	void GBuffer::SetAllAsResources(ID3D11DeviceContext* aContext, const unsigned int aSlot)
	{
		aContext->PSSetShaderResources(aSlot, static_cast<int>(GBufferTexture::Count),
		                               myShaderResourceViews[0].GetAddressOf());

		int aDepthSlot = aSlot + static_cast<int>(GBufferTexture::Count);

		aContext->PSSetShaderResources(aDepthSlot, 1, myDepthStencilShaderResourceView.GetAddressOf());
	}

	void GBuffer::ClearAllAsResourcesSlots(ID3D11DeviceContext* aContext, unsigned int aSlot)
	{
		ID3D11ShaderResourceView* const nullSRV[static_cast<int>(GBufferTexture::Count) + 1] = { NULL };

		aContext->PSSetShaderResources(aSlot, static_cast<int>(GBufferTexture::Count) + 1,
			nullSRV);
	}

	void GBuffer::ResizeViewport(ID3D11Device* aDevice, const DirectX::XMINT2 aSize)
	{
		myViewport.Width = static_cast<float>(aSize.x);
		myViewport.Height = static_cast<float>(aSize.y);

		ReleaseResources();
		CreateRTandDSV(aDevice, aSize);
	}

	void GBuffer::SetDepthAsResourceOnSlot(ID3D11DeviceContext* aContext, unsigned aSlot)
	{
		aContext->PSSetShaderResources(aSlot, 1, myDepthStencilShaderResourceView.GetAddressOf());
	}

	void GBuffer::UnbindTarget(ID3D11DeviceContext* aContext)
	{
		ID3D11RenderTargetView* const nullRTV[static_cast<int>(GBufferTexture::Count) + 1] = { NULL };
		aContext->OMSetRenderTargets(static_cast<int>(GBufferTexture::Count) + 1, nullRTV, nullptr);
	}

	ID3D11Texture2D* GBuffer::GetTexture(const unsigned int aIndex)
	{
		assert(aIndex < (int)GBufferTexture::Count && "Trying to get a Texture2D from Gbuffer that doesnt exist.");

		return myTextures[aIndex].Get();
	}

	void GBuffer::CopyFrom(ID3D11DeviceContext* aContext, GBuffer* aBuffer)
	{
		for (int i = 0; i < (int)GBufferTexture::Count; i++)
		{
			aContext->CopyResource(myTextures[i].Get(), aBuffer->myTextures[i].Get());
		}
		aContext->CopyResource(myDepthStencilTexture.Get(), aBuffer->myDepthStencilTexture.Get());
	}

	void GBuffer::Resize(ID3D11Device* aDevice, const DirectX::XMINT2& aSize)
	{
		ReleaseResources();
		CreateRTandDSV(aDevice, aSize);
	}

	void GBuffer::CreateRTandDSV(ID3D11Device* aDevice, const DirectX::XMINT2 aSize)
	{
		HRESULT hr;
		constexpr std::array textureFormats =
		{
			DXGI_FORMAT_R32G32B32A32_FLOAT, // Position
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, // Albedo
			DXGI_FORMAT_R10G10B10A2_UNORM, // Normal,
			DXGI_FORMAT_R8G8B8A8_UNORM, // Material
			DXGI_FORMAT_R8G8B8A8_UNORM, // AmbientOcclusionAndCustom
			DXGI_FORMAT_R8G8B8A8_UNORM, // Depth
		};

		D3D11_TEXTURE2D_DESC desc = { 0 };
		desc.Width = aSize.x;
		desc.Height = aSize.y;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		for (unsigned int idx = 0; idx < static_cast<int>(GBufferTexture::Count); idx++)
		{
			desc.Format = textureFormats[idx];
			hr = aDevice->CreateTexture2D(&desc, nullptr, myTextures[idx].ReleaseAndGetAddressOf());
			assert(SUCCEEDED(hr));

			hr = aDevice->CreateRenderTargetView(
				myTextures[idx].Get(),
				nullptr,
				myRenderTargetViews[idx].ReleaseAndGetAddressOf());
			assert(SUCCEEDED(hr));

			hr = aDevice->CreateShaderResourceView(
				myTextures[idx].Get(),
				nullptr,
				myShaderResourceViews[idx].ReleaseAndGetAddressOf());
			assert(SUCCEEDED(hr));
		}


		D3D11_TEXTURE2D_DESC depthStencilDesc = {};
		depthStencilDesc.Width = aSize.x;
		depthStencilDesc.Height = aSize.y;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_R32_TYPELESS; //TODO: make this a parameter
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT; //TODO: make this a parameter
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;



		hr = aDevice->CreateTexture2D(&depthStencilDesc, nullptr,
			myDepthStencilTexture.ReleaseAndGetAddressOf());
		assert(SUCCEEDED(hr));

		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		hr = aDevice->CreateDepthStencilView(
			myDepthStencilTexture.Get(),
			&depthStencilViewDesc,
			myDepthStencilView.ReleaseAndGetAddressOf());
		assert(SUCCEEDED(hr));

		//create desc for the shader resource view
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT; //TODO: make this a parameter
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;

		hr = aDevice->CreateShaderResourceView(myDepthStencilTexture.Get(), &shaderResourceViewDesc,
			myDepthStencilShaderResourceView.ReleaseAndGetAddressOf());
		assert(SUCCEEDED(hr));

		myViewport = D3D11_VIEWPORT(
			D3D11_VIEWPORT{
				0,
				0,
				static_cast<float>(desc.Width),
				static_cast<float>(desc.Height),
				0,
				1
			});
	}

	void GBuffer::ReleaseResources()
	{
		//for (unsigned int idx = 0; idx < static_cast<int>(GBufferTexture::Count); idx++)
		//{
		//	myTextures[idx]->Release();
		//	myRenderTargetViews[idx]->Release();
		//	myShaderResourceViews[idx]->Release();
		//}

		//myDepthStencilTexture->Release();
		//myDepthStencilView->Release();
		//myDepthStencilShaderResourceView->Release();
	}
}
