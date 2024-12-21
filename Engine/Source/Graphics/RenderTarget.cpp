#include "stdafx.h"
#include "RenderTarget.h"
#include "Engine/Source/Utility/Logging.h"

#pragma message("What did the cat say to the other cat?")
#pragma message("Copilot: I'm feline fine!")

void KE::RenderTarget::Create()
{
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = myWidth;
	textureDesc.Height = myHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = myFormat; //TODO: make this a parameter
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT; //TODO: make this a parameter
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	HRESULT hr = myDevice->CreateTexture2D(&textureDesc, nullptr, myTexture.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		KE_ERROR("Failed to create texture for render target.");
		return;
	}

	hr = myDevice->CreateRenderTargetView(myTexture.Get(), nullptr, myRenderTargetView.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		KE_ERROR("Failed to create render target view for render target.");
		return;
	}

	hr = myDevice->CreateShaderResourceView(myTexture.Get(), nullptr, myShaderResourceView.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		KE_ERROR("Failed to create shader resource view for render target.");
		return;
	}

	//

	D3D11_TEXTURE2D_DESC depthStencilDesc = {};
	depthStencilDesc.Width = myWidth;
	depthStencilDesc.Height = myHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT; //TODO: make this a parameter
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT; //TODO: make this a parameter
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;


	hr = myDevice->CreateTexture2D(&depthStencilDesc, nullptr, myDepthStencilTexture.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		KE_ERROR("Failed to create texture for depth stencil.");
		return;
	}

	hr = myDevice->CreateDepthStencilView(myDepthStencilTexture.Get(), nullptr, myDepthStencilView.ReleaseAndGetAddressOf());
	if (FAILED(hr))
	{
		KE_ERROR("Failed to create depth stencil view for depth stencil.");
		return;
	}
}

KE::RenderTarget::RenderTarget()
{

}

KE::RenderTarget::~RenderTarget()
{
}

void KE::RenderTarget::Init(ID3D11Device* aDevice, ID3D11DeviceContext* aContext, int aWidth, int aHeight)
{
	myWidth = aWidth;
	myHeight = aHeight;

	myDevice = aDevice;
	myContext = aContext;

	Create();
}

void KE::RenderTarget::MakeActive(bool aUseDepth, ID3D11DepthStencilView* aDSV)
{
	//unbind shader resource views
	//ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	//myContext->PSSetShaderResources(0, 1, nullSRV);
	if (aDSV)
	{
		myContext->OMSetRenderTargets(1, myRenderTargetView.GetAddressOf(), aDSV);
		return;
	}
	myContext->OMSetRenderTargets(1, myRenderTargetView.GetAddressOf(), aUseDepth ? myDepthStencilView.Get() : nullptr);
}

void KE::RenderTarget::MakeActive()
{
	myContext->OMSetRenderTargets(1, myRenderTargetView.GetAddressOf(), nullptr);
}

void KE::RenderTarget::Free()
{
	myRenderTargetView->Release();
	myDepthStencilView->Release();
	myShaderResourceView->Release();
	myTexture->Release();
	myDepthStencilTexture->Release();

	//flush the context
	myContext->Flush();
}

void KE::RenderTarget::Clear(const float* aColour, bool aClearDepth)
{
	if (aColour)
	{
		myContext->ClearRenderTargetView(myRenderTargetView.Get(), aColour);
	}

	if (aClearDepth)
	{
		myContext->ClearDepthStencilView(myDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}
}

void KE::RenderTarget::Resize(int aWidth, int aHeight)
{
	if (aWidth == myWidth && aHeight == myHeight)
	{
		return;
	}
	myWidth = aWidth;
	myHeight = aHeight;
	
	//Free();
	Create();
}

void KE::RenderTarget::SetShouldRenderDebug(bool aShouldRenderDebug)
{
	myShouldRenderDebug = aShouldRenderDebug;
}

void KE::RenderTarget::SetAsShaderResource(const int aSlot)
{
	myContext->PSSetShaderResources(aSlot, 1, myShaderResourceView.GetAddressOf());
}

void KE::RenderTarget::SetAsVertexShaderResource(const int aSlot)
{
	myContext->VSSetShaderResources(aSlot, 1, myShaderResourceView.GetAddressOf());
}


void KE::RenderTarget::CopyFrom(RenderTarget* aRenderTarget, bool aCopyDepth)
{
	myContext->CopyResource(myTexture.Get(), aRenderTarget->myTexture.Get());
	if (aCopyDepth)
	{
		myContext->CopyResource(myDepthStencilTexture.Get(), aRenderTarget->myDepthStencilTexture.Get());
	}
}

void KE::RenderTarget::CopyFrom(ID3D11Texture2D* aTexture)
{
	myContext->CopyResource(myTexture.Get(), aTexture);
}

void KE::RenderTarget::CopyFrom(ID3D11ShaderResourceView* aShaderResourceView)
{
	ID3D11Texture2D* texture = nullptr;
	aShaderResourceView->GetResource(reinterpret_cast<ID3D11Resource**>(&texture));

	myContext->CopyResource(myTexture.Get(), texture);
}