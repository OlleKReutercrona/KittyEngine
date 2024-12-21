#pragma once
#include "Window.h"
#include "Files/GUIFile.h"
#include "Utility/EventSystem.h"

namespace KE
{
	enum class eProgressionDirection;
	enum class eAlignType;
	enum class eGUIElementType;
	struct GUIElement;
	class GUIScene;
	class GUIHandler;
}

struct GUIDragDropPayload
{
	int elementIndex;
	//const char* sourceSceneName;
	KE::GUIScene* sourceScene;
};

std::string GetElementTypeFromEnum(KE::eGUIElementType aType);
std::string GetAlignTypeFromEnum(KE::eAlignType aType);
std::string GetProgressionDirectionFromEnum(KE::eProgressionDirection aType);

namespace KE_EDITOR
{
	class GUIEditor : public EditorWindowBase, ES::IObserver
	{
		KE_EDITOR_FRIEND
	public:
		GUIEditor(EditorWindowInput aStartupData = {}) : EditorWindowBase(aStartupData) {}
		~GUIEditor() override;

		const char* GetWindowName() const override { return "GUI Editor"; }
		void Init() override;
		void Update() override;
		bool DisplayDebugOptions();
		void DisplaySelectedGuiElement();
		void RenameNotUniqueElementName(KE::GUIScene* aGuiScene, std::string& aOutElementName);
		void RenameNotUniqueSceneName(KE::GUIHandler* aGuiHandler, std::string& aOutElementName);
		bool DisplayGuiSceneElements(KE::GUIScene* aGuiScene);
		void LoadGUIFromFile();
		void Render() override;

		bool IsSceneActive(const KE::GUIScene* aScene) const;
		void SetSceneActive(const KE::GUIScene* aScene, bool aActive) const;

		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		void OnDestroy() override;

	private:
		KE::GUIHandler* myGUIHandler = nullptr;

		//std::vector<KE::GUIScene*> mySelectableGUIScenes;

		KE::GUIScene* mySelectedGUIScene = nullptr;
		KE::GUIElement* mySelectedGUIElement = nullptr;
		int mySelectedElementIndex = -1;

		static constexpr size_t bufferSize = 64;
		char textBuffer[bufferSize] = { 0 };
		char sceneInputBuffer[bufferSize] = { 0 };
		char elementInputBuffer[bufferSize] = { 0 };
		char eventBuffer[bufferSize] = { 0 };

		bool shouldDrawAnchor = true;

		bool lockSelect = false;
	};
}

