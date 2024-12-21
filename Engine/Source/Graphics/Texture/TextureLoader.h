#pragma once
#include <d3d11.h>
#include <unordered_map>
#include <string>

#include "Texture.h"

namespace KE
{
	class RenderTarget;

	enum class CubemapProjectionType
	{
		Cross,
		Equirectangular,
		Sphere,


	};

	class TextureLoader
	{
		inline static std::string SUFFIXES[] = { "_c.dds", "_n.dds", "_m.dds", "_fx.dds" };
		inline static std::string FILETYPES[] = { "Colour", "Normal", "Material", "Effects" };

	private:
		std::unordered_map<std::string, Texture> loadedTextures;
		std::unordered_map<std::string, Material> loadedMaterials;
		std::unordered_map<std::string, Cubemap> loadedCubemaps;

		std::unordered_map<std::string, Material> customMaterials;

		std::unordered_map<std::string, Texture> loadedDataTextures;
		std::unordered_map<std::string, Texture> projectedCubemaps;
		std::unordered_map<std::string, DataMaterial> loadedDataMaterials;

		ID3D11Device* myDevice;
		ID3D11DeviceContext* myContext;
		
		inline static const std::string myDefaultTexture_c =  "Data/EngineAssets/KEDefault_c.dds"; // "Data/InternalAssets/KittyEnginePattern_c.dds"
		inline static const std::string myDefaultTexture_n =  "Data/EngineAssets/KEDefault_n.dds"; // "Data/InternalAssets/KittyEnginePattern_n.dds"
		inline static const std::string myDefaultTexture_m =  "Data/EngineAssets/KEDefault_m.dds"; // "Data/InternalAssets/KittyEnginePattern_m.dds"
		inline static const std::string myDefaultTexture_fx = "Data/EngineAssets/KEDefault_fx.dds"; // "Data/InternalAssets/KittyEnginePattern_m.dds"

	public:
		TextureLoader();
		~TextureLoader();

		void Clear();

		void Init(ID3D11Device* aDevice, ID3D11DeviceContext* aContext);

		bool SaveTextureToFile(
			const std::string& aOutputFilePath,
			ID3D11ShaderResourceView* aTexture,
			unsigned int aWidth,
			unsigned int aHeight,
			Vector2f aUVMin = Vector2f(0.0f, 0.0f),
			Vector2f aUVMax = Vector2f(1.0f, 1.0f)
		) const;

		Texture* GetDefaultTexture(int anIndex = 0);
		Material* GetDefaultMaterial();

		bool InitializeTexture(Texture* aTexture);

		Texture* GetTextureFromPath(
			const std::string& aFilePath,
			bool loggingEnabled = true,
			bool alwaysRegenerate = false
		);

		Texture* CreateTextureFromData(
			const std::string& aTextureName,
			const unsigned char* aData,
			const int aWidth,
			const int aHeight,
			bool alwaysRegenerate = false
		);

		Material* GetMaterialFromPath(
			const std::string& aFilePath);

		Cubemap* GetCubemapFromPath(
			const std::string& aFilePath);
		
		Material* GetCustomMaterial(
			const std::string& aAlbedoPath = myDefaultTexture_c,
			const std::string& aNormalPath = myDefaultTexture_n,
			const std::string& aMaterialPath = myDefaultTexture_m,
			const std::string& aFXPath = myDefaultTexture_fx
		);

		DataMaterial* CreateDataMaterialTextures(const DataMaterial& aDataMaterial);

		DataMaterial* LoadDataMaterial(
			const std::string& aFilePath,
			bool alwaysRegenerate = false
		);

		void SaveDataMaterial(
			const std::string& aFilePath,
			const DataMaterial& aDataMaterial
		);

		DataMaterial* GetDataMaterial(
			const DataMaterial& aDataMaterial
		);

		void RenderThumbnail(KE::Material* aMaterial, const std::string& aThumbnailPath);

		Texture* ProjectCubemap(const std::string& aCubemapPath, CubemapProjectionType aProjection);

		inline std::unordered_map<std::string, Texture>&  GetLoadedTextures()  { return loadedTextures;  }
		inline std::unordered_map<std::string, Material>& GetLoadedMaterials() { return loadedMaterials; }
		
		bool ReloadTexture(Texture* aTexture);

		inline void ReloadAllTextures()
		{
			for (auto& texture : loadedTextures)
			{
				if (texture.second.myMetadata.myFilePath.find("KEDefault") != std::string::npos)
				{
					continue;
				}
				ReloadTexture(&texture.second);
			}
		}
	};
}