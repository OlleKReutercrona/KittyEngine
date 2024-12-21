#pragma once
//#include <Engine/Source/Windows/KittyEngineWin.h>
#include "Engine\Source\Math\KittyMath.h"

namespace KE
{
#define KITTY_CLEAR_COLOUR {0.75f, 0.75f, 0.75f, 1.0f}
	static constexpr UINT COMMON_BUFFER_PS_SLOT = 0u;
	static constexpr UINT COMMON_BUFFER_VS_SLOT = 0u;
	static constexpr UINT LIGHT_BUFFER_PS_SLOT = 2u;
	static constexpr UINT DEFERRED_LIGHT_BUFFER_SLOT = 2u;

	struct VertexPoint
	{
		VertexPoint() = default;
		VertexPoint(const float aX, const float aY, const float aZ, const float aW) : x(aX), y(aY), z(aZ), w(aW) {};
		VertexPoint(const Vector4f aVector) : x(aVector.x), y(aVector.y), z(aVector.z), w(aVector.w) {};
		float x;
		float y;
		float z;
		float w;
	};

#pragma region DeferredPasses
	enum class eDeferredPasses
	{
		Camera,
		WorldSpace,
		Albedo,
		Normal,
		Material,
		AmbientOcclusionAndCustom,
		Depth,
		Count
	};

	inline const char* EnumToString(const eDeferredPasses aPass)
	{
		switch (aPass)
		{
		case eDeferredPasses::Camera: return "Camera";
		case eDeferredPasses::WorldSpace: return "WorldSpace";
		case eDeferredPasses::Albedo: return "Albedo";
		case eDeferredPasses::Normal: return "Normal";
		case eDeferredPasses::Material: return "Material";
		case eDeferredPasses::AmbientOcclusionAndCustom: return "AmbientOcclusionAndCustom";
		case eDeferredPasses::Depth: return "Depth";
		default: return "Unknown";
		}
	}
#pragma endregion

#pragma region BlendStates
	enum class eBlendStates
	{
		Disabled,
		AlphaBlend,
		AdditiveBlend,
		TransparencyBlend,
		VFXBlend,
		Count,
	};

	inline const char* EnumToString(const eBlendStates aState)
	{
		switch (aState)
		{
		case eBlendStates::Disabled: return "Disabled";
		case eBlendStates::AlphaBlend: return "AlphaBlend";
		case eBlendStates::AdditiveBlend: return "AdditiveBlend";
		case eBlendStates::TransparencyBlend: return "TransparencyBlend";
		case eBlendStates::VFXBlend: return "VFXBlend";
		default: return "Unknown";
		}
	}

#pragma endregion

#pragma region Depth StencilStates
	enum class eDepthStencilStates
	{
		Write,
		ReadOnlyGreater,
		ReadOnlyLess,
		ReadOnlyEmpty,
		Always,
		ReadOnlyLessEqual,
		Count,
	};

	inline const char* EnumToString(const eDepthStencilStates aState)
	{
		switch (aState)
		{
		case eDepthStencilStates::Write: return "Write";
		case eDepthStencilStates::ReadOnlyGreater: return "ReadOnlyGreater";
		case eDepthStencilStates::ReadOnlyLess: return "ReadOnlyLess";
		case eDepthStencilStates::ReadOnlyEmpty: return "ReadOnlyEmpty";
		case eDepthStencilStates::Always: return "Always";
		case eDepthStencilStates::ReadOnlyLessEqual: return "ReadOnlyLessEqual";
		default: return "Unknown";
		}
	}

#pragma endregion

#pragma region RasterizerStates
	enum class eRasterizerStates
	{
		BackfaceCulling,
		FrontfaceCulling,
		NoCulling,
		Wireframe,
		AALines,
		Count,
	};

	inline const char* EnumToString(const eRasterizerStates aState)
	{
		switch (aState)
		{
		case eRasterizerStates::BackfaceCulling: return "BackfaceCulling";
		case eRasterizerStates::FrontfaceCulling: return "FrontfaceCulling";
		case eRasterizerStates::NoCulling: return "NoCulling";
		case eRasterizerStates::Wireframe: return "Wireframe";
		default: return "Unknown";
		}
	}

#pragma endregion

#pragma region RenderLayers
	enum class eRenderLayers : int
	{
		Back,
		Main,
		Front,
		UI,

		Count
	};

	inline const char* EnumToString(const eRenderLayers aLayer)
	{
		switch (aLayer)
		{
		case eRenderLayers::Back: return "Back";
		case eRenderLayers::Main: return "Main";
		case eRenderLayers::Front: return "Front";
		case eRenderLayers::UI: return "UI";
		default: return "Unknown";
		}
	}
#pragma endregion
}
