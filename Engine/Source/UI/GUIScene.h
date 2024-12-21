#pragma once
#include "Engine/Source/UI/GUIElement.h"

namespace KE_EDITOR {
	class GUIEditor;
}

namespace KE
{
	//enum class eGUISceneType
	//{
	//	PlayerHUD,
	//	InGameMenu,
	//	MainMenu,
	//	Settings,
	//	Credits,
	//	PlayerDeath,
	//	Count
	//};

	class GUIScene
	{
		friend class KE_EDITOR::GUIEditor;
	public:
		GUIScene() = default;
		~GUIScene() = default;
		//void	Init(const eGUISceneType aType, const size_t aNumberOfElementsToReserve);
		void Init(const std::string& aSceneName, const size_t aNumberOfElementsToReserve);
		void Init(const std::string& aSceneName);
		void UpdateGUISize(int aWidth, int aHeight);
		GUIElement& CreateGUIElement(
			const Vector2f& aSize,
			const Vector2f& aOffset,
			const Vector2f& aScreenResolution,
			//const eGUIElementType aGUIElementType,
			const bool aIsClickable,
			const eAlignType aAlignType,
			const eProgressionDirection aDirection = eProgressionDirection::None
		);
		GUIElement& CreateGUIElement(
			const std::string& aName,
			const std::string& aTexturePath,
			const std::string& aEventName,
			const eAlignType aAlignType,
			const eProgressionDirection aDirection,
			const Vector2f& aOffsetResolutionFactor,
			const Vector2f& aSizeResolutionFactor,
			const Vector2i& aScreenResolution,
			const bool aIsButton
		);
		GUIElement& DuplicateGUIElement(const GUIElement& aElement, const Vector2f& aResolution);
		GUITooltip& CreateTooltip(
			GUIElement& aElement,
			const Vector2f& aSize,
			const Vector2f& aOffsetFromParent
		);
		std::vector<GUIElement>& GetGUIElements() { return myGUIElements; }
		//eGUISceneType GetType() const { return myType; }
		const std::string& GetSceneName() const { return myName; }
		void MoveElementUp(const size_t aIndex);
		void MoveElementDown(const size_t aIndex);
		void MoveElement(const size_t aIndex, const size_t aNewIndex);
		void AddElement(GUIElement& aElement);
		void RemoveElement(const size_t aIndex);
		void SetActive(bool aState) { myIsActive = aState; }
		bool IsActive() { return myIsActive; }

	private:
		std::vector<GUIElement> myGUIElements = {};
		//eGUISceneType myType = eGUISceneType::Count;
		std::string myName;
		bool myIsActive = false;
	};
}
