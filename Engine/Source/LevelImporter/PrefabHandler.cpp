#include "stdafx.h"
#include "PrefabHandler.h"

#include "Engine/Source/SceneManagement/SceneManager.h"
#include "Engine/Source/ComponentSystem/GameObjectManager.h"
#include <External/Include/nlohmann/json.hpp>

#include <filesystem>

void KE::PrefabHandler::Init(KE::SceneManager* aSceneManager)
{
	mySceneManager = aSceneManager;

	// Iterates through every file in the given folder
	std::string folderPath = "Data/Prefabs";
	for (const auto& entry : std::filesystem::directory_iterator(folderPath))
	{
		if (entry.path().extension() != ".json") continue;

		std::string filePath = folderPath + '/' + entry.path().filename().string();

		nlohmann::json prefab = LevelImporter::GetJsonObj(filePath);

		AddPrefab(prefab);
	}
}

void KE::PrefabHandler::AddPrefab(const nlohmann::json& aJsonBlock)
{
	if (!aJsonBlock.contains("components")) return;

	std::string prefabName = aJsonBlock["name"];

	PostponedObjects prefab;

	prefab.prefabs = aJsonBlock["components"];

	myPostponedObjects[prefabName] = prefab;

	KE_LOG_CHANNEL("Prefabs", "Created a new prefab with name: %s", prefabName.c_str());
}

KE::GameObject* KE::PrefabHandler::Instantiate(const std::string& aPrefab)
{
	if (myPostponedObjects.count(aPrefab) < 1) return nullptr;

	auto& GOManager = mySceneManager->GetCurrentScene()->GetGameObjectManager();

	KE::GameObject* gameObject = GOManager.CreateGameObject(GOManager.GenerateUniqueID(), &aPrefab);

	LevelImporter::AddComponents(mySceneManager->GetCurrentScene(), *gameObject, myPostponedObjects[aPrefab].prefabs);

	return gameObject;
}
