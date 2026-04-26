#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "GameObject.h"
#include <vector>
#include <string>
#include <unordered_map>

namespace KE
{
	enum ReservedGameObjects
	{
		eMainCamera = 0,
		eDirectionalLight = 1,
		ePostProcessing = 2,
		eGameSystemManager = 3,
		eSkybox = 4,
	};

	class GameObject;
	class Scene;
	class LevelTransformFile;
	class DebugRenderer;

	class GameObjectManager
	{
		friend class LevelImporter; // This might get replaced by Adapter? Meanwhile I put this /DR
		friend class Scene; // This might get replaced by Adapter? Meanwhile I put this /OKR
		friend class SceneManager; // This might get replaced by Adapter? Meanwhile I put this /OKR

	public:
		GameObjectManager();
		~GameObjectManager();

		KE::GameObject* GetGameObject(const int anID);

		template<class T>
		KE::GameObject* GetGameObjectWithComponent();

		template<class T>
		std::vector<KE::GameObject*> GetGameObjectsWithComponent();

		KE::GameObject* GetLatestGameObject();

		void DestroyGameObject(const int anID);

		const std::vector<KE::GameObject*>& GetGameObjects() const;
		const std::vector<KE::GameObject*>& GetNewGameObjects() const;

		inline Scene* GetScene() const { return myScene; }

		KE::GameObject* CreateGameObject(const int anID, const std::string* aName = nullptr, const Transform& aTransform = Transform(), const bool isStatic = false);
		const int GenerateUniqueID();
	private:
		template<class T>
		void AddComponentToLast(void* aDataObject);
		template<class T>
		void AddComponentToObject(GameObject* aGO, void* aDataObject);
		void AddChild(GameObject& aParent, GameObject& aChild);
		void RemoveParent(GameObject& aChild);


		void Init(Scene* aScene);
		void RegisterToBlackboard();

		void UpdateHierarchy();
		void PreCalculateWorldTransforms();

		void Awake();
		void Update();
		void LateUpdate();
		void EarlyUpdate();

		void DebugDraw(KE::DebugRenderer& aDrawer);

		// Loading
		void ReserveSpace(size_t aSize);
		void LoadTransforms(const LevelTransformFile& aFile);

		void Unload();

	private:
		std::vector<KE::GameObject*> myGameObjects;
		std::vector<KE::GameObject*> myNewGameObjects;
		inline static std::vector<KE::GameObject*> myPersistantGameObjects;
		Scene* myScene = nullptr;

		std::unordered_map<int, GameObject*> myMappedGameObjects;
	};

	


	template<class T>
	inline void GameObjectManager::AddComponentToLast(void* aDataObject)
	{
		static_assert(std::is_base_of<Component, T>::value, "Type must inherit from Component");

		//static_assert(myNewGameObjects.size() > 0 && "Cant add component externaly when no GameObject has been created");

		myNewGameObjects.back()->AddComponent<T>();
		if (aDataObject != nullptr)
		{
			myNewGameObjects.back()->GetComponents<T>().back()->SetData(aDataObject);
			
		}
	}

	template<class T>
	inline void GameObjectManager::AddComponentToObject(GameObject* aGO, void* aDataObject)
	{
		static_assert(std::is_base_of<Component, T>::value, "Type must inherit from Component");

		//static_assert(myNewGameObjects.size() > 0 && "Cant add component externaly when no GameObject has been created");

		auto* component = aGO->AddComponent<T>();
		if (aDataObject != nullptr)
		{
			component->SetData(aDataObject);
		}
	}

	template<class T>
	inline KE::GameObject* GameObjectManager::GetGameObjectWithComponent()
	{
		for (size_t i = 0; i < myNewGameObjects.size(); i++)
		{
			if (myNewGameObjects[i]->HasComponent<T>())
			{
				return myNewGameObjects[i];
			}
		}

		for (size_t i = 0; i < myGameObjects.size(); i++)
		{
			if (myGameObjects[i]->HasComponent<T>())
			{
				return myGameObjects[i];
			}
		}

		return nullptr;
	}

	template<class T>
	inline std::vector<KE::GameObject*> GameObjectManager::GetGameObjectsWithComponent()
	{
		std::vector<KE::GameObject*> gameObjects;

		for (size_t i = 0; i < myNewGameObjects.size(); i++)
		{
			if (myNewGameObjects[i]->HasComponent<T>())
			{
				gameObjects.push_back(myNewGameObjects[i]);
			}
		}

		for (size_t i = 0; i < myGameObjects.size(); i++)
		{
			if (myGameObjects[i]->HasComponent<T>())
			{
				gameObjects.push_back(myGameObjects[i]);
			}
		}
		

		return gameObjects;
	}
}
