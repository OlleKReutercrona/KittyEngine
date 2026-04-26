#pragma once
#include <wrl/client.h>
#include <string>
struct ID3D11ShaderResourceView;
using Microsoft::WRL::ComPtr;

namespace KE
{
	constexpr int PBR_TEXTURES = 4; //colour, normal, material, fx

	enum class TextureType
	{
		Colour,
		Normal,
		Material,
		Effects,

		Count
	};

	enum class TextureFormat
	{
		Texture2D,
		Texture3D,
		TextureCube,

		Count
	};

	static const std::string TextureTypeStrings[(int)TextureType::Count + 1] = {
		"Colour",
		"Normal",
		"Material",
		"Effects",
		"Unknown",
	};

	static const char* EnumToString(TextureFormat aFormat)
	{
		switch (aFormat)
		{
		case TextureFormat::Texture2D: return "Texture2D";
		case TextureFormat::Texture3D: return "Texture3D";
		case TextureFormat::TextureCube: return "TextureCube";
		default: return "Unknown";
		}
	}


	struct Texture
	{
		struct
		{
			std::string myFilePath;
			std::string myFileName;
			TextureType myTextureType = TextureType::Count;

			TextureFormat myTextureFormat = TextureFormat::Texture2D;

			bool failed = false;

			int myWidth;
			int myHeight;
		} myMetadata = {};

		ComPtr<ID3D11ShaderResourceView> myShaderResourceView;

		//todo: maybe ifdef these out when relevant
	};

	struct Cubemap
	{
		struct
		{
			std::string myFileName;
			int myWidth;
			int myHeight;

			TextureFormat myTextureFormat = TextureFormat::TextureCube;
		} myMetadata = {};

		ComPtr<ID3D11ShaderResourceView> myShaderResourceView;
	};

	struct Material
	{
		std::string myName;
		bool myIsCustom = false;
		bool myIsData = false;
		Texture* myTextures[PBR_TEXTURES]; // :)
	};

	struct DataMaterial : public Material
	{
		Vector3f albedo;
		float metallic;
		float roughness;
		float emission;

		std::string albedoMap;
		std::string normalMap;
		std::string materialMap;
		std::string effectsMap;
	};


}
