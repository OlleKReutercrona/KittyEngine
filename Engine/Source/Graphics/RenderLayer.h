#pragma once

#include "GBuffer.h"
#include "RenderTarget.h"
#include "Renderers/BasicRenderer.h"
#include "Renderers/DecalRenderer.h"
#include "Renderers/InstancedRenderer.h"
#include "Renderers/SkeletalRenderer.h"



namespace KE
{
	class Camera;
	struct InstancedRenderPackage;
	class InstancedRenderer;

	typedef int RenderLayerFlags;
	enum RenderLayerFlags_
	{
		RenderLayerFlags_Active			= 1 << 0,
		RenderLayerFlags_ReceiveShadows = 1 << 1,
		RenderLayerFlags_CastShadows	= 1 << 2,
		RenderLayerFlags_Lit			= 1 << 3,
		RenderLayerFlags_DrawModels		= 1 << 4,
		RenderLayerFlags_DrawSkeletal	= 1 << 6,
		RenderLayerFlags_DrawDecals		= 1 << 5,
		RenderLayerFlags_DrawSprites	= 1 << 7,
		RenderLayerFlags_DrawVFX		= 1 << 8,
		RenderLayerFlags_SSAO			= 1 << 9,


		RenderLayerFlags_Count = 10
	};

	inline const char* RenderLayerFlagStrings[] = {
			"Active",
			"ReceiveShadows",
			"CastShadows",
			"Lit",
			"DrawModels",
			"DrawDecals",
			"DrawSkeletal",
			"DrawSprites",
			"DrawVFX",
			"SSAO",
		};

	class RenderLayer
	{
		KE_EDITOR_FRIEND
		friend class Graphics;
	private:
		Graphics* myGraphics = nullptr;
		RenderTarget myProcessedRender{};

		InstancedRenderer* myInstancedRenderer = nullptr;
		SkeletalRenderer* mySkeletalRenderer = nullptr;
		BasicRenderer* myBasicRenderer = nullptr;
		DecalRenderer* myDecalRenderer = nullptr;

		std::vector<size_t> myModelDataIndices{};
		std::vector<size_t> mySkeletalModelDataIndices{};
		std::vector<size_t> myDecalIndices{};
		std::vector<size_t> myRegularModelIndices{};
		InstancedRenderPackageList myInstancedRenderPackages{};

		struct RenderLayerSettings
		{
			eDepthStencilStates depthStencilState = eDepthStencilStates::Count;
			eRasterizerStates rasterizerState = eRasterizerStates::Count;
			eBlendStates blendState = eBlendStates::Count;
			Camera* relativeCamera  = nullptr;
		} mySettings;

	public:
		RenderLayerFlags flags = RenderLayerFlags_Active | RenderLayerFlags_CastShadows | RenderLayerFlags_ReceiveShadows;

		void Init(Graphics* aGraphics);

		void AssignRenderers(
			BasicRenderer* aBasicRenderer,
			SkeletalRenderer* aSkeletalRenderer,
			InstancedRenderer* aInstancedRenderer,
			DecalRenderer* aDecalRenderer
		)
		{
			myInstancedRenderer = aInstancedRenderer;
			mySkeletalRenderer = aSkeletalRenderer;
			myBasicRenderer = aBasicRenderer;
			myDecalRenderer = aDecalRenderer;
		};

		void SetDepthStencilState(eDepthStencilStates aState) { mySettings.depthStencilState = aState; }
		void SetRasterizerState(eRasterizerStates aState) { mySettings.rasterizerState = aState; }
		void SetBlendState(eBlendStates aState) { mySettings.blendState = aState; }
		void SetRelativeCamera(Camera* aCamera) { mySettings.relativeCamera = aCamera; }

		const RenderLayerSettings& GetSettings() const { return mySettings; }

		void ApplySettings();

		void SetActive();

		//void Render(Camera* aCamera, Camera* aCameraRelative);

		void Render(Camera* aCamera, VertexShader* aVSOverride = nullptr, PixelShader* aPSOverride = nullptr);

		void RenderDecals(Camera* aCamera, GBuffer* aMainGBuffer, GBuffer* aCopyGBuffer, DecalManager* aDecalManager);

		//void Render(const DirectX::XMMATRIX& aView, 
		//			const DirectX::XMMATRIX& aProjection,
		//            VertexShader* aVSOverride = nullptr, 
		//			PixelShader* aPSOverride = nullptr
		//);

		void AddModelDataIndex(size_t aModelIndex);
		void AddSkeletalModelDataIndex(size_t aModelIndex);
		void AddDecalIndex(size_t aDecalIndex);

		void GenerateInstancingData(const ModelDataList& aModelDataList);

		//GBuffer* GetGBuffer() { return &myGBuffer; }
		RenderTarget* GetProcessedRender() { return &myProcessedRender; }

		void NewFrame();

		void Reset();
		void Resize(Vector2i aSize);
	};
}
