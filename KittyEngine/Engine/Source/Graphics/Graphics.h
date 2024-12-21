#pragma once
#include <wrl.h>
#include <DirectXMath.h>
#include <d3d11.h>

#include "Engine/Source/Graphics/ModelData.h"
#include "Engine/Source/Graphics/ModelLoader.h"
#include "Engine/Source/Graphics/Texture/TextureLoader.h"
#include "Engine/Source/Graphics/ShaderLoader.h"
#include "Engine/Source/Graphics/CameraManager.h"
#include "Engine/Source/Graphics/RenderTarget.h"
#include "Engine/Source/Graphics/PostProcessing.h"
#include "Engine/Source/Graphics/FX/VFX.h"
#include "Engine/Source/Graphics/DistanceFog.h"

#include "Sprite/SpriteManager.h"
/// TEST
#include "AssetLoader/AssetLoader.h"
#include "Decals/DecalManager.h"
#include "Engine/Source/Graphics/DebugLine.h"
#include "Engine/Source/Graphics/DebugRenderer.h"
#include "Engine/Source/Graphics/GBuffer.h"
#include "Engine/Source/Math/Vector2.h"
#include "Lighting/DeferredLightManager.h"
#include "Engine/Source/UI/GUIHandler.h"

#include "CBuffer.h"
#include "RenderLayer.h"
#include "SSAO.h"
#include "Renderers/ShellTexturedRenderer.h"

namespace KE
{
	struct ResolutionEvent;
	class ModelComponent;

	using Microsoft::WRL::ComPtr;

	struct CommonBuffer
	{
		DirectX::XMMATRIX worldToClipMatrix;
		DirectX::XMMATRIX clipToWorldMatrix;

		DirectX::XMMATRIX projectionMatrix;
		DirectX::XMMATRIX viewMatrix;

		DirectX::XMMATRIX projMatrixInverse;
		DirectX::XMMATRIX viewMatrixInverse;

		DirectX::XMFLOAT4 cameraPosition;
		DirectX::XMUINT2 resolution;
		float timePassed;
		float deltaTime;

		DirectX::XMFLOAT2 camNearFar;
		DirectX::XMFLOAT2 PADDING2;
	};

	class Graphics
	{
		friend class SpriteManager;
		friend class DebugRenderer;

	public:
		Graphics(HWND aHWnd, UINT aWidth, UINT aHeight);
		Graphics(const Graphics&) = delete;
		Graphics& operator=(const Graphics&) = delete;
		~Graphics();

		void Init();
		void SetRenderResolution(int aWidth, int aHeight);
		void SetViewport(int aWidth, int aHeight, float aTopLeftX = 0.0f, float aTopLeftY = 0.0f);
		int GetRenderWidth() const;
		int GetRenderHeight() const;
		Vector2i GetRenderSize() const;

		void BeginFrame();
		void SetCommonBuffer(Camera& aCamera);
		void EndFrame();

		void RenderDebug();
		void DrawLayer(Camera* camera, eRenderLayers layer, bool debugDraw);
		void CompositeLayers(KE::RenderTarget& workingRenderTarget, eRenderLayers aHighestLayer);
		void PerformPostProcess(KE::RenderTarget& workingRenderTarget);
		bool DetermineBackbufferRender();
		void PerformBackBufferRender();
		void Render();

		// Common Getters
		inline ComPtr<ID3D11Device> GetDevice() const { return myDevice; }
		inline ComPtr<ID3D11DeviceContext> GetContext() const { return myContext; }
		inline ComPtr<ID3D11DepthStencilView> GetDepthStencil() const { return myDepthStencil; }

		void SetProjection(const DirectX::XMMATRIX& aProjectionMatrix);
		void SetView(const DirectX::XMMATRIX& aViewMatrix);

		//asset stuff!
		void AssignCubemap(const std::string& aPath);
		void AssignSkybox(const std::string& aPath);
		ModelData* CreateModelData(const std::string& aModelName);
		SkeletalModelData* CreateSkeletalModelData(const std::string& aModelName, KE::SkeletalModelData* aModelData = nullptr);
		void ClearModelData();

		/// Object Getters ///
		inline GBuffer& GetGBuffer() { return myGBuffer; }
		inline GBuffer& GetSecondaryGBuffer() { return mySecondaryGBuffer; }
		inline ModelLoader& GetModelLoader() { return myModelLoader; }
		inline TextureLoader& GetTextureLoader() { return myTextureLoader; }
		inline ShaderLoader& GetShaderLoader() { return myShaderLoader; }
		inline CameraManager& GetCameraManager() { return myCameraManager; }
		inline DeferredLightManager& GetDeferredLightManager() { return myDeferredLightManager; }
		inline AssetLoader& GetAssetLoader() { return myAssetLoader; }
		inline int GetDrawCalls() { return myDrawCalls; }
		inline int GetLightDrawCalls() { return myLightDrawCalls; }
		inline int GetLastDrawCalls() { return myDrawCallsLastFrame; }
		inline int GetLastLightDrawCalls() { return myLightDrawCallsLastFrame; }
		inline PostProcessing* GetPostProcessing() { return &myPostProcess; }
		inline Camera& GetMainCamera() { return *myCameraManager.GetMainCamera(); }
		inline SpriteManager& GetSpriteManager() { return mySpriteManager; }
		inline VFXManager& GetVFXManager() { return myVFXManager; }
		inline DebugRenderer& GetDebugRenderer() { return myDebugRenderer; }
		inline DecalManager& GetDecalManager() { return myDecalManager; }

		inline SSAO& GetSSAO() { return mySSAO; }

		/// Const Object Getters ////
		inline const ModelLoader& GetModelLoader() const { return myModelLoader; }
		inline const TextureLoader& GetTextureLoader() const { return myTextureLoader; }
		inline const ShaderLoader& GetShaderLoader() const { return myShaderLoader; }
		inline const CameraManager& GetCameraManager() const { return myCameraManager; }
		inline const VFXManager& GetVFXManager() const { return myVFXManager; }

		/// Setters ///
		void SetBlendState(eBlendStates aBlendState);
		void SetDepthStencilState(eDepthStencilStates aDepthStencilState);
		void SetRasterizerState(eRasterizerStates aRasterizerState);
		void SetRenderTarget(RenderTarget* aRenderTarget, bool aUseDepth = true);

		/// Debug Lines ///
		void AddLine(LineVertex& aFirstLine, LineVertex& aSecondLine);
		void DrawLineCube(const Vector3f& aPosition, const Vector3f& aSize, const Vector4f& aColour);
		void DrawLineCube(const Vector3f& aPosition, const Vector3f& aSize, const Transform& aTransform, const Vector4f& aColour);
		void DrawLineSphere(const Vector3f& aPosition, const float aRadius, const Vector4f& aColour);
		void DrawCone(const Vector3f& aPosition, const Vector3f& aDirection, const float aLength, const float anOuterRadius, const float anInnerRadius, const Vector4f& aColour);
		void AddScreenSpaceLine(const Vector2f& aFrom, const Vector2f& aTo, const Vector4f& aColour);

		void RenderFullscreen(Texture* aTexture);

		//
		void AppendRenderTargetQueue(int aRenderTargetIndex, int aCameraIndex);
		RenderTarget* GetRenderTarget(int aIndex);
		//

		void DrawIndexed(UINT aIndexCount, UINT aStartIndexLocation, UINT aBaseVertexLocation);

		void IncrementDeferredPass();
		inline void AddDrawCall() { ++myDrawCalls; }
		void SetShouldDrawModels(bool aShouldDraw) { shouldDrawModels = aShouldDraw; }
		bool GetShouldDrawModels() const { return shouldDrawModels; }

		const DirectX::XMMATRIX& GetProjection() const { return myProjection; }
		const DirectX::XMMATRIX& GetView() const { return myView; }

		const ModelDataList& GetModelData() const { return myModelData; }
		const SkeletalModelDataList& GetSkeletalModelData() const { return mySkeletalModelData; }

		void BindTexture(const Texture* aTexture, unsigned int aStartSlot);
		void BindMaterial(const Material* aMaterial, unsigned int aStartSlot);

		RenderLayer* GetRenderLayer(eRenderLayers aLayer);
		void ResetRenderLayer(eRenderLayers aLayer);
		void ResetRenderLayers();

		ID3D11ShaderResourceView* GetBoundResource(unsigned int aSlot);

		inline BasicRenderer* GetDefaultRenderer() { return &myDefaultRenderer; }
		inline SkeletalRenderer* GetSkeletalRenderer() {return &mySkeletalRenderer; }
		inline InstancedRenderer* GetInstancedRenderer() {return &myInstancedRenderer; }
		inline DecalRenderer* GetDecalRenderer() {return &myDecalRenderer; }
		inline ShellTexturedRenderer* GetShellTexturedRenderer() {return &myShellTexturedRenderer; }

		void ToggleDrawSSAOAsResult();
		inline FullscreenAsset* GetFullscreenAsset() { return &myFullscreenAsset; }

		void AddWaterModel(ModelData* aModelData) { myWaterModels.push_back(aModelData); }
		void ClearWaterModels() { myWaterModels.clear(); }

#ifdef KITTYENGINE_SHIP
		void SetFullscreenStatus(bool aState) { isFullscreen = aState; }
		bool GetFullscreenStatus() const { return isFullscreen; }
#endif

	public:
		GBuffer myGBuffer;
		GBuffer mySecondaryGBuffer;
		DeferredLightManager myDeferredLightManager;

	private:
		bool CreateBlendStates();
		bool CreateDepthStencilStates();
		bool CreateRasterizerStates();

	private:
		std::array<ComPtr<ID3D11BlendState>, (int)eBlendStates::Count> myBlendStates;
		std::array<ComPtr<ID3D11DepthStencilState>, (int)eDepthStencilStates::Count> myDepthStencilStates;
		std::array<ComPtr<ID3D11RasterizerState>, (int)eRasterizerStates::Count> myRasterizerStates;
		SSAO mySSAO;

		PixelShader* myPSOverride = nullptr;

		std::vector<std::pair<int, int>> myRenderTargetQueue{};

		std::array<RenderLayer, (int)eRenderLayers::Count> myRenderLayers;

		std::vector<ModelData*> myWaterModels; 

		//SYSTEMS
		AssetLoader myAssetLoader;
		ModelLoader myModelLoader;
		TextureLoader myTextureLoader;
		ShaderLoader myShaderLoader;
		CameraManager myCameraManager;
		VFXManager myVFXManager;
		DecalManager myDecalManager;
		SpriteManager mySpriteManager;
		GUIHandler myGUIHandler;
		PostProcessing myPostProcess;
		DebugRenderer myDebugRenderer;
		//
		BasicRenderer myDefaultRenderer;
		SkeletalRenderer mySkeletalRenderer;
		InstancedRenderer myInstancedRenderer;
		DecalRenderer myDecalRenderer;
		ShellTexturedRenderer myShellTexturedRenderer;
		//

		std::vector<ModelData> myModelData{};
		std::vector<SkeletalModelData> mySkeletalModelData{};

		std::vector<RenderTarget> myRenderTargets{};

		DirectX::XMMATRIX myProjection{};
		DirectX::XMMATRIX myView{};

		ComPtr<ID3D11Device> myDevice;
		ComPtr<ID3D11DeviceContext> myContext;
		ComPtr<IDXGISwapChain> mySwap;

		// Samplers
		ComPtr<ID3D11SamplerState> mySampler;
		ComPtr<ID3D11SamplerState> myFullscreenEffectSampler;
		ComPtr<ID3D11SamplerState> myShadowSampler;
		ComPtr<ID3D11SamplerState> myShadowCompSampler;
		ComPtr<ID3D11SamplerState> mySSAOSampler;
		ComPtr<ID3D11SamplerState> myWrappingSampler;
		ComPtr<ID3D11SamplerState> myBilinearSampler;
		ComPtr<ID3D11SamplerState> myPointSampler;
		//

		ComPtr<ID3D11RenderTargetView> myRenderTarget;
		ComPtr<ID3D11DepthStencilView> myDepthStencil;

		/// BUFFERS
		ComPtr<ID3D11Buffer> myCommonBuffer1;
		CBuffer myCommonbuffer;


		DebugLine myDebugLines;
		DebugLine myLineGrid;
		DebugLine myFullscreenLines;

		// Outline Fullscreen Asset
		FullscreenAsset myFullscreenAsset;

		// Distance Fog
		DistanceFog myDistanceFog;

		// POST PROCESS

		ComPtr<ID3D11BlendState> myBlendState;
		UINT myWidth;
		UINT myHeight;
		Vector2i myRenderResolution;

		UINT myViewportWidth = 0u;
		UINT myViewportHeight = 0u;

		int myDrawCalls = 0;
		int myLightDrawCalls = 0;
		int myDrawCallsLastFrame = 0;
		int myLightDrawCallsLastFrame = 0;

		bool shouldDrawModels = true;
		bool isFullscreenRenderQueued = false;

		int myDrawSSAOPass = 0;

#ifdef KITTYENGINE_SHIP
		bool isFullscreen;
		int myFullscreenRenderWidth = 0;
		int myFullscreenRenderHeight = 0;
#endif

		eDeferredPasses myActivePass = eDeferredPasses::Camera;

		struct FrustumPlanes
		{
			DirectX::XMFLOAT4 planes[6];
		};

		Cubemap* myCubemap = nullptr;
		Cubemap* mySkybox = nullptr;

	public:
		FrustumPlanes ExtractFrustumPlanes() const;
		bool IsModelBoundsInFrustum(const Vector3f& aPosition, const ModelBounds& aModelBounds) const;
	};
}
