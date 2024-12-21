#pragma once
#include "ModelData.h"
#include "Shader.h"

#include <d3d11.h>

#include "CBuffer.h"

struct ID3D11ShaderResourceView;

namespace KE
{

	struct Texture;
	class Graphics;

	struct UVTransform
	{
		Vector2f uvMin;
		Vector2f uvMax;
	};

	class FullscreenAsset
	{
	private:

		Mesh myMesh;

		CBuffer myCBuffer;

		void MakeUVTransform(Graphics* aGraphics, Vector2f aUVMin, Vector2f aUVMax);
	public:
		FullscreenAsset();
		~FullscreenAsset();

		bool Init(Graphics* aGraphics);

		inline void AssignTexture(Texture* aTexture) { myTexture = aTexture; }
		void Render(
			Graphics* aGraphics,
			const Vector2f& uvMin = {0.0f, 0.0f},
			const Vector2f& uvMax = {1.0f, 1.0f}
		);
		void Render(
			Graphics* aGraphics,
			ID3D11ShaderResourceView* aShaderResourceView,
			VertexShader* aVS = nullptr,
			PixelShader* aPS  = nullptr,
			const Vector2f& uvMin = {0.0f, 0.0f},
			const Vector2f& uvMax = {1.0f, 1.0f}
		);
		void RenderWithoutSRV(
			Graphics* aGraphics,
			VertexShader* aVS = nullptr,
			PixelShader* aPS = nullptr,
			const Vector2f& uvMin = {0.0f, 0.0f},
			const Vector2f& uvMax = {1.0f, 1.0f}
		);

		VertexShader* myVertexShader = nullptr;
		PixelShader* myPixelShader = nullptr;
		Texture* myTexture = nullptr;
	};
}