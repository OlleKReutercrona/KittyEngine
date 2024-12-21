#pragma once

//#include "Engine/Source/Graphics/Graphics.h"

#include <array>
#include <DirectXMath.h>
#include <d3d11.h>
#include <string>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

namespace KE
{
	//static constexpr UINT MAX_LIGHTS = 16u; // Needs to be the same in Model_PBR_PS.hlsl
	static constexpr SIZE_T MAX_LIGHT_VECTOR_CAPACITY = 2048;

	static constexpr DirectX::XMFLOAT3 DEFAULT_POSITION = {0.0f, 0.0f, 0.0f};
	static constexpr DirectX::XMFLOAT3 DEFAULT_DIRECTION = {0.0f, -1.0f, 0.0f};
	static constexpr DirectX::XMFLOAT3 DEFAULT_COLOUR = {1.0f, 1.0f, 1.0f};
	static constexpr float DEFAULT_LIGHT_INTENSITY = 50.0f;
	static constexpr float DEFAULT_DIRECTIONAL_LIGHT_INTENSITY = 1.0f;
	static constexpr float DEFAULT_AMBIENT_INTENSITY = 1.0f;
	static constexpr float DEFAULT_RANGE = 100.0f;
	static constexpr float DEFAULT_INNER_ANGLE = 1.5f;
	static constexpr float DEFAULT_OUTER_ANGLE = 2.5f;
	static constexpr bool DEFAULT_IS_ACTIVE = true;

	enum class eLightType
	{
		Directional,
		Point,
		Spot,
	};

	static std::string LIGHT_TYPE_NAME[] =
	{
		"Directional Light",
		"Point Light",
		"Spot Light"
	};

	struct LightData { };

	struct DirectionalLightData : LightData
	{
		DirectX::XMFLOAT3 myDirection = {}; // 12
		float myDirectionalLightIntensity = {}; // 16
		DirectX::XMFLOAT3 myColour = {}; // 28
		float myAmbientLightIntensity = {}; // 32
		DirectX::XMMATRIX myDirectionalLightCamera; // 96
	};

	struct PointLightData : LightData
	{
		DirectX::XMFLOAT3 myPosition = {}; // 12
		float myIntensity = {}; // 16
		DirectX::XMFLOAT3 myColour = {}; // 28
		float myRange = {}; // 32
		BOOL isActive = false; // 36
		float padding[3] = {}; // 48
	};

	struct SpotLightData : LightData
	{
		DirectX::XMFLOAT3 myPosition = {}; // 12
		float myIntensity = {}; // 16
		DirectX::XMFLOAT3 myDirection = {}; // 28
		float myRange = {}; // 32
		DirectX::XMFLOAT3 myColour = {}; // 44
		float myInnerAngle = {}; // 48
		float myOuterAngle = {}; // 52
		BOOL isActive = false; // 56
		float padding[2] = {}; // 64
	};

	struct LightConstantBuffer
	{
		DirectionalLightData myDirectionalLightData = {};
		std::array<PointLightData, KE_MAX_LIGHTS> myPointLightData = {};
		std::array<SpotLightData, KE_MAX_LIGHTS> mySpotLightData = {};
		UINT myNumberOfPointLights = {};
		UINT myNumberOfSpotLights = {};
		float padding[2] = {};
	};
}
