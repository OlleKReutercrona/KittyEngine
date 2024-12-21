#pragma once
#include <vector>

#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/Lighting/Lighting.h"
#include "Engine/Source/Graphics/ModelData.h"
#include "Engine/Source/Graphics/DepthBuffer.h"
#include "Engine/Source/Graphics/Camera.h"

namespace DirectX
{
	class ID3D11Device;
}

namespace KE
{
	struct LightComponentData;
	class VertexShader;
	class PixelShader;
	class ShaderLoader;
	class Graphics;
	class TextureLoader;
	struct Cubemap;

	constexpr float unityLightIntensityMultiplier = 20.0f;
	constexpr float unityLightRangeMultiplier = 2.0f;

	class DeferredLightManager
	{
	public:
		DeferredLightManager();
		~DeferredLightManager() = default;

		void InitShadowCamera(const Vector2i aSize, const float aNear, const float aFar);
		HRESULT Init(ID3D11Device* aDevice, ShaderLoader* aShaderLoader, const Vector2i aSize);

		void PrepareShadowPass(Graphics* aGraphics);
		int Render(Graphics* aGraphics);
		void Reset();

		// This is a dangerous function. If a Vector of LightData resizes will cause the pointers to break
		LightData* CreateLightData(eLightType aLightType);
		void RemoveLightData(eLightType aType, LightData* aLightData);

		void AssignCubemap(Cubemap* aCubemap);

		Camera myDirectionalLightCamera;
		DepthBuffer myDepthBuffer;

		Cubemap* GetCubemap() const { return myCubemap; }
	private:
		struct ConstantBuffer
		{
			float positionAndRange[4];
			BOOL isDirectional;
			float padding[3];
		};

		ID3D11Device* myDevice = nullptr;

		Cubemap* myCubemap = nullptr;

		VertexShader* myLightVS = nullptr;

		PixelShader* myPointLightPS = nullptr;
		std::vector<PointLightData> myPointLights = {};
		std::vector<PointLightData*> myFreePointLights = {};
		CBuffer myPointLightBuffer;

		PixelShader* mySpotLightPS = nullptr;
		std::vector<SpotLightData> mySpotLights = {};
		std::vector<SpotLightData*> myFreeSpotLights = {};

		CBuffer mySpotLightbuffer;

		PixelShader* myDirectionalLightPS = nullptr;
		DirectionalLightData myDirectionalLight;
		CBuffer myDirectionalLightBuffer;
		CBuffer myTransformBuffer;

		Mesh myQuad;
		Mesh mySphere;

		std::string myLightVSName = "DeferredLight_VS.cso";
		std::string myDirectionalLightPSName = "DeferredDirectionalLight_PS.cso";
		std::string myPointLightPSName = "DeferredPointLight_PS.cso";
		std::string mySpotLightPSName = "DeferredSpotLight_PS.cso";
	};
}
