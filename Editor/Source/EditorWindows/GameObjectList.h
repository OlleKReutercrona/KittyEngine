#pragma once
#ifndef KITTYENGINE_NO_EDITOR

namespace KE
{
	class GameObject;
}

namespace KE_EDITOR
{

	struct GameObjectDataCache
	{
		std::string assembledName;
		std::string objectName;

		std::string searchName;

		bool matchesFilter = true;
		bool childrenMatchesFilter = true;
		int filterMatchCharacterStart = 0;
		int filterMatchCharacterEnd = 0;
	};

	class GameObjectList : public KE_EDITOR::EditorWindowBase
	{
	private:
		std::unordered_map<int, GameObjectDataCache> gameObjectDataCache = {};
		size_t lastGameObjectCount = 0;

	public:
		GameObjectList(EditorWindowInput aStartupData = {}) : EditorWindowBase(aStartupData) {}
		void Init() override;
		void Update() override;
		void Render() override;

		const char* GetWindowName() const override { return "Game Object List"; };

		bool GameObjectCacheDirty(const std::vector<KE::GameObject*>& aGameObjectList);
		void CreateGameObjectCache(const std::vector<KE::GameObject*>& aGameObjectList);
		void CalculateGameObjectFilter(const std::vector<KE::GameObject*>& aGameObjectList, const std::string& aNameFilter);
		void DisplayListEntry(KE::GameObject* aGameObject);
	};

}

#endif