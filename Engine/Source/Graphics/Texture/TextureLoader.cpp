#include "stdafx.h"
#include "TextureLoader.h"

#include <d3d11.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <External/Include/dxtex/DirectXTex.h>
#include <External/Include/dxtex/DDSTextureLoader11.h>

#include "Engine/Source/Utility/Logging.h"

#include <fstream>
#include <filesystem>
#include <iostream>

#include "dxtex/DDSTextureLoader11.h"
#include "Graphics/FullscreenAsset.h"
#include "Graphics/Graphics.h"
#include "nlohmann/json.hpp"
#define TEXTUREPATH "\\Data\\Assets\\Textures\\"

namespace KE
{
	TextureLoader::TextureLoader()
	{

	}

	TextureLoader::~TextureLoader()
	{

	}

	void TextureLoader::Clear()
	{
		for (auto& texture : loadedTextures)
		{
			texture.second.myShaderResourceView = nullptr;
		}

		for (auto& material : loadedMaterials)
		{
			material.second.myTextures[0] = nullptr;
			material.second.myTextures[1] = nullptr;
			material.second.myTextures[2] = nullptr;
			material.second.myTextures[3] = nullptr;
		}

		for (auto& cubemap : loadedCubemaps)
		{
			cubemap.second.myShaderResourceView = nullptr;
		}
	}

	void TextureLoader::Init(ID3D11Device* aDevice, ID3D11DeviceContext* aContext)
	{
		myDevice = aDevice;
		myContext = aContext;

		GetTextureFromPath(myDefaultTexture_c);
		GetTextureFromPath(myDefaultTexture_n);
		GetTextureFromPath(myDefaultTexture_m);
		GetTextureFromPath(myDefaultTexture_fx);

		loadedMaterials["DEFAULT"] = Material();
		auto& defaultMaterial = loadedMaterials["DEFAULT"];
		for (int i = 0; i < 4; i++)
		{
			defaultMaterial.myTextures[i] = GetDefaultTexture(i);
		}
	}

	bool TextureLoader::SaveTextureToFile(
		const std::string& aOutputFilePath,
		ID3D11ShaderResourceView* aTexture,
		unsigned int aWidth,
		unsigned int aHeight,
		Vector2f aUVMin,
		Vector2f aUVMax
	) const
	{
		DirectX::Image img;
		img.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		img.width = aWidth;
		img.height = aHeight;

		img.rowPitch = 4 * aWidth;
		img.slicePitch = 4 * aWidth * aHeight;

		img.pixels = new uint8_t[aWidth * aHeight * 4];

		ComPtr<ID3D11Texture2D> stagingTexture = nullptr;
		{
			//create a staging texture
			D3D11_TEXTURE2D_DESC desc;
			desc.Width = aWidth;
			desc.Height = aHeight;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_STAGING;
			desc.BindFlags = 0;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			desc.MiscFlags = 0;


			myDevice->CreateTexture2D(&desc, nullptr, stagingTexture.ReleaseAndGetAddressOf());
		}

		ComPtr<ID3D11Texture2D> intermediateTexture = nullptr;
		ComPtr<ID3D11RenderTargetView> intermediateRT = nullptr;
		{
			D3D11_TEXTURE2D_DESC desc;
			desc.Width = aWidth;
			desc.Height = aHeight;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			desc.CPUAccessFlags = 0;
			desc.MiscFlags = 0;

			myDevice->CreateTexture2D(&desc, nullptr, intermediateTexture.ReleaseAndGetAddressOf());
			myDevice->CreateRenderTargetView(intermediateTexture.Get(), nullptr, intermediateRT.ReleaseAndGetAddressOf());
		}

		myContext->OMSetRenderTargets(1, intermediateRT.GetAddressOf(), nullptr);

		auto* gfx = KE_GLOBAL::blackboard.Get<Graphics>();
		static bool fsInit = false;
		static FullscreenAsset fs;
		if (!fsInit)
		{
			fs.Init(gfx);
			fsInit = true;
		}
		fs.Render(
			gfx,
			aTexture,
			gfx->GetShaderLoader().GetVertexShader(SHADER_LOAD_PATH "FullscreenAsset_VS.cso"),
			gfx->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH "FullscreenAsset_PS.cso"),
			aUVMin,
			aUVMax
		);

		//null rtv
		ID3D11RenderTargetView* nullRTV[1] = { nullptr};
		myContext->OMSetRenderTargets(1, nullRTV, nullptr);

		//copy the texture to the staging texture
		myContext->CopyResource(stagingTexture.Get(), intermediateTexture.Get());
		myContext->Flush();

		//copy the pixels from the staging texture (a D3D11Texture2D) to img.pixels
		D3D11_MAPPED_SUBRESOURCE mappedResource = {0};

		myContext->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
		memcpy(img.pixels, mappedResource.pData, img.slicePitch);
		myContext->Unmap(stagingTexture.Get(), 0);


		std::wstring filePathW = std::wstring(aOutputFilePath.begin(), aOutputFilePath.end());

		//check if file is read only
		if (std::filesystem::exists(aOutputFilePath) && 
			std::filesystem::is_regular_file(aOutputFilePath) && 
			std::filesystem::status(aOutputFilePath).permissions() == std::filesystem::perms::_File_attribute_readonly)
		{
			std::filesystem::permissions(aOutputFilePath, std::filesystem::perms::owner_write, std::filesystem::perm_options::replace);
		}

		HRESULT hr = DirectX::SaveToDDSFile(img, DirectX::DDS_FLAGS_NONE, filePathW.c_str());
		if (FAILED(hr))
		{
			return false;
		}


		//make sure we don't leak memory
		delete[] img.pixels;
		stagingTexture->Release();
		mappedResource.pData = nullptr;

		return true;

	}

	Texture* TextureLoader::GetDefaultTexture(int anIndex)
	{
		switch (anIndex)
		{
		case 0:
			return &loadedTextures[myDefaultTexture_c];
		case 1:
			return &loadedTextures[myDefaultTexture_n];
		case 2:
			return &loadedTextures[myDefaultTexture_m];
		case 3:
			return &loadedTextures[myDefaultTexture_fx];
		default:
			//KE_WARNING("Invalid index for default texture request: %i", anIndex);
			return &loadedTextures[myDefaultTexture_c];
		}
	}

	Material* TextureLoader::GetDefaultMaterial()
	{
		return GetMaterialFromPath("DEFAULT");
	}

	bool TextureLoader::InitializeTexture(Texture* aTexture)
	{
		HRESULT hr;

		std::string filePath = aTexture->myMetadata.myFilePath;

		//iterate through SUFFIXES to find the correct suffix
		for (int i = 0; i < PBR_TEXTURES; ++i)
		{
			if (filePath.find(SUFFIXES[i]) != std::string::npos)
			{
				aTexture->myMetadata.myTextureType = (TextureType)i;
				break;
			}
		}

		//const size_t cSize = strlen(aTexture->myMetadata.myFilePath.c_str());
		//wchar_t* wcPath = new wchar_t[cSize];
		//mbstowcs(wcPath, aTexture->myMetadata.myFilePath.c_str(), cSize);

		std::wstring wPath(
			filePath.begin(),
			filePath.end());

		if (
			filePath.find(".dds") != std::string::npos ||
			filePath.find(".DDS") != std::string::npos
			)
		{
			//ID3D11Resource* texture;
			//ComPtr<ID3D11Texture2D> texture2D;
			ID3D11Texture2D* texture2D;
			D3D11_TEXTURE2D_DESC desc;

			hr = DirectX::CreateDDSTextureFromFileEx(
				myDevice,
				nullptr,
				wPath.c_str(),
				0,
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_SHADER_RESOURCE, /*D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET*/
				0,
				0,
				DirectX::DDS_LOADER_IGNORE_SRGB,
				(ID3D11Resource**)&texture2D,
				aTexture->myShaderResourceView.ReleaseAndGetAddressOf(),
				nullptr);

			if (FAILED(hr))
			{
				return false;
			}

			texture2D->GetDesc(&desc);


			if (desc.ArraySize > 1)
			{
				std::cout << "Array size was greater than 0 for "<< filePath << std::endl;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			aTexture->myShaderResourceView->GetDesc(&srvDesc);

			if (srvDesc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURECUBE)
			{
				aTexture->myMetadata.myTextureFormat = TextureFormat::TextureCube;
			}


			aTexture->myMetadata.myWidth = desc.Width;
			aTexture->myMetadata.myHeight = desc.Height;
		}
		else
		{
			DirectX::ScratchImage image;
			DirectX::TexMetadata metadata;

			hr = LoadFromWICFile(
				wPath.c_str(),
				DirectX::WIC_FLAGS_IGNORE_SRGB,
				&metadata,
				image
			);


			if (FAILED(hr))
			{
				return false;
			}

			aTexture->myMetadata.myWidth = (int)metadata.width;
			aTexture->myMetadata.myHeight = (int)metadata.height;

			hr = CreateShaderResourceView(
				myDevice,
				image.GetImages(),
				image.GetImageCount(),
				metadata,
				aTexture->myShaderResourceView.ReleaseAndGetAddressOf()
			);
		}





		//DirectX::ScratchImage image;
		//DirectX::TexMetadata metadata;

		//HRESULT hr;

		//std::string filePath = aTexture->myMetadata.myFilePath;

		////iterate through SUFFIXES to find the correct suffix
		//for (int i = 0; i < PBR_TEXTURES; ++i)
		//{
		//	if (filePath.find(SUFFIXES[i]) != std::string::npos)
		//	{
		//		aTexture->myMetadata.myTextureType = (TextureType)i;
		//		break;
		//	}
		//}

		//if (
		//	filePath.find(".dds") != std::string::npos ||
		//	filePath.find(".DDS") != std::string::npos
		//	)
		//{
		//	hr = LoadFromDDSFile(std::wstring(
		//		filePath.begin(),
		//		filePath.end()).c_str(),
		//		DirectX::DDS_FLAGS_NONE,
		//		//DirectX::DDS_FLAGS_FORCE_RGB,
		//		&metadata,
		//		image
		//	);
		//}
		//else
		//{
		//	hr = LoadFromWICFile(std::wstring(
		//		filePath.begin(),
		//		filePath.end()).c_str(),
		//		DirectX::WIC_FLAGS_NONE,
		//		&metadata,
		//		image
		//	);
		//}

		//if (FAILED(hr))
		//{
		//	//KE_ERROR("Failed to load texture %s", aFilePath.c_str()); // TODO: Add more info to this error message.

		//	return false;//return GetDefaultTexture();
		//}

		//hr = CreateShaderResourceView(
		//	myDevice,
		//	image.GetImages(),
		//	image.GetImageCount(),
		//	metadata,
		//	&aTexture->myShaderResourceView
		//);

		//if (FAILED(hr))
		//{
		//	//KE_ERROR("Failed to create shader resource view."); // TODO: Add more info to this error message.
		//	//return GetDefaultTexture();
		//	return false;
		//}

		//aTexture->myMetadata.myWidth = (int)metadata.width;
		//aTexture->myMetadata.myHeight = (int)metadata.height;

		//if (metadata.arraySize > 1)
		//{
		//	KE_ERROR("Texture array size is greater than 1. Load as a cubemap instead?.");
		//	return false;
		//}

		return true;
	}

	Texture* TextureLoader::GetTextureFromPath(const std::string& aFilePath, bool loggingEnabled, bool alwaysRegenerate)
	{
		//replace backslashes with forward slashes
		std::string reformattedString = std::filesystem::path(aFilePath).string();

		if (loadedTextures.find(reformattedString) != loadedTextures.end())
		{
			if (!alwaysRegenerate)
			{
				return &loadedTextures[reformattedString];
			}
		}

		//loadedTextures[reformattedString] = Texture();
		Texture* workingTexture = &loadedTextures[reformattedString];

		workingTexture->myMetadata.myFilePath = reformattedString;
		workingTexture->myMetadata.myFileName = reformattedString.substr(reformattedString.find_last_of("/") + 1);
		workingTexture->myMetadata.failed = false;

		if (!InitializeTexture(workingTexture))
		{
			if (loggingEnabled)
			{
				KE_LOG_CHANNEL("TextureLoader", "Failed to load texture %s", reformattedString.c_str()); // TODO: Add more info to this error message.
			}

			

			workingTexture->myShaderResourceView = GetDefaultTexture(
				(int)workingTexture->myMetadata.myTextureType
			)->myShaderResourceView;
			workingTexture->myMetadata.failed = true;
		}

		return workingTexture;
	}

	Texture* TextureLoader::CreateTextureFromData(
		const std::string& aTextureName,
		const unsigned char* aData,
		const int aWidth,
		const int aHeight,
		bool alwaysRegenerate
	)
	{
		if (loadedTextures.find(aTextureName) != loadedTextures.end())
		{
			if (!alwaysRegenerate)
			{
				return &loadedTextures[aTextureName];
			}
		}

		//loadedTextures[aTextureName] = Texture();
		auto& workingTexture = loadedTextures[aTextureName];


		D3D11_SUBRESOURCE_DATA initData = { 0 };
		initData.pSysMem = (const void*)aData;
		initData.SysMemPitch = aWidth * 4;
		initData.SysMemSlicePitch = aHeight * aWidth * 4;

		D3D11_TEXTURE2D_DESC desc;
		desc.Width = aWidth;
		desc.Height = aHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		ComPtr<ID3D11Texture2D> tex = nullptr;
		HRESULT hr = myDevice->CreateTexture2D(&desc, &initData, &tex);
		if (FAILED(hr))
		{
			return nullptr;
		}

		hr = myDevice->CreateShaderResourceView(tex.Get(), nullptr, workingTexture.myShaderResourceView.ReleaseAndGetAddressOf());

		workingTexture.myMetadata.myWidth = aWidth;
		workingTexture.myMetadata.myHeight = aHeight;
		workingTexture.myMetadata.myTextureType = TextureType::Colour;
		workingTexture.myMetadata.myFilePath = aTextureName;
		workingTexture.myMetadata.myFileName = aTextureName;
		workingTexture.myMetadata.failed = false;

		return &workingTexture;
	}

	Material* TextureLoader::GetMaterialFromPath(const std::string& aFilePath)
	{
		if (loadedMaterials.find(aFilePath) != loadedMaterials.end())
		{
			return &loadedMaterials[aFilePath];
		}

		loadedMaterials[aFilePath] = Material();
		Material* workingMaterial = &loadedMaterials[aFilePath];
		workingMaterial->myName = aFilePath;
		
		// Build the texture path by adding suffix to the FBX file name
		{
			for (int i = 0; i < PBR_TEXTURES; ++i)
			{
				std::string texturePath = "Data/Assets/Materials/" + aFilePath + SUFFIXES[i];

				workingMaterial->myTextures[i] = GetTextureFromPath(texturePath, i < 3);
				if (workingMaterial->myTextures[i] == GetDefaultTexture())
				{
					//KE_ERROR("Failed to load texture %s", texturePath.c_str()); // TODO: Add more info to this error message.
					workingMaterial->myTextures[i] = GetDefaultTexture(i);
				}
			}
		}

		return workingMaterial;
	}
	
	Cubemap* TextureLoader::GetCubemapFromPath(const std::string& aFilePath)
	{
		if (aFilePath.find(".dds") == std::string::npos)
		{
			KE_ERROR("Cubemap must be a .dds file. %s", aFilePath.c_str());
			return nullptr;
		}
		
		if (loadedCubemaps.find(aFilePath) != loadedCubemaps.end())
		{
			return &loadedCubemaps[aFilePath];
		}

		loadedCubemaps[aFilePath] = Cubemap();
		Cubemap* workingCubemap = &loadedCubemaps[aFilePath];
		
		DirectX::ScratchImage image;
		DirectX::TexMetadata metadata;
		
		HRESULT hr;
		
		hr = LoadFromDDSFile(std::wstring(
			aFilePath.begin(),
			aFilePath.end()).c_str(),
			DirectX::DDS_FLAGS_NONE,
			&metadata,
			image
		);

		if (FAILED(hr))
		{
			KE_ERROR("Failed to load cubemap %s", aFilePath.c_str()); // TODO: Add more info to this error message.
			return nullptr;
		}
		
		hr = CreateShaderResourceView(
			myDevice,
			image.GetImages(),
			image.GetImageCount(),
			metadata,
			&workingCubemap->myShaderResourceView
		);
		
		if (FAILED(hr))
		{
			KE_ERROR("Failed to create shader resource view."); // TODO: Add more info to this error message.
			return nullptr;
		}

		workingCubemap->myMetadata.myFileName = aFilePath;
		workingCubemap->myMetadata.myWidth = (int)metadata.width;
		workingCubemap->myMetadata.myHeight = (int)metadata.height;
		workingCubemap->myMetadata.myTextureFormat = TextureFormat::TextureCube;

		return workingCubemap;
	}

	Material* TextureLoader::GetCustomMaterial(const std::string& aAlbedoPath, const std::string& aNormalPath, const std::string& aMaterialPath, const std::string& aFXPath)
	{
		const std::string aName = aAlbedoPath + aNormalPath + aMaterialPath + aFXPath;
		if (customMaterials.find(aName) != customMaterials.end())
		{
			return &customMaterials[aName];
		}
		
		customMaterials[aName] = Material();
		Material* workingMaterial = &customMaterials[aName];
		workingMaterial->myName = "Custom Material";

		workingMaterial->myTextures[0] = GetTextureFromPath(aAlbedoPath);
		workingMaterial->myTextures[1] = GetTextureFromPath(aNormalPath);
		workingMaterial->myTextures[2] = GetTextureFromPath(aMaterialPath);
		workingMaterial->myTextures[3] = GetTextureFromPath(aFXPath);		
		
		return workingMaterial;
	}

	DataMaterial* TextureLoader::CreateDataMaterialTextures(const DataMaterial& aDataMaterial)
	{
		//make the actual material here
		loadedDataMaterials[aDataMaterial.myName] = aDataMaterial;
		auto& workingDataMaterial = loadedDataMaterials[aDataMaterial.myName];
		

		std::string albedoTexName = workingDataMaterial.myName + "_c.dds";
		std::string normalTexName = workingDataMaterial.myName + "_n.dds"; // not sure we need this.
		std::string materialTexName = workingDataMaterial.myName + "_m.dds";
		std::string fxTexName = workingDataMaterial.myName + "_fx.dds";

		constexpr unsigned int textureDimensions = 8;
		unsigned char texureDataArray[textureDimensions * textureDimensions * 4];
		//heap allocate instead
		//unsigned char* texureDataArray = new unsigned char[textureDimensions * textureDimensions * 4];

			
		{//ALBEO MAP

			if (aDataMaterial.albedoMap.empty())
			{
				//albedo tex
				for (int i = 0; i < textureDimensions * textureDimensions * 4; i += 4)
				{
					texureDataArray[i + 0] = (unsigned char)(workingDataMaterial.albedo.x * 255);
					texureDataArray[i + 1] = (unsigned char)(workingDataMaterial.albedo.y * 255);
					texureDataArray[i + 2] = (unsigned char)(workingDataMaterial.albedo.z * 255);
					texureDataArray[i + 3] = 255;
				}
				workingDataMaterial.myTextures[0] = CreateTextureFromData(
					albedoTexName,
					texureDataArray,
					textureDimensions, 
					textureDimensions,
					true
				);
			}
			else
			{
				workingDataMaterial.myTextures[0] = GetTextureFromPath(aDataMaterial.albedoMap);
			}
		}

		{//NORMAL MAP

			bool found = false;
			if (!aDataMaterial.normalMap.empty())
			{
				workingDataMaterial.myTextures[1] = GetTextureFromPath(aDataMaterial.normalMap);
				found = !workingDataMaterial.myTextures[1]->myMetadata.failed;
			}

			if (!found)
			{
				//normal tex
				for (int i = 0; i < textureDimensions * textureDimensions * 4; i += 4)
				{
					texureDataArray[i + 0] = 128;
					texureDataArray[i + 1] = 128;
					texureDataArray[i + 2] = 0  ;
					texureDataArray[i + 3] = 255;
				}
				workingDataMaterial.myTextures[1] = CreateTextureFromData(
					normalTexName,
					texureDataArray,
					textureDimensions, 
					textureDimensions,
					true
				);
			}
		}

		{//MATERIAL MAP

			bool found = false;
			if (!aDataMaterial.materialMap.empty())
			{
				workingDataMaterial.myTextures[2] = GetTextureFromPath(aDataMaterial.materialMap);
				found = !workingDataMaterial.myTextures[2]->myMetadata.failed;
			}

			if (!found)
			{
				//material tex, ao (unused for now) in r, roughness in g, metallic in b
				for (int i = 0; i < textureDimensions * textureDimensions * 4; i += 4)
				{
					texureDataArray[i + 0] = 0;
					texureDataArray[i + 1] = (unsigned char)(workingDataMaterial.roughness * 255);
					texureDataArray[i + 2] = (unsigned char)(workingDataMaterial.metallic * 255);
					texureDataArray[i + 3] = 255;
				}
				workingDataMaterial.myTextures[2] = CreateTextureFromData(
					materialTexName,
					texureDataArray,
					textureDimensions, 
					textureDimensions,
					true
				);
			}
		}

		{//FX MAP
			bool found = false;
			if (!aDataMaterial.effectsMap.empty())
			{
				workingDataMaterial.myTextures[3] = GetTextureFromPath(aDataMaterial.effectsMap);
				found = !workingDataMaterial.myTextures[3]->myMetadata.failed;
			}

			if (!found)
			{
				//fx tex, emission in r
				for (int i = 0; i < textureDimensions * textureDimensions * 4; i += 4)
				{
					texureDataArray[i + 0] = (unsigned char)(workingDataMaterial.emission * 255);
					texureDataArray[i + 1] = 0 ;
					texureDataArray[i + 2] = 0 ;
					texureDataArray[i + 3] = 255;
				}
				workingDataMaterial.myTextures[3] = CreateTextureFromData(
					fxTexName,
					texureDataArray,
					textureDimensions, 
					textureDimensions,
					true
				);
			}
		}

		return &workingDataMaterial;
	}

	DataMaterial* TextureLoader::LoadDataMaterial(const std::string& aFilePath, bool alwaysRegenerate)
	{
		if (!alwaysRegenerate && loadedDataMaterials.find(aFilePath) != loadedDataMaterials.end())
		{
			return &loadedDataMaterials[aFilePath];
		}

		using nlohmann::json;

		json jsonMaterial;
		std::ifstream file(aFilePath);
		if(!file.is_open())
		{
			KE_WARNING("DataMaterial Failed to load: %s", aFilePath.c_str());
			return nullptr;
		}

		DataMaterial& workingDataMaterial = loadedDataMaterials[aFilePath];

		file >> jsonMaterial;

		workingDataMaterial.myName = jsonMaterial["name"].get<std::string>();
		workingDataMaterial.albedo = Vector3f(
			jsonMaterial["albedo"]["r"].get<float>(),
			jsonMaterial["albedo"]["g"].get<float>(),
			jsonMaterial["albedo"]["b"].get<float>()
		);

		workingDataMaterial.metallic = jsonMaterial["metallic"].get<float>();
		workingDataMaterial.roughness = jsonMaterial["roughness"].get<float>();
		workingDataMaterial.emission = jsonMaterial["emission"].get<float>();

		//
		CreateDataMaterialTextures(workingDataMaterial);

		return &workingDataMaterial;
	}

	void TextureLoader::SaveDataMaterial(const std::string& aFilePath, const DataMaterial& aDataMaterial)
	{
		using nlohmann::json; 

		json jsonMaterial;

		jsonMaterial["name"] = aDataMaterial.myName;

		json jAlbedo;
		jAlbedo["r"] = aDataMaterial.albedo.x;
		jAlbedo["g"] = aDataMaterial.albedo.y;
		jAlbedo["b"] = aDataMaterial.albedo.z;
		jsonMaterial["albedo"] = jAlbedo;

		jsonMaterial["metallic"] = aDataMaterial.metallic;
		jsonMaterial["roughness"] = aDataMaterial.roughness;
		jsonMaterial["emission"] = aDataMaterial.emission;

		//write the file
		std::ofstream file;
		file.open(aFilePath);
		if (file.is_open())
		{
			file << jsonMaterial.dump(4);
			file.close();
		}
	}

	DataMaterial* TextureLoader::GetDataMaterial(const DataMaterial& aDataMaterial)
	{
		const std::string aName = aDataMaterial.myName;
		if (loadedDataMaterials.find(aName) != loadedDataMaterials.end())
		{
			return &loadedDataMaterials[aName];
		}

		loadedDataMaterials[aName] = aDataMaterial;
		CreateDataMaterialTextures(aDataMaterial);

		return &loadedDataMaterials[aName];
	}

	void TextureLoader::RenderThumbnail(KE::Material* aMaterial, const std::string& aThumbnailPath)
	{
		KE::ModelData modelData;
		Transform t;
		KE::Graphics* gfx = KE_GLOBAL::blackboard.Get<KE::Graphics>();

		auto& modelLoader = gfx->GetModelLoader();
		auto& shaderLoader = gfx->GetShaderLoader();
		auto& textureLoader = gfx->GetTextureLoader();
		auto* defaultRenderer = gfx->GetDefaultRenderer();

		modelData.myMeshList = &modelLoader.Load("Data/EngineAssets/SmoothSphere.fbx");

		modelData.myRenderResources.emplace_back();

		modelData.myRenderResources[0].myVertexShader = shaderLoader.GetVertexShader(SHADER_LOAD_PATH "Model_Deferred_VS.cso");
		modelData.myRenderResources[0].myPixelShader  = shaderLoader.GetPixelShader(SHADER_LOAD_PATH "Model_Deferred_PS.cso");
		modelData.myRenderResources[0].myMaterial = aMaterial;

		modelData.myTransform = &t.GetMatrix();
		t.SetScale({ 0.65f,0.65f,0.65f });

		const int previewSize = 512;

		KE::Camera renderCamera;
		renderCamera.SetPerspective(previewSize, previewSize, 90.0f * KE::DegToRadImmediate, 0.01f, 10.f);

		static float angle = 0.0f;
		//angle += 10.0f * KE_GLOBAL::deltaTime;

		renderCamera.transform.SetRotation({0.0f ,angle * KE::DegToRadImmediate, 0.0f});
		renderCamera.transform.TranslateLocal({ 0.0f, 0.0f, -1.0f });
		//Vector3f horizAngle = { sin(angle * KE::DegToRadImmediate), 0.0f, cos(angle * KE::DegToRadImmediate)};
		//renderCamera.transform.SetPosition(horizAngle * 1.0f);

		KE::BasicRenderInput in;
		in.viewMatrix = renderCamera.GetViewMatrix();
		in.projectionMatrix = renderCamera.GetProjectionMatrix();

		auto& buf = gfx->GetSecondaryGBuffer();
		buf.ClearTextures(gfx->GetContext().Get(), true);
		buf.SetAsActiveTarget(gfx->GetContext().Get(), buf.GetDepthStencilView());

		Transform skyT;
		KE::ModelData skybox; 
		skybox.myTransform = &skyT.GetMatrix();
		skybox.myIsInstanced = false;
		skybox.myMeshList = &modelLoader.Load("Data/EngineAssets/Cube.fbx");
		auto& rr = skybox.myRenderResources.emplace_back();
		rr.myMaterial = textureLoader.GetDefaultMaterial();
		rr.myVertexShader = shaderLoader.GetVertexShader(SHADER_LOAD_PATH "Skybox_VS.cso");
		rr.myPixelShader = shaderLoader.GetPixelShader(SHADER_LOAD_PATH "Skybox_Preview_PS.cso");

		auto rX = gfx->GetRenderWidth();
		auto rY = gfx->GetRenderHeight();

		gfx->SetViewport(512, 512);

		KE::BasicRenderInput skyboxRenderIn;
		gfx->SetDepthStencilState(KE::eDepthStencilStates::ReadOnlyLessEqual);
		gfx->SetRasterizerState(KE::eRasterizerStates::FrontfaceCulling);

		skyboxRenderIn.viewMatrix = renderCamera.GetViewMatrix();
		skyboxRenderIn.projectionMatrix = renderCamera.GetProjectionMatrix();
		defaultRenderer->RenderModel(skyboxRenderIn, skybox);
		gfx->SetDepthStencilState(KE::eDepthStencilStates::Write);
		gfx->SetRasterizerState(KE::eRasterizerStates::BackfaceCulling);

		defaultRenderer->RenderModel(in, modelData);
		//gfx->GetDeferredLightManager().PrepareShadowPass(gfx);
		//defaultRenderer->RenderModel(in, modelData);

		KE::RenderTarget* rt = gfx->GetRenderTarget(9);
		float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		rt->Clear(clearColor, true);
		rt->MakeActive(false);


		KE::Cubemap* myCubemap = textureLoader.GetCubemapFromPath("Data/EngineAssets/CubeMap_Skansen.dds");
		if (myCubemap) { gfx->GetContext()->PSSetShaderResources(15, 1, myCubemap->myShaderResourceView.GetAddressOf()); }
		buf.SetAllAsResources(gfx->GetContext().Get(), 0u);
		gfx->GetDeferredLightManager().Render(gfx);


		Vector2f uvMin = { 0.0f, 0.0f };
		Vector2f uvMax = { 512.0f / 4096.0f, 512.0f / 4096.0f};

		textureLoader.SaveTextureToFile(aThumbnailPath, rt->GetShaderResourceView(), 512, 512, uvMin, uvMax);

		gfx->SetViewport(rX, rY);
	}

	Texture* TextureLoader::ProjectCubemap(const std::string& aCubemapPath, CubemapProjectionType aProjection)
	{
		if (projectedCubemaps.find(aCubemapPath) != projectedCubemaps.end())
		{
			return &projectedCubemaps[aCubemapPath];
		}

		KE::Graphics* gfx = KE_GLOBAL::blackboard.Get<KE::Graphics>();

		switch (aProjection)
		{
			case CubemapProjectionType::Equirectangular:
			{
				Cubemap* map = GetCubemapFromPath(aCubemapPath);

				KE::RenderTarget* rt = gfx->GetRenderTarget(9);
				rt->Resize(4000, 3000);
				gfx->SetViewport(4000, 3000);

				float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
				rt->Clear(clearColor, true);
				rt->MakeActive(false);

				gfx->GetFullscreenAsset()->Render(
					gfx,
						map->myShaderResourceView.Get(),
						gfx->GetShaderLoader().GetVertexShader(SHADER_LOAD_PATH "FullscreenAsset_VS.cso"),
						gfx->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH "CubemapProjection_PS.cso"),
						{ 0.0f, 0.0f },
						{ 1.0f, 1.0f }
				);

				//unbind slot 0
				ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
				gfx->GetContext()->PSSetShaderResources(0, 1, nullSRV);

				Vector2f uvMin = { 0.0f, 0.0f };
				Vector2f uvMax = { 1.0f, 1.0f };

				gfx->SetViewport(1920, 1080);


				SaveTextureToFile("testOut.dds", rt->GetShaderResourceView(), 512, 512, uvMin, uvMax);

				break;
			}
			case CubemapProjectionType::Cross:
			{

				break;
			}
			case CubemapProjectionType::Sphere:
			{

				break;
			}
			default: break;
		}
		return nullptr;
	}

	bool TextureLoader::ReloadTexture(Texture* aTexture)
	{
		if (aTexture == nullptr)
		{
			KE_LOG_CHANNEL("TextureLoader", "ReloadTexture called with a nullptr texture.");
			return false;
		}

		return InitializeTexture(aTexture);
	}
}
