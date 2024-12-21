#pragma once
#include "Engine/Source/ComponentSystem/GameObjectManager.h"
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Collision/CollisionHandler.h"
#include "Engine/Source/AI/Pathfinding/Navmesh.h"
#include "Engine/Source/AI/Pathfinding/Pathfinder.h"
#include "Engine/Source/Collision/RaycastHandler.h"

#ifndef KE_NOEDITOR
namespace KE_EDITOR
{
	class Editor;
	class ImGuiHandler;
}
#endif

namespace KE
{
	class Window;
	class SpriteFont;
	class PrefabHandler;

	enum class SceneDrawFlags
	{
		eDrawGameObject = 1 << 0,
		eDrawPhysX = 1 << 1,
		eDrawNavmesh = 1 << 2,
		eDrawPathfinding = 1 << 3,
	};

	static inline const char* EnumToString(SceneDrawFlags aFlag)
	{
		switch (aFlag)
		{
			case SceneDrawFlags::eDrawGameObject: 
			{
				return "Game Objects";
			}
			case SceneDrawFlags::eDrawPhysX:
			{
				return "PhysX";
			}
			case SceneDrawFlags::eDrawNavmesh:
			{
				return "Navmesh";
			}
			case SceneDrawFlags::eDrawPathfinding:
			{
				return "Pathfinding";
			}
			default:
			{
				return "Unknown";
			}
		}
	}

	class Scene
	{
		KE_EDITOR_FRIEND
		friend class SceneManager;
		friend class LevelImporter;

	public:
		Scene(const int aSceneID, const std::string aSceneName, const int aBuildIndex = -1);
		~Scene();

		//this kinda sucks:
		void SetLevelChangeIndex(int aIndex) { levelChangeIndex = aIndex; }

		// This changes level from the build order
		void SetLevelFromBuildIndex(int aIndex) { levelChangeBuildIndex = aIndex; }

		const int sceneID;
		const int buildIndex;
		const std::string sceneName;

		CollisionHandler myCollisionHandler;
		inline static PrefabHandler* myPrefabHandler;

		inline GameObjectManager& GetGameObjectManager() { return gameObjectManager; }
		inline Navmesh& GetNavmesh() { return myNavmesh; }

		void ToggleDrawDebugFlag(const SceneDrawFlags aFlag);
		bool GetDrawFlag(const SceneDrawFlags aFlag);
	private:
		void Init(Window* aWindow, PrefabHandler* aPFHandler);

		void Activate();
		void Deactivate();

		void UpdateHierarchy();
		void Update();

		void DebugDraw(KE::DebugRenderer& aDrawer);

		// If no Transform provided, default Transform constructor will be assigned.
		KE::GameObject* AddGameObject(const int anID, const std::string& aName, const Transform& aTransform = Transform(), const bool isStatic = false);

		template <class T>
		void AddComponentToLast(void* aDataObject);

		void RegisterNavmesh(std::string& aObjFilepath);

		void AddChildParentPair(const int aParent, const int aChild);
		void AssignAdoptedChildren();

		GameObjectManager gameObjectManager;

		std::string myNavmeshPath;
		Navmesh myNavmesh;
		Pathfinder myPathfinder;
		RaycastHandler myRaycastHandler;

		Window* myWindow = nullptr;

		int levelChangeIndex = -1;
		int levelChangeBuildIndex = -1;

		// Parent - Child
		std::unordered_map<int, int> myToBeParents;

		int myDrawDebugFlags = 0;
	};


	template <class T>
	inline void Scene::AddComponentToLast(void* aDataObject)
	{
		static_assert(std::is_base_of<Component, T>::value, "Type must inherit from Component");

		gameObjectManager.AddComponentToLast<T>(aDataObject);
	}
}
