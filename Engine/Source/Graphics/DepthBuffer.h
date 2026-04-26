#pragma once
#include <wrl/client.h>
#include <d3d11.h>

using Microsoft::WRL::ComPtr;

struct ID3D11DepthStencilView;
struct ID3D11Texture2D;
struct D3D11_VIEWPORT;

namespace KE
{
	class RenderTarget;

	class DepthBuffer
	{
	public:
		DepthBuffer();

		void Init(ID3D11Device* aDevice, const Vector2i aSize);

		void Resize(ID3D11Device* aDevice, const Vector2i aSize);

		void Clear(ID3D11DeviceContext* aContext, float aClearDepthValue = 1.0f, uint8_t aClearStencilValue = 0);
		ID3D11DepthStencilView* GetDepthStencilView() { return myDepth.Get(); };

		void SetAsActiveTarget(ID3D11DeviceContext* aContext, KE::RenderTarget* aRT = nullptr);

		void SetAsResourceOnSlot(unsigned int aSlot, ID3D11DeviceContext* aContext) const;
		void SetShaderResourceView(ID3D11ShaderResourceView* aSRV);
		ID3D11ShaderResourceView* GetShaderResourceView() const { return mySRV.Get(); };
		Vector2i CalculateTextureSize() const;
	private:
		void Create(ID3D11Device* aDevice, const Vector2i aSize);

		ComPtr<ID3D11DepthStencilView> myDepth = 0;
		D3D11_VIEWPORT myViewport = {};
		ComPtr<ID3D11ShaderResourceView> mySRV = nullptr;
	};
}

