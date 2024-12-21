#include "stdafx.h"
#include "GameObjectManager.h"
#include "GameObject.h"
#include "SceneManagement/Scene.h"
#include "Engine/Source/Utility/DebugTimeLogger.h"
#include "Engine/Source/Files/PawFile.h"
#include "Engine/Source/Utility/Logging.h"
#include "Engine/Source/Graphics/DebugRenderer.h"
#include <Windows.h>

KE::GameObjectManager::GameObjectManager()
{
}

KE::GameObjectManager::~GameObjectManager()
{
	Unload();
}

void KE::GameObjectManager::Unload()
{
	for (int i = 0; i < myNewGameObjects.size(); i++)
	{
		if (myNewGameObjects[i]->myID == ReservedGameObjects::eGameSystemManager)
		{
			myNewGameObjects[i]->OnSceneChange();
			myPersistantGameObjects.push_back(myNewGameObjects[i]);
			continue;
		}
		delete myNewGameObjects[i];
		myNewGameObjects[i] = nullptr;
	}
	myNewGameObjects.clear();

	for (int i = 0; i < myGameObjects.size(); i++)
	{
		if (myGameObjects[i]->myID == ReservedGameObjects::eGameSystemManager)
		{
			myGameObjects[i]->OnSceneChange();

			myPersistantGameObjects.push_back(myGameObjects[i]);
			continue;
		}
		delete myGameObjects[i];
		myGameObjects[i] = nullptr;
	}
	myGameObjects.clear();

	myMappedGameObjects.clear();
}

void KE::GameObjectManager::ReserveSpace(size_t aSize)
{
	myNewGameObjects.reserve(aSize);
	myGameObjects.reserve(aSize);
}

void KE::GameObjectManager::LoadTransforms(const LevelTransformFile& aFile)
{
	// Allocate memory
	ReserveSpace(aFile.myData.transforms.size());


	/*
		TODO:
		This is probably not the best solution to this problem...
		Preferably we would want to pool up all the transforms elsewhere (maybe as a component) where we can memcpy 
		all the transforms from the file directly to the component.
		- Olle 
	*/

	for (int i = 0; i < aFile.myData.transforms.size(); i++)
	{
		CreateGameObject(aFile.myData.transformsIDs[i], nullptr, aFile.myData.transforms[i]);
	}
}

KE::GameObject* KE::GameObjectManager::CreateGameObject(const int anID, const std::string* aName, const Transform& aTransform, const bool isStatic)
{
	GameObject* GO = new GameObject(anID, isStatic, *this);

	GO->name = aName == nullptr ? "GameObject_" + std::to_string(anID) : *aName;
	GO->myTransform = aTransform;

	myNewGameObjects.push_back(GO);

	myMappedGameObjects.emplace(std::pair(anID, GO));

	return GO;
}

KE::GameObject* KE::GameObjectManager::GetGameObject(const int anID)
{
	if (myMappedGameObjects.count(anID) > 0)
	{
		return myMappedGameObjects[anID];
	}

	OutputDebugString(L"GAMEOBJECT DOESNT EXIST");
	return nullptr;
}

KE::GameObject* KE::GameObjectManager::GetLatestGameObject()
{
	if (myNewGameObjects.size() > 0)
	{
		return myNewGameObjects.back();
	}
	if (myGameObjects.size() > 0)
	{
		return myGameObjects.back();
	}

	return nullptr;
}

void KE::GameObjectManager::DestroyGameObject(const int anID)
{
	if (!myMappedGameObjects.contains(anID)) return;

	GameObject* GO = myMappedGameObjects.at(anID);

	myGameObjects.erase(std::remove(myGameObjects.begin(), myGameObjects.end(), GO), myGameObjects.end());
	myNewGameObjects.erase(std::remove(myNewGameObjects.begin(), myNewGameObjects.end(), GO), myNewGameObjects.end());

	myMappedGameObjects.erase(anID);

	delete GO;
}

void KE::GameObjectManager::AddChild(GameObject& aParent, GameObject& aChild)
{
	// Assign parent / child
	aParent.myChildren.push_back(&aChild);
	aChild.myParent = &aParent;

	// Update childs transform
	aChild.UpdateTransforms();
}

void KE::GameObjectManager::RemoveParent(GameObject& aChild)
{
	if (aChild.myParent == nullptr) return;

	for (size_t i = 0; i < aChild.myParent->myChildren.size(); i++)
	{
		if (aChild.myParent->myChildren[i] == &aChild)
		{
			aChild.myParent->myChildren.erase(aChild.myParent->myChildren.begin() + i);
			break;
		}
	}
	
	aChild.myParent = nullptr;
}

void KE::GameObjectManager::Init(Scene* aScene)
{
	myScene = aScene;

	for (unsigned int i = 0; i < myPersistantGameObjects.size(); i++)
	{
		myPersistantGameObjects[i]->myManager = this;

		if (GameObject* GO = GetGameObject(myPersistantGameObjects[i]->myID))
		{
			DestroyGameObject(GO->myID);
		}
		myNewGameObjects.push_back(myPersistantGameObjects[i]);
		myMappedGameObjects.emplace(std::pair(myPersistantGameObjects[i]->myID, myPersistantGameObjects[i]));
	}

	myPersistantGameObjects.clear();
}

void KE::GameObjectManager::RegisterToBlackboard()
{
	KE_GLOBAL::blackboard.Register("gameObjectManager", this);
}

void KE::GameObjectManager::UpdateHierarchy()
{
	KE::DebugTimeLogger::BeginLogVar("GameObject Transform Update");
	for (int i = 0; i < myNewGameObjects.size(); i++)
	{
		if (myNewGameObjects[i]->isStatic) continue;

		myNewGameObjects[i]->UpdateTransforms();
	}

	for (int i = 0; i < myGameObjects.size(); i++)
	{
		if (myGameObjects[i]->isStatic) continue;

		myGameObjects[i]->UpdateTransforms();
	}
	KE::DebugTimeLogger::EndLogVar("GameObject Transform Update");
}

void KE::GameObjectManager::PreCalculateWorldTransforms()
{
	for (int i = 0; i < myNewGameObjects.size(); i++)
	{
		myNewGameObjects[i]->UpdateTransforms();
	}

	for (int i = 0; i < myGameObjects.size(); i++)
	{
		myGameObjects[i]->UpdateTransforms();
	}
}

void KE::GameObjectManager::Awake()
{
	int size = static_cast<int>(myNewGameObjects.size());

	for (int i = size - 1; i >= 0; i--)
	{
		if (!myNewGameObjects[i]->isActive) continue;

		myNewGameObjects[i]->Awake();


		myGameObjects.push_back(myNewGameObjects[i]);

		myNewGameObjects.erase(myNewGameObjects.begin() + i);
	}
}

void KE::GameObjectManager::Update()
{
	KE::DebugTimeLogger::BeginLogVar("GameObject Update Call");
	for (int i = 0; i < myGameObjects.size(); i++)
	{
		myGameObjects[i]->Update();
	}
	KE::DebugTimeLogger::EndLogVar("GameObject Update Call");

}

void KE::GameObjectManager::LateUpdate()
{
	for (int i = 0; i < myGameObjects.size(); i++)
	{
		myGameObjects[i]->LateUpdate();
	}
}

void KE::GameObjectManager::EarlyUpdate()
{
	for (int i = 0; i < myGameObjects.size(); i++)
	{
		myGameObjects[i]->EarlyUpdate();
	}
}

void KE::GameObjectManager::DebugDraw(KE::DebugRenderer& aDrawer)
{
	for (int i = 0; i < myGameObjects.size(); i++)
	{
		//myGameObjects[i]->DebugDraw(aDrawer);
	}
}

const std::vector<KE::GameObject*>& KE::GameObjectManager::GetGameObjects() const
{
	return myGameObjects;
}

const std::vector<KE::GameObject*>& KE::GameObjectManager::GetNewGameObjects() const
{
	return myNewGameObjects;
}

const int KE::GameObjectManager::GenerateUniqueID()
{
	int id;
	while (true)
	{
		id = rand() % 100000 + 100;
		if (myMappedGameObjects.count(id) == 0)
		{
			break;
		}
	}

	return id;
}

