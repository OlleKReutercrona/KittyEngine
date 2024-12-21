#pragma once

#include <wrl.h>
#include <d3d11.h>

namespace KE
{
	using Microsoft::WRL::ComPtr;

	class Graphics;
	class RenderTarget
	{
		friend class Graphics;
	private:
		
		int myWidth = 0;
		int myHeight = 0;

		//actual texture part
		ComPtr<ID3D11Texture2D> myTexture;
		ComPtr<ID3D11RenderTargetView> myRenderTargetView;
		ComPtr<ID3D11ShaderResourceView> myShaderResourceView;		
		//depth stencil part
		ComPtr<ID3D11Texture2D> myDepthStencilTexture;
		ComPtr<ID3D11DepthStencilView> myDepthStencilView;

		ID3D11Device* myDevice;
		ID3D11DeviceContext* myContext;

		bool myShouldRenderDebug = false;
		bool myShouldRenderPostProcessing = false;

		DXGI_FORMAT myFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

		void Create();

	public:
		RenderTarget();
		void SetFormat(DXGI_FORMAT aFormat) { myFormat = aFormat; Create(); }
		
		//RenderTarget(const RenderTarget&) = delete;
		//RenderTarget& operator=(const RenderTarget&) = delete;
		~RenderTarget();
		
		void Init(ID3D11Device* aDevice, ID3D11DeviceContext* aContext, int aWidth, int aHeight);
		
		//usage functions
		void MakeActive(bool aUseDepth, ID3D11DepthStencilView* aDSV = nullptr);
		void MakeActive();
		
		void Free();
		void Clear(const float* aColour = nullptr, bool aClearDepth = true);

		void Resize(int aWidth, int aHeight);
		
		bool ShouldRenderDebug() const { return myShouldRenderDebug; }
		void SetShouldRenderDebug(bool aShouldRenderDebug);

		bool ShouldRenderPostProcessing() const { return myShouldRenderPostProcessing; }
		void SetShouldRenderPostProcessing(bool aShouldRenderPostProcessing) { myShouldRenderPostProcessing = aShouldRenderPostProcessing; }

		void SetAsShaderResource(const int aSlot);
		void SetAsVertexShaderResource(const int aSlot);

		inline ID3D11ShaderResourceView* GetShaderResourceView() const { return myShaderResourceView.Get(); }
		inline ID3D11DepthStencilView* GetDepthStencilView() const { return myDepthStencilView.Get(); }
		inline ID3D11RenderTargetView* GetRenderTargetView() const { return myRenderTargetView.Get(); }

		void CopyFrom(RenderTarget* aRenderTarget, bool aCopyDepth = false);
		void CopyFrom(ID3D11Texture2D* aTexture);
		void CopyFrom(ID3D11ShaderResourceView* aShaderResourceView);

		ID3D11Texture2D* GetTexture() const { return myTexture.Get(); }

	};
}