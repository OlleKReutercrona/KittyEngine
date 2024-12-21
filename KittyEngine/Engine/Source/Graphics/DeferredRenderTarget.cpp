#include "stdafx.h"
#include "DeferredRenderTarget.h"
#include "Engine/Source/Utility/Logging.h"

void KE::DeferredRenderTarget::Create()
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = myWidth;
	textureDesc.Height = myHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //TODO: make this a parameter
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT; //TODO: make this a parameter
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	HRESULT hr;
	for (int i = 0; i < KE_DEFERRED_RENDER_TARGET_COUNT; i++)
	{
		hr = myDevice->CreateTexture2D(&textureDesc, nullptr, myTextures[i].GetAddressOf());
		if (FAILED(hr))
		{
			KE_ERROR("Failed to create texture for render target, step %i.", i);
			return;
		}

		hr = myDevice->CreateRenderTargetView(myTextures[i].Get(), nullptr, myRenderTargetViews[i].GetAddressOf());
		if (FAILED(hr))
		{
			KE_ERROR("Failed to create render target view for render target, step %i.", i);
			return;
		}

		hr = myDevice->CreateShaderResourceView(myTextures[i].Get(), nullptr, myShaderResourceViews[i].GetAddressOf());
		if (FAILED(hr))
		{
			KE_ERROR("Failed to create shader resource view for render target, step %i.", i);
			return;
		}
	}

	//

	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = myWidth;
	depthStencilDesc.Height = myHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R32_TYPELESS; //TODO: make this a parameter
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT ; //TODO: make this a parameter
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;


	hr = myDevice->CreateTexture2D(&depthStencilDesc, nullptr, myDepthStencilTexture.GetAddressOf());
	if (FAILED(hr))
	{
		KE_ERROR("Failed to create texture for depth stencil.");
		return;
	}

	//creeate desc for the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT; //TODO: make this a parameter
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	
	hr = myDevice->CreateDepthStencilView(myDepthStencilTexture.Get(), &depthStencilViewDesc, myDepthStencilView.GetAddressOf());
	if (FAILED(hr))
	{
		KE_ERROR("Failed to create depth stencil view for depth stencil.");
		return;
	}

	//create desc for the shader resource view
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
	shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT; //TODO: make this a parameter
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;
	
	hr = myDevice->CreateShaderResourceView(myDepthStencilTexture.Get(), &shaderResourceViewDesc, myDepthStencilShaderResourceView.GetAddressOf());
	if (FAILED(hr))
	{
		KE_ERROR("Failed to create shader resource view for depth stencil.");
		return;
	}
}

KE::DeferredRenderTarget::DeferredRenderTarget(ID3D11Device* aDevice, ID3D11DeviceContext* aContext, int aWidth, int aHeight)
{
	myWidth = aWidth;
	myHeight = aHeight;

	myDevice = aDevice;
	myContext = aContext;

	Create();
}

KE::DeferredRenderTarget::~DeferredRenderTarget()
{
}

void KE::DeferredRenderTarget::MakeActive(bool aUseDepth)
{
	//unbind shader resource views
	ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	myContext->PSSetShaderResources(0, 1, nullSRV);

	ID3D11RenderTargetView* renderTargets[KE_DEFERRED_RENDER_TARGET_COUNT] = { 
		myRenderTargetViews[0].Get(), 
		myRenderTargetViews[1].Get(), 
		myRenderTargetViews[2].Get(),
		myRenderTargetViews[3].Get(),
		myRenderTargetViews[4].Get(),

	};

	myContext->OMSetRenderTargets(
		KE_DEFERRED_RENDER_TARGET_COUNT, 
		renderTargets, 
		aUseDepth ? myDepthStencilView.Get() : nullptr
	);
}

void KE::DeferredRenderTarget::Free()
{
	for (int i = 0; i < KE_DEFERRED_RENDER_TARGET_COUNT; i++)
	{
		
	myRenderTargetViews[i].Reset();
	myShaderResourceViews[i].Reset();
	myTextures[i].Reset();
	}
	myDepthStencilView.Reset();
	myDepthStencilTexture.Reset();

	//flush the context
	myContext->Flush();
}

void KE::DeferredRenderTarget::Clear(float* aColour, bool aClearDepth)
{
	if (aColour)
	{
		for (int i = 0; i < KE_DEFERRED_RENDER_TARGET_COUNT; i++)
		{
			myContext->ClearRenderTargetView(myRenderTargetViews[i].Get(), aColour);
		}
	}

	if (aClearDepth)
	{
		myContext->ClearDepthStencilView(myDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}

void KE::DeferredRenderTarget::Resize(int aWidth, int aHeight)
{
	if (aWidth == myWidth && aHeight == myHeight)
	{
		return;
	}
	myWidth = aWidth;
	myHeight = aHeight;

	Free();
	Create();
}