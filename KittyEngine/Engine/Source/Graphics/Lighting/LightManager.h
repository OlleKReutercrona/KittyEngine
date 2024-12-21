#pragma once
#include <vector>

#include "Lighting.h"
#include "Engine/Source/ComponentSystem/Components/LightComponent.h"

namespace KE
{
	enum class eLightType;

	//constexpr float unityLightIntensityMultiplier = 2.0f;
	//constexpr float unityLightRangeMultiplier = 2.0f;

	class LightManager
	{
	public:
		LightManager(ID3D11Device* aDevice, UINT aSlot = 0u);
		~LightManager();
		void BindBuffer(ID3D11DeviceContext* aContext);
		LightComponentData& CreateLightData(eLightType aLightType);

		void Clear();

	private:
		void UpdateBuffer(ID3D11DeviceContext* aContext);

	private:
		UINT myNumberOfDirectionalLights = 0u;
		UINT myNumberOfPointLights = 0u;
		UINT myNumberOfSpotLights = 0u;

		LightConstantBuffer myLightBuffer;
		ComPtr<ID3D11Buffer> myConstantBuffer;
		UINT mySlot;

		std::vector<LightComponentData> myLightComponentData = {};
		std::vector<LightData*> myLightData = {};
		DirectionalLightData myDirectionalLightData;
	};
}
