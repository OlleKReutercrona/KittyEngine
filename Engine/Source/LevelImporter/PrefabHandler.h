#pragma once
#include <External/Include/nlohmann/json.hpp>

namespace KE
{
	class GameObject;
	class SceneManager;

	class PrefabHandler
	{
	public:
		PrefabHandler() = default;
		~PrefabHandler() = default;

		void Init(KE::SceneManager* aSceneManager);

		void AddPrefab(const nlohmann::json& aJsonBlock);
		KE::GameObject* Instantiate(const std::string& aPrefab);
	private:
		struct PostponedObjects
		{
			nlohmann::json prefabs;
		};
		std::unordered_map<std::string, PostponedObjects> myPostponedObjects;

		SceneManager* mySceneManager;
	};
}