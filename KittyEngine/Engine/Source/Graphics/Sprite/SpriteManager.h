#pragma once
#include <vector>
#include <unordered_map>
#include "Sprite.h"

#define MAX_SPRITE_INSTANCES 1024

struct ID3D11Buffer;

namespace KE
{
	class Graphics;
	class Camera;

	struct SpriteVertex
	{
		float x, y, z, w;
		float u, v;
		float padding[2];
	};

	struct SpriteRenderBuffer
	{
		DirectX::XMMATRIX completeTransform; //64

		struct
		{
			struct
			{
				float u = 0.0f;
				float v = 0.0f;
			} min;

			struct
			{
				float u = 1.0f;
				float v = 1.0f;
			} max;
		} uvRegion; // 16

		int displayMode; // 4
		int myEffectType = (int)eEffectType::None;
		BOOL flipX = false;
		BOOL flipY = false;
	};


	class SpriteManager
	{
	private:
		std::vector<SpriteBatch*> mySpriteBatches;
		std::vector<SpriteBatch*> myScreenSpriteBatches;
		Graphics* myGraphics = nullptr;

		//the quad that all sprites are rendered to
		ID3D11Buffer* myInstanceBuffer = nullptr;
		ID3D11Buffer* mySpriteRenderBuffer = nullptr;

		ID3D11Buffer* myTextBuffer = nullptr;

		ID3D11Buffer* myVertexBuffer = nullptr;
		ID3D11Buffer* myIndexBuffer = nullptr;

	public:
		SpriteManager();
		~SpriteManager();

		void Init(Graphics* aGraphics);

		void BindBuffers(SpriteBatch& aSpriteBatch, KE::Camera* aCamera);
		void RenderBatch(SpriteBatch& aSpriteBatch);

		void Render(KE::Camera* aCamera, eRenderLayers aLayer);

		void BeginFrame();
		void EndFrame();

		void QueueSpriteBatch(SpriteBatch* aSpriteBatch);
	};
}
