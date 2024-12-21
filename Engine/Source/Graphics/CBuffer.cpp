#include "stdafx.h"
#include "CBuffer.h"
#include <d3d11.h>

KE::CBuffer::CBuffer()
{
}

KE::CBuffer::~CBuffer()
{
	myBuffer.Reset();
}

ComPtr<ID3D11Buffer>& KE::CBuffer::GetBuffer()
{
	return myBuffer; 
}

HRESULT KE::CBuffer::Init(ComPtr<ID3D11Device> aDevice, D3D11_BUFFER_DESC* aDesc)
{
	HRESULT result = aDevice->CreateBuffer(&*aDesc, nullptr, &myBuffer);
	if (FAILED(result))
	{
		return result;
	}

	return S_OK;
}

void KE::CBuffer::MapBuffer(void* aData, int aSize, ID3D11DeviceContext* aContext)
{
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	aContext->Map(myBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, aData, aSize);
	aContext->Unmap(myBuffer.Get(), 0);
}

void KE::CBuffer::BindForPS(const int aSlot, ID3D11DeviceContext* aContext)
{
	aContext->PSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
}

void KE::CBuffer::BindForVS(const int aSlot, ID3D11DeviceContext* aContext)
{
	aContext->VSSetConstantBuffers(aSlot, 1, myBuffer.GetAddressOf());
}
