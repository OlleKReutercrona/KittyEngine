#pragma once
#include <array>
#include <d3d11.h>
#include <wrl/client.h>

namespace DirectX
{
	struct XMINT2;
}

namespace KE
{
	using Microsoft::WRL::ComPtr;

	class Graphics;
	class GBuffer
	{
		friend class Graphics;
	public:
		enum class GBufferTexture
		{
			WorldPosition,
			Albedo,
			Normal,
			Material,
			Effect,
			AmbientOcclusionAndCustom,
			Count,
		};

		GBuffer();
		~GBuffer();

		static GBuffer Create(DirectX::XMINT2 aSize, ID3D11Device* aDevice);
		void ClearTextures(ID3D11DeviceContext* aContext, bool aClearDepth = true) const;
		void SetAsActiveTarget(ID3D11DeviceContext* aContext, ID3D11DepthStencilView* aDepth = nullptr);
		void SetAsResourceOnSlot(ID3D11DeviceContext* aContext, GBufferTexture aTexture, unsigned int aSlot);
		void SetAllAsResources(ID3D11DeviceContext* aContext, unsigned int aSlot);
		void ClearAllAsResourcesSlots(ID3D11DeviceContext* aContext, unsigned int aSlot);
		void ResizeViewport(ID3D11Device* aDevice, const DirectX::XMINT2 aSize);
		void SetDepthAsResourceOnSlot(ID3D11DeviceContext* aContext, unsigned int aSlot);
		void UnbindTarget(ID3D11DeviceContext* aContext);

		inline ID3D11ShaderResourceView* const* GetShaderResourceViews() const
		{
			return myShaderResourceViews[0].GetAddressOf();
		}

		inline ID3D11ShaderResourceView* const* GetDepthShaderResourceView() const
		{
			return myDepthStencilShaderResourceView.GetAddressOf();
		}

		inline ID3D11DepthStencilView* const GetDepthStencilView() const
		{
			return myDepthStencilView.Get();
		}

		ID3D11Texture2D* GetTexture(const unsigned int aIndex);

		void CopyFrom(ID3D11DeviceContext* aContext, GBuffer* aBuffer);

	private:
		void Resize(ID3D11Device* aDevice, const DirectX::XMINT2& aSize);
		void CreateRTandDSV(ID3D11Device* aDevice, const DirectX::XMINT2 aSize);
		void ReleaseResources();

		std::array<ComPtr<ID3D11Texture2D>, static_cast<size_t>(GBufferTexture::Count)> myTextures;
		std::array<ComPtr<ID3D11RenderTargetView>, static_cast<size_t>(GBufferTexture::Count)>
		myRenderTargetViews;
		std::array<ComPtr<ID3D11ShaderResourceView>, static_cast<size_t>(GBufferTexture::Count)>
		myShaderResourceViews;
		D3D11_VIEWPORT myViewport;

		// Depth Stencil
		ComPtr<ID3D11Texture2D> myDepthStencilTexture;
		ComPtr<ID3D11DepthStencilView> myDepthStencilView;
		ComPtr<ID3D11ShaderResourceView> myDepthStencilShaderResourceView;
	};
}
