#pragma once

#include <d3d11.h>
#include <wrl.h>

#define KE_DEFERRED_RENDER_TARGET_COUNT 5

namespace KE
{
	using Microsoft::WRL::ComPtr;

	class DeferredRenderTarget
	{
	private:

		int myWidth = 0;
		int myHeight = 0;
		

		//actual texture part
		ComPtr<ID3D11Texture2D> myTextures[KE_DEFERRED_RENDER_TARGET_COUNT];
		ComPtr<ID3D11RenderTargetView> myRenderTargetViews[KE_DEFERRED_RENDER_TARGET_COUNT];
		ComPtr<ID3D11ShaderResourceView> myShaderResourceViews[KE_DEFERRED_RENDER_TARGET_COUNT];
		//depth stencil part
		ComPtr<ID3D11Texture2D> myDepthStencilTexture;
		ComPtr<ID3D11DepthStencilView> myDepthStencilView;
		ComPtr<ID3D11ShaderResourceView> myDepthStencilShaderResourceView;

		ID3D11Device* myDevice;
		ID3D11DeviceContext* myContext;

		bool myShouldRenderDebug = false;

		void Create();

	public:
		DeferredRenderTarget(ID3D11Device* aDevice, ID3D11DeviceContext* aContext, int aWidth, int aHeight);

		~DeferredRenderTarget();
		//

		//usage functions
		void MakeActive(bool aUseDepth);

		void Free();
		void Clear(float* aColour = nullptr, bool aClearDepth = true);

		void Resize(int aWidth, int aHeight);

		inline ID3D11ShaderResourceView* const * GetShaderResourceViews() const { return myShaderResourceViews[0].GetAddressOf(); }
		inline ID3D11DepthStencilView* GetDepthStencilView() const { return myDepthStencilView.Get(); }
		inline ID3D11ShaderResourceView* GetDepthStencilShaderResourceView() const { return myDepthStencilShaderResourceView.Get(); }

	};
}