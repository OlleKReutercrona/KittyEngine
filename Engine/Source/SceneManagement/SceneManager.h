#pragma once
#include "Scene.h"
#include "Engine/Source/Utility/Event.h"
#include "Engine/Source/Utility/EventSystem.h"
#include "Engine/Source/LevelImporter/LevelImporter.h"
#include "Engine/Source/Graphics/DebugRenderer.h"
#include "Engine/Source/LevelImporter/PrefabHandler.h"
#include <memory>


namespace KE
{
	enum class eSceneState : char
	{
		ePlayMode,
		ePauseMode,
		eEditorMode,
		eExitPlayMode
	};

	struct SceneBuildData
	{
		std::string name;
		int buildIndex = -1;
		std::string cubemap = "";
		int ambientSFX = -1;
		int musicSFX = -1;
	};

	struct SceneEvent : ES::Event
	{
		SceneEvent() = default;
		SceneEvent(const eSceneState aSceneState) : mySceneState(aSceneState) { }
		~SceneEvent() override = default;

		eSceneState mySceneState = eSceneState::eEditorMode;
	};

	//struct QueueChangeSceneEvent : ES::Event
	//{
	//	int aBuildIndex = -1;
	//};

	class Window;
	class PrefabHandler;

	class SceneManager : ES::IObserver
	{
	public:
		SceneManager();
		~SceneManager();

		void Init(Window& aWindow);

		void Update();
		void SwapScene(const int aSceneIndex);
		bool LoadScene(Scene* aScene);
		void ReloadScene();

		const std::vector<Scene>& GetScenes() const;
		const Scene* GetCurrentScene() const;
		int GetSceneIDByName(const std::string& aSceneName) const;

		eSceneState GetSceneState() { return mySceneState; }
		void SetGameState(eSceneState aState) { mySceneState = aState; }

		inline Scene* GetStartupScene() { return myStartupScene; }
		void ExportStartupScene(const Scene* aScene);

		inline Scene* GetCurrentScene() { return myCurrentScene; }


#ifndef  KITTYENGINE_NO_EDITOR
		inline void SetActive(const bool aValue) { IsActive = aValue; }
		inline const bool GetActive() const { return IsActive; }
#endif // ! KITTYENGINE_NO_EDITOR

		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		void OnDestroy() override;

	private:
		void LoadBuildSettings();
		const int CheckSceneForBuildMatch(const std::string& aScene, const int anIndex);

	public:

		PrefabHandler myPrefabHandler;


	private:
		std::map<int, SceneBuildData> myBuildSettingsData;
		std::vector<Scene> myScenes;
		Scene* myCurrentScene = nullptr;
		Scene* myStartupScene = nullptr;
		std::unique_ptr<LevelImporter> myLevelImporter;
		std::unordered_map<int, int> myBuildOrderMap;

		DebugRenderer* myDebugRenderer = nullptr;

		Window* myWindow = nullptr;

		int myLevelChangeIndex = -1;
		//

		eSceneState mySceneState = eSceneState::eEditorMode;

#ifndef  KITTYENGINE_NO_EDITOR
		bool IsActive = true;
#endif // ! KITTYENGINE_NO_EDITOR
	};
}
