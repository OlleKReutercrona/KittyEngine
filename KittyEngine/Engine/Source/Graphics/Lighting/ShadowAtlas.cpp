#include "stdafx.h"
#include "ShadowAtlas.h"
#include <d3d11.h>


Vector2i KE::ShadowAtlas::CalculateTextureSize() const
{
	return Vector2i();
}

void KE::ShadowAtlas::Create(ID3D11Device* aDevice, const Vector2i aShadowAtlasSize)
{
	HRESULT result;
	D3D11_TEXTURE2D_DESC desc = { 0 };
	desc.Width = aShadowAtlasSize.x;
	desc.Height = aShadowAtlasSize.y;
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
}

const Vector2i KE::ShadowAtlas::MapViewPort(const unsigned int aSize)
{
	Vector2i position(-1, -1);

	unsigned int convertedSize = 16384 / aSize;
	unsigned int step = 128 / convertedSize;

	bool isDone = false;
	// Look for a free slot on map
	for (unsigned int y = 0; y < convertedSize; y++)
	{
		for (unsigned int x = 0; x < convertedSize; x++)
		{
			int xCoord = x * step;
			int yCoord = (y * step) * 64;
			int index = yCoord + xCoord;

			if (myMap[index] != 0) continue;

			position = { index % 64, static_cast<int>(index / 64) };

			if (IsMapQuadrantEmpty(convertedSize / 2, position))
			{
				isDone = true;
				break;
			}
		}
		if (isDone) break;
	}

	// if no position found, return negative 
	if (position.x == -1) return Vector2i(-1, -1);

	// Register viewport to map
	for (unsigned y = 0; y < step; y++)
	{
		for (unsigned x = 0; x < step; x++)
		{
			int index = (position.y + y * 64) + (position.x + x);
			myMap[index] = 1;
		}
	}

	position *= 128;

	return position;
}
const bool KE::ShadowAtlas::IsMapQuadrantEmpty(const unsigned int aSize, const Vector2i aStartPosition)
{
	if (aSize <= 128) return true;

	Vector2i firstPos = aStartPosition;
	firstPos.x += aSize;

	int index = firstPos.y * 64 + firstPos.x;
	if (myMap[index] != 0)
	{
		return false;
	}

	Vector2i secondPos = aStartPosition;
	secondPos.y += aSize;

	index = secondPos.y * 64 + secondPos.x;
	if (myMap[index] != 0)
	{
		return false;
	}

	Vector2i thirdPos = aStartPosition;
	thirdPos.x += aSize;
	thirdPos.y += aSize;

	index = thirdPos.y * 64 + thirdPos.x;
	if (myMap[index] != 0)
	{
		return false;
	}

	if (!IsMapQuadrantEmpty(aSize / 2, firstPos)) return false;
	if (!IsMapQuadrantEmpty(aSize / 2, secondPos)) return false;
	if (!IsMapQuadrantEmpty(aSize / 2, thirdPos)) return false;


	return true;
}

KE::ShadowAtlas::~ShadowAtlas()
{
	for (int i = 0; i < myViewPorts.size(); i++)
	{
		delete myViewPorts[i];
	}
	myViewPorts.clear();
}

void KE::ShadowAtlas::Clear(ID3D11DeviceContext* aContext, float aClearDepthValue, uint8_t aClearStencilValue)
{
	aContext->ClearDepthStencilView(myDepth.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, aClearDepthValue, aClearStencilValue);
}

void KE::ShadowAtlas::Init(ID3D11Device* aDevice, const Vector2i aShadowAtlasSize)
{
	Create(aDevice, aShadowAtlasSize);
}

void KE::ShadowAtlas::SetAsActiveTarget(ID3D11DeviceContext* aContext, const D3D11_VIEWPORT* aViewPort)
{
	aContext->OMSetRenderTargets(0, nullptr, myDepth.Get());

	aContext->RSSetViewports(1, aViewPort);
}

bool KE::ShadowAtlas::RegisterViewport(const unsigned int aSize, D3D11_VIEWPORT** anOutViewport, UINT& aShadowMapInfo)
{
	Vector2i mapCoords = MapViewPort(aSize);

	if (mapCoords.x == -1)
	{
		anOutViewport = nullptr;
		return false;
	}
	*anOutViewport = myViewPorts.emplace_back(new D3D11_VIEWPORT());

	**anOutViewport = {
		static_cast<float>(mapCoords.x),
		static_cast<float>(mapCoords.y),
		static_cast<float>(aSize),
		static_cast<float>(aSize),
		0,
		1
	};

	//constexpr unsigned int mapY = 2048 << 18;
	//constexpr unsigned int mapX = 2048 << 4;
	//constexpr unsigned int SIZE = 3;

	//constexpr unsigned int res = mapY | mapX | SIZE;

	// This is very uggo
	//		Top Left Y			Top Left X		  Size
	// 0b 0000'0000'0000'00 | 00'1000'0000'0000 | 0000;
	
	
	unsigned int mapY = static_cast<unsigned int>(mapCoords.y) << 18;
	unsigned int mapX = static_cast<unsigned int>(mapCoords.x) << 4;
	unsigned int SIZE = 0;

	switch (aSize)
	{
	case 128:
	{
		SIZE = 0b0000'0000'0000'0000'0000'0000'0000'0000;
		break;
	}
	case 256:
	{
		SIZE = 0b0000'0000'0000'0000'0000'0000'0000'0001;
		break;
	}
	case 512:
	{
		SIZE = 0b0000'0000'0000'0000'0000'0000'0000'0010;
		break;
	}
	case 1024:
	{
		SIZE = 0b0000'0000'0000'0000'0000'0000'0000'0011;
		break;
	}
	case 2048:
	{
		SIZE = 0b0000'0000'0000'0000'0000'0000'0000'0100;
		break;
	}
	case 4096:
	{
		SIZE = 0b0000'0000'0000'0000'0000'0000'0000'0101;
		break;
	}
	default:
		break;
	}

	aShadowMapInfo = mapY | mapX | SIZE;

	return true;
}

void KE::ShadowAtlas::SetAsResourceOnSlot(unsigned int aSlot, ID3D11DeviceContext* aContext) const
{
	assert(mySRV.Get());

	aContext->PSSetShaderResources(aSlot, 1, mySRV.GetAddressOf());
}

void KE::ShadowAtlas::SetAsComputeShaderResourceOnSlot(unsigned int aSlot, ID3D11DeviceContext* aContext)
{
	assert(mySRV.Get());

	aContext->CSSetShaderResources(aSlot, 1, mySRV.GetAddressOf());
}
