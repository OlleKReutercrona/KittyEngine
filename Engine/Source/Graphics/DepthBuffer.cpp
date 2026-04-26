#include "stdafx.h"
#include "DepthBuffer.h"
#include "RenderTarget.h"

void KE::DepthBuffer::Create(ID3D11Device* aDevice, const Vector2i aSize)
{
	HRESULT result;
	D3D11_TEXTURE2D_DESC desc = { 0 };
	desc.Width = aSize.x;
	desc.Height = aSize.y;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32_TYPELESS;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	ID3D11Texture2D* texture;
	result = aDevice->CreateTexture2D(&desc, nullptr, &texture);
	assert(SUCCEEDED(result));

	ID3D11DepthStencilView* DSV;
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};

	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	result = aDevice->CreateDepthStencilView(texture, &dsvDesc, &DSV);
	assert(SUCCEEDED(result));

	myDepth = DSV;
	DSV->Release();

	ID3D11ShaderResourceView* SRV;
	D3D11_SHADER_RESOURCE_VIEW_DESC srDesc{};
	srDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srDesc.Texture2D.MostDetailedMip = 0;
	srDesc.Texture2D.MipLevels = UINT_MAX;
	result = aDevice->CreateShaderResourceView(texture, &srDesc, &SRV);
	assert(SUCCEEDED(result));
	mySRV = SRV;
	SRV->Release();

	myViewport = {
			0,
			0,
			static_cast<float>(aSize.x),
			static_cast<float>(aSize.y),
			0,
			1
	};
}

KE::DepthBuffer::DepthBuffer()
{
}

void KE::DepthBuffer::Init(ID3D11Device* aDevice, const Vector2i aSize)
{
	Create(aDevice, aSize);
}

void KE::DepthBuffer::Resize(ID3D11Device* aDevice, const Vector2i aSize)
{
	myDepth.Reset();
	mySRV.Reset();

	Create(aDevice, aSize);
}

void KE::DepthBuffer::Clear(ID3D11DeviceContext* aContext, float aClearDepthValue, uint8_t aClearStencilValue)
{
	aContext->ClearDepthStencilView(myDepth.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, aClearDepthValue, aClearStencilValue);
}

void KE::DepthBuffer::SetAsActiveTarget(ID3D11DeviceContext* aContext, KE::RenderTarget* aRT)
{
	if (aRT)
	{
		aRT->MakeActive(true, GetDepthStencilView());
	}
	else
	{
		aContext->OMSetRenderTargets(0, nullptr, GetDepthStencilView());
	}
	
	aContext->RSSetViewports(1, &myViewport);
}

void KE::DepthBuffer::SetAsResourceOnSlot(unsigned int aSlot, ID3D11DeviceContext* aContext) const
{
	assert(mySRV.Get());

	aContext->PSSetShaderResources(aSlot, 1, mySRV.GetAddressOf());
}

void KE::DepthBuffer::SetShaderResourceView(ID3D11ShaderResourceView* aSRV)
{
    mySRV = ComPtr<ID3D11ShaderResourceView>(aSRV);
}

Vector2i KE::DepthBuffer::CalculateTextureSize() const
{
	ID3D11Resource* resource = nullptr;
	mySRV->GetResource(&resource);
	if (!resource)
	{
		return Vector2i(0, 0);
	}
	ID3D11Texture2D* txt = reinterpret_cast<ID3D11Texture2D*>(resource);
	D3D11_TEXTURE2D_DESC desc;
	txt->GetDesc(&desc);

	Vector2i size(desc.Width, desc.Height);
	resource->Release();

	return size;
}
