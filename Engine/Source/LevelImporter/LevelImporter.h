#pragma once
#include <External/Include/nlohmann/json.hpp>
#include <string>
#include <unordered_map>

#include "Engine/Source/Math/Transform.h"

namespace KE {
	class Graphics;
	class Scene;
	class SceneManager;
	class GameObject;

	class LevelImporter {
	public:
		void Init(Graphics* aGraphics, SceneManager* aSceneManager);
		bool LoadLevel(Scene& aScene, std::string& aFileName);
		bool LoadGameSystemData(Scene& aScene, std::string& aFileName);
		// void AddComponents(Scene& aScene, nlohmann::json& someComponents);
		static void AddComponents(KE::Scene* aCurrentScene,
								  KE::GameObject& aGameObject,
								  nlohmann::json& someComponents);
		KE::GameObject& AddGameObject(Scene& aScene,
									  nlohmann::json& aGameObject);
		static nlohmann::json GetJsonObj(std::string& aFilePath);

	private:
		void LoadLevelSettings(const Scene& aScene);

		std::string myLevelsDir = "Data/Levels/";

		SceneManager* mySceneManager;
		inline static Graphics* myGraphics = nullptr;
		Transform transformData;

		std::vector<Transform> myNavmeshBuilderData;
	};

}  // namespace KE
