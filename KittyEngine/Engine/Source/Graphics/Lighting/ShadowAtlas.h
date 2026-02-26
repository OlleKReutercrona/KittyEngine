#pragma once
#include <wrl/client.h>
#include <bitset>

using Microsoft::WRL::ComPtr;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11UnorderedAccessView;
struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;
struct D3D11_VIEWPORT;

namespace KE
{
	class RenderTarget;


	class ShadowAtlas
	{
	public:
		ShadowAtlas() = default;
		~ShadowAtlas();


		void Clear(ID3D11DeviceContext* aContext, float aClearDepthValue = 1.0f, uint8_t aClearStencilValue = 0);

		void Init(ID3D11Device* aDevice, const Vector2i aShadowAtlasSize);

		ID3D11DepthStencilView* GetDepthStencilView() { return myDepth.Get(); };

		void SetAsActiveTarget(ID3D11DeviceContext* aContext, const D3D11_VIEWPORT* aViewPort);

		bool RegisterViewport(const unsigned int aSize, D3D11_VIEWPORT** anOutViewport, UINT& aShadowMapInfo);

		void SetAsResourceOnSlot(unsigned int aSlot, ID3D11DeviceContext* aContext) const;

		void SetAsComputeShaderResourceOnSlot(unsigned int aSlot, ID3D11DeviceContext* aContext);

		inline D3D11_VIEWPORT* GetViewPort(const unsigned int anIndex) { return myViewPorts[anIndex]; }

		ID3D11ShaderResourceView* GetShaderResourceView() const { return mySRV.Get(); };
		Vector2i CalculateTextureSize() const;
	private:
		void Create(ID3D11Device* aDevice, const Vector2i aShadowAtlasSize);
		
		const Vector2i MapViewPort(const unsigned int aSize);

		const bool IsMapQuadrantEmpty(const unsigned int aSize, const Vector2i aStartPosition);

	private:
		std::vector<D3D11_VIEWPORT*> myViewPorts;
		ComPtr<ID3D11ShaderResourceView> mySRV = nullptr;
		ComPtr<ID3D11DepthStencilView> myDepth = 0;


		/* 
		0 is top left on texture and 4096 is bottom right 
		64 * 64 = 4096
		*/
		std::bitset<4096> myMap;
	};
}

