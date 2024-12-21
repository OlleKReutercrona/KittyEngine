#pragma once
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

struct ID3D11Buffer;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct D3D11_BUFFER_DESC;

namespace KE
{
	class CBuffer
	{
	public:
		CBuffer();
		~CBuffer();
		
		ComPtr<ID3D11Buffer>& GetBuffer();

		HRESULT Init(ComPtr<ID3D11Device> aDevice, D3D11_BUFFER_DESC* aDesc);

		void MapBuffer(void* aData, int aSize, ID3D11DeviceContext* aContext);

		void BindForPS(const int aSlot, ID3D11DeviceContext* aContext);
		void BindForVS(const int aSlot, ID3D11DeviceContext* aContext);
	private:
		ComPtr<ID3D11Buffer> myBuffer;
	};
}

