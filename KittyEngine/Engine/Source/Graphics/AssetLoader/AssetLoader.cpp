#include "stdafx.h"
#include "AssetLoader.h"
#include <filesystem>
#include <iostream>

#include "Engine/Source/Graphics/Texture/TextureLoader.h"
#include "Engine/Source/Graphics/Graphics.h"
#include "Engine/Source/Utility/Timer.h"

void PrepareDirectory(std::string aDirectoryName)
{
	if (!std::filesystem::exists(aDirectoryName))
	{
		std::filesystem::create_directory(aDirectoryName);
	}
	else
	{
		std::filesystem::remove_all(aDirectoryName);
		std::filesystem::create_directory(aDirectoryName);
	}
}

void KE::AssetLoader::CopyAssets()
{
//#ifndef KITTYENGINE_SHIP

	Timer timer;

	if (!std::filesystem::exists("Data"))
	{
		std::filesystem::create_directory("Data");
	}

	PrepareDirectory("Data/EngineAssets");
	PrepareDirectory("Data/EditorAssets");
	PrepareDirectory("Data/ProjectAssets");

	
	constexpr auto options = std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing;

	if (std::filesystem::exists("../Engine/EngineAssets"))
	{
		Timer engineTimer;
		std::filesystem::copy("../Engine/EngineAssets",   "Data/EngineAssets", options);
		const float engineTime = engineTimer.UpdateDeltaTime();
	}

	if (std::filesystem::exists("../Engine/EngineAssets"))
	{
		Timer editorTimer;
		std::filesystem::copy("../Editor/EditorAssets", "Data/EditorAssets", options);
		const float editorTime = editorTimer.UpdateDeltaTime();
	}

	if (std::filesystem::exists("../Engine/EngineAssets"))
	{
		Timer projectTimer;
		std::filesystem::copy("../Project/ProjectAssets", "Data/ProjectAssets", options);
		const float projectTime = projectTimer.UpdateDeltaTime();
	}

	const float copyTime = timer.UpdateDeltaTime();
//#endif
}

void KE::AssetLoader::Load()
{
	myTextureThread = std::thread(&KE::AssetLoader::TextureLoad, this);
	myModelThread = std::thread(&KE::AssetLoader::ModelLoad, this);
	myShaderThread = std::thread(&KE::AssetLoader::ShaderLoad, this);
}

void KE::AssetLoader::TextureLoad()
{
	KE::Timer timer;

	KE::TextureLoader& texLoader = myGraphics->GetTextureLoader();
	for (auto& entry : std::filesystem::recursive_directory_iterator("Data"))
	{
		std::filesystem::path path = entry.path();
		std::filesystem::path fileName = entry.path().filename();

		if (
			fileName.extension() == ".dds" || 
			fileName.extension() == ".DDS" || 
			fileName.extension() == ".png" ||
			fileName.extension() == ".PNG"
		)
		{
			texLoader.GetTextureFromPath(path.string());
			std::cout << "TextureLoader thread loaded: " << path.string() << std::endl;
		}
	}

	std::cout << "TextureLoader thread finished in " << timer.UpdateDeltaTime() << " seconds (?)" << std::endl;
}

void KE::AssetLoader::ModelLoad()
{
	KE::Timer timer;

	KE::ModelLoader& modLoader = myGraphics->GetModelLoader();
	for (auto& entry : std::filesystem::recursive_directory_iterator("Data"))
	{
		std::filesystem::path path = entry.path();
		std::filesystem::path fileName = entry.path().filename();

		if (
			(fileName.extension() == ".fbx" ||
			fileName.extension() == ".FBX") &&
			fileName.string().find("SK") == std::string::npos &&
			fileName.string().find("AN") == std::string::npos
			)
		{
			std::string pathStr = path.string();
			//replace double backslash with single forward slash
			std::replace(pathStr.begin(), pathStr.end(), '\\', '/');
			
			modLoader.Load(pathStr);
			std::cout << "ModelLoader thread loaded: " << pathStr << std::endl;
		}
	}

	std::cout << "ModelLoader thread finished in " << timer.UpdateDeltaTime() << " seconds (?)" << std::endl;
}

void KE::AssetLoader::ShaderLoad()
{

}

KE::AssetLoader::AssetLoader(Graphics* aGraphics) : myGraphics(aGraphics)
{
	
}

KE::AssetLoader::~AssetLoader()
{
	if (!isInit) { return; }
	myTextureThread.join();
	myModelThread.join();
	myShaderThread.join();
}

void KE::AssetLoader::Init()
{
	isInit = true;
	Load();
}

void KE::AssetLoader::Update()
{
}
