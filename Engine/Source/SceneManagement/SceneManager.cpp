#include "stdafx.h"
#include "SceneManager.h"
#pragma message("KITTY ENGIIINEEE")
#include <filesystem>

#include "Windows/Window.h"
#include "Engine/Source/Utility/DebugTimeLogger.h"
#include "Editor/Source/ImGui/ImGuiHandler.h"
#include "Engine/Source/Audio/GlobalAudio.h"
#include "Engine/Source/Utility/Randomizer.h"
#include "Engine/Source/Utility/StringUtils.h"

#include <External\Include\nlohmann\json.hpp>

#include "Project/Source/GameEvents/GameEvents.h"
KE::SceneManager::SceneManager()
{
	myLevelImporter = std::make_unique<LevelImporter>();
}

KE::SceneManager::~SceneManager()
{
	OnDestroy();
}

void KE::SceneManager::Init(Window& aWindow)
{
	KE_GLOBAL::blackboard.Register("sceneManager", this);

	OnInit();

	myWindow = &aWindow;

	myScenes.reserve(64);

	myLevelImporter->Init(&myWindow->GetGraphics(), this);

	// This should be exported
	myWindow->GetGraphics().AssignCubemap("Data/EngineAssets/CubeMap_Skansen.dds");

	std::string skyboxDir("Data/Assets/Materials/Skyboxes/");

	if (std::filesystem::is_directory(skyboxDir))
	{
		std::string defaultSkybox;
		defaultSkybox = skyboxDir + "t_defaultSkybox.dds";

		if (std::filesystem::exists(defaultSkybox))
		{
			//for now, not sure what skybox we will even use?
			myWindow->GetGraphics().AssignSkybox("Data/Assets/Materials/Skyboxes/t_defaultSkybox.dds");
		}
		else
		{
			//for now, not sure what skybox we will even use?
			myWindow->GetGraphics().AssignSkybox("Data/Assets/Materials/Skyboxes/t_spaceSkybox.dds");
		}
	}



	myDebugRenderer = &myWindow->GetGraphics().GetDebugRenderer();

	myPrefabHandler.Init(this);

	LoadBuildSettings();


	std::string path = "Data/Levels/";
	std::string levelStartInfoFile = "Data/Settings/LevelSettings.json";

	nlohmann::json levelStartInfo;
	std::string customStartLevel = "";

	std::ifstream file(levelStartInfoFile);

	if (file.is_open())
	{
		file >> levelStartInfo;
		customStartLevel = levelStartInfo["start_level_name"];
		customStartLevel += ".json";
	}
	file.close();

	if (!std::filesystem::exists(path))
	{

	}

	for (const auto& entry : std::filesystem::directory_iterator(path))
	{
		std::string levelName = entry.path().string();

		levelName = levelName.substr(levelName.find_last_of("/") + 1);

		if (levelName == "Levels.json") { continue; }

		int buildIndex = CheckSceneForBuildMatch(levelName, static_cast<int>(myScenes.size()));

		myScenes.emplace_back(static_cast<int>(myScenes.size()), levelName, buildIndex);

	}

	P8::LevelSelectDataEvent levelSelectDataEvent;

	for (auto& scene : myScenes)
	{
		if (scene.buildIndex > 3)
		{
			levelSelectDataEvent.myLevelData.push_back({ scene.sceneName, scene.buildIndex });
		}
	}
	
	for (auto& scene : myScenes)
	{
		if (levelStartInfoFile != "" && scene.sceneName == customStartLevel)
		{
			myStartupScene = &scene;
			myCurrentScene = &scene;
			break;
		}

		if (scene.sceneName == "AAAA_MainMenu.json")
		{
			myStartupScene = &scene;
			myCurrentScene = &scene;
		}
	}

	ES::EventSystem::GetInstance().SendEvent(levelSelectDataEvent);

	myCurrentScene->Init(myWindow, &myPrefabHandler);


	if (LoadScene(myCurrentScene))
	{
		myCurrentScene->Activate();
		//GlobalAudio::PlayMusic(sound::Music::Menu);
		//myWindow->GetGraphics().CreateRenderingQueue(true);
	}
}

void KE::SceneManager::Update()
{
	myCurrentScene->UpdateHierarchy();

	if (myLevelChangeIndex >= 0)
	{
		// Change Scene Event was recieved and queued

		myCurrentScene->levelChangeBuildIndex = -1;
		myCurrentScene->levelChangeIndex = -1;
		SwapScene(myLevelChangeIndex);

		myLevelChangeIndex = -1;
		return;
	}
#ifndef  KITTYENGINE_NO_EDITOR
	if (mySceneState == eSceneState::ePlayMode)
	{
#endif // ! KITTYENGINE_NO_EDITOR

		myCurrentScene->Update();

		if (myCurrentScene->levelChangeIndex > -1)
		{
			// Scene registered a request to change scene
			int index = myCurrentScene->levelChangeIndex;
			myCurrentScene->levelChangeIndex = -1;
			SwapScene(index);
			return;
		}

		if (myCurrentScene->levelChangeBuildIndex > -1)
		{
			// Scene registered a request to change scene by build index
			int vectorIndex = myBuildOrderMap.at(myCurrentScene->levelChangeBuildIndex);
			int buildIndex = myCurrentScene->levelChangeBuildIndex;

			SceneBuildData sceneData = myBuildSettingsData.at(buildIndex);
			myCurrentScene->levelChangeBuildIndex = -1;

			// Apply cubemap
			if (!sceneData.cubemap.empty())
			{
				myWindow->GetGraphics().AssignSkybox("Data/Assets/Materials/Skyboxes/" + myBuildSettingsData.at(buildIndex).cubemap);
				//myWindow->GetGraphics().AssignCubemap("Data/assets/" + myBuildSettingsData.at(buildIndex).cubemap);
			}

			if ((int)sound::Ambient::Count >= sceneData.ambientSFX)
			{
				if (sceneData.ambientSFX == -1) 
				{
					KE::GlobalAudio::PlayAmbient(sound::Ambient::Count);
				}
				else if(sceneData.ambientSFX >= 0)
				{
					KE::GlobalAudio::PlayAmbient((sound::Ambient)sceneData.ambientSFX);
				}
			}

			if (sceneData.musicSFX >= 0 && (int)sound::Music::Count > sceneData.musicSFX)
			{
				KE::GlobalAudio::PlayMusic((sound::Music)sceneData.musicSFX);
			}

			SwapScene(vectorIndex);
			return;
		}
#ifndef KITTYENGINE_NO_EDITOR
	}
#endif // 
	KE::DebugTimeLogger::BeginLogVar("Debug Draw");
	myCurrentScene->DebugDraw(*myDebugRenderer);
	KE::DebugTimeLogger::EndLogVar("Debug Draw");
}

void KE::SceneManager::SwapScene(const int aSceneIndex)
{
	for (size_t i = 0; i < myScenes.size(); i++)
	{
		if (myScenes[i].sceneID == aSceneIndex)
		{
			myCurrentScene->Deactivate();
			myCurrentScene = &myScenes[i];


			myWindow->GetGraphics().ClearModelData();

			myWindow->GetGraphics().GetVFXManager().ClearVFX();

			Timer timer;

			myCurrentScene->Init(myWindow, &myPrefabHandler);

			LoadScene(myCurrentScene);

			timer.UpdateDeltaTime();
			std::cout << myCurrentScene->sceneName << " | buildIndex = " << myCurrentScene->buildIndex << " | Scene load time: " << timer.GetDeltaTime() << std::endl;

#ifndef KITTYENGINE_SHIP
			myWindow->AddToWindowName(NarrowStringToWide(myCurrentScene->sceneName).c_str());
#endif
			myCurrentScene->Activate();

			//myWindow->GetGraphics().CreateRenderingQueue(true);

			P8::OnLevelLoadedEvent levelLoadedEvent(myCurrentScene->sceneName);
			ES::EventSystem::GetInstance().SendEvent(levelLoadedEvent);

#ifdef KITTYENGINE_NO_EDITOR
			myWindow->GetGraphics().GetCameraManager().SetHighlightedCamera(KE_MAIN_CAMERA_INDEX);
#else
			KE_EDITOR::ImGuiHandler::RegenerateGameObjectCache();
#endif
			return;
		}
	}
}

bool KE::SceneManager::LoadScene(Scene* aScene)
{
	std::string name = aScene->sceneName;
	return (myLevelImporter->LoadLevel(*aScene, name));
}

void KE::SceneManager::ReloadScene()
{
	myLevelChangeIndex = myCurrentScene->sceneID;
	//SwapScene(myCurrentScene->sceneID);
}

const std::vector<KE::Scene>& KE::SceneManager::GetScenes() const
{
	return myScenes;
}

const KE::Scene* KE::SceneManager::GetCurrentScene() const
{
	return myCurrentScene;
}

int KE::SceneManager::GetSceneIDByName(const std::string& aSceneName) const
{
	for (size_t i = 0; i < myScenes.size(); i++)
	{
		if (myScenes[i].sceneName == aSceneName)
		{
			return myScenes[i].sceneID;
		}
	}

	return -1;
}

void KE::SceneManager::ExportStartupScene(const Scene* aScene)
{
	std::string levelStartInfoFile = "Data/Settings/LevelSettings.json";

	nlohmann::json levelStartInfo;
	std::string customStartLevel = "";

	{
		std::ifstream file(levelStartInfoFile);

		if (file.is_open())
		{
			file >> levelStartInfo;
			customStartLevel = levelStartInfo["start_level_name"];
			customStartLevel += ".json";
		}
		file.close();
	}

	if (customStartLevel != aScene->sceneName)
	{
		levelStartInfo["start_level_name"] = aScene->sceneName.substr(0, aScene->sceneName.find_last_of("."));
		std::ofstream file(levelStartInfoFile);
		file << levelStartInfo;
		file.close();
	}

	myStartupScene = (Scene*)aScene;
}

void KE::SceneManager::LoadBuildSettings()
{
	std::string buildSettingsData = "Data/Settings/SceneData.json";
	std::ifstream ifs(buildSettingsData);

	nlohmann::json buildData;
	if (ifs.good())
	{
		buildData = nlohmann::json::parse(ifs);
	}
	else
	{
		KE_WARNING("Couldn't find SceneData.json @ %s , this could beintentional.", buildSettingsData.c_str());
	}

	ifs.close();

	if (!buildData.contains("Scenes"))
	{
		KE_WARNING("SceneData.json doesn't contain the correct data, check the file @ %s", buildSettingsData.c_str());
		return;
	}

	for (auto& scene : buildData["Scenes"])
	{
		const int index = scene["build_id"];

		myBuildSettingsData[index].name = scene["name"];
		myBuildSettingsData[index].buildIndex = index;
		if (scene.contains("cubemap"))
		{
			myBuildSettingsData[index].cubemap = scene["cubemap"];
		}
		if (scene.contains("ambientIndex"))
		{
			myBuildSettingsData[index].ambientSFX = scene["ambientIndex"];
		}
		if (scene.contains("musicIndex"))
		{
			myBuildSettingsData[index].musicSFX = scene["musicIndex"];
		}
	}
}

const int KE::SceneManager::CheckSceneForBuildMatch(const std::string& aScene, const int anIndex)
{
	for (int i = 0; i < myBuildSettingsData.size(); i++)
	{
		if (aScene == myBuildSettingsData[i].name)
		{
			myBuildOrderMap.insert(std::pair(myBuildSettingsData[i].buildIndex, anIndex));
			return myBuildSettingsData[i].buildIndex;
		}
	}
	return -1;
}

void KE::SceneManager::OnReceiveEvent(ES::Event& aEvent)
{
	if (SceneEvent* event = dynamic_cast<SceneEvent*>(&aEvent))
	{

		mySceneState = event->mySceneState;
		switch (event->mySceneState)
		{
		case KE::eSceneState::eEditorMode:
		{
			//mySceneState = event->mySceneState;
			break;
		}
		case KE::eSceneState::ePauseMode:
		{
			//mySceneState = event->mySceneState;
			break;
		}
		case KE::eSceneState::ePlayMode:
		{
			//mySceneState = event->mySceneState;
			break;
		}
		case KE::eSceneState::eExitPlayMode:
		{
			mySceneState = eSceneState::eEditorMode;
			myLevelChangeIndex = myCurrentScene->sceneID;
			break;
		}
		}

	}
	else if (P8::ChangeSceneByBuildIndex* sceneChangeEvent = dynamic_cast<P8::ChangeSceneByBuildIndex*>(&aEvent))
	{
		static int buildSceneToChangeTo = -1;

		if (sceneChangeEvent->buildIndex == 0)
		{
			// Change to main menu
			myCurrentScene->levelChangeBuildIndex = 0;
			buildSceneToChangeTo = -1;

			return;
		}
		else if (sceneChangeEvent->buildIndex > 0 && sceneChangeEvent->buildIndex < myBuildSettingsData.size())
		{
			myCurrentScene->levelChangeBuildIndex = sceneChangeEvent->buildIndex;
			return;
		}

		if (myCurrentScene->buildIndex == 0)
		{
			// if current scene is main menu, change scene to lobby
			myCurrentScene->levelChangeBuildIndex = 1;
			return;
		}
		else if (myCurrentScene->buildIndex == 1)
		{
			// if current scene is lobby, change scene to tutorial
			myCurrentScene->levelChangeBuildIndex = 2;
			return;
		}

		if (buildSceneToChangeTo < 0)
		{
			buildSceneToChangeTo = GetRandomUniformInt(4, (int)myBuildSettingsData.size() - 1);
			// else, randomize the level -- THIS IS TEMP ONLY FOR P8 -- OLLE
			myCurrentScene->levelChangeBuildIndex = buildSceneToChangeTo;
			return;
		}

		buildSceneToChangeTo = buildSceneToChangeTo + 1 > myBuildSettingsData.size() - 1 ? 4 : buildSceneToChangeTo + 1;

		myCurrentScene->levelChangeBuildIndex = buildSceneToChangeTo;
	}

	else if (P8::ChangeSceneEvent* changeSceneEvent = dynamic_cast<P8::ChangeSceneEvent*>(&aEvent))
	{
		int id = GetSceneIDByName(changeSceneEvent->sceneName);
		if (id > -1)
		{
			myLevelChangeIndex = id;
			std::cout << "\nHello from SceneManager!";
		}
	}
}

void KE::SceneManager::OnInit()
{
	ES::EventSystem::GetInstance().Attach<SceneEvent>(this);
	ES::EventSystem::GetInstance().Attach<P8::ChangeSceneEvent>(this); // TEMP
	ES::EventSystem::GetInstance().Attach<P8::ChangeSceneByBuildIndex>(this);
}

void KE::SceneManager::OnDestroy()
{
	ES::EventSystem::GetInstance().Detach<SceneEvent>(this);
	ES::EventSystem::GetInstance().Detach<P8::ChangeSceneEvent>(this); // TEMP
	ES::EventSystem::GetInstance().Detach<P8::ChangeSceneByBuildIndex>(this);

}
