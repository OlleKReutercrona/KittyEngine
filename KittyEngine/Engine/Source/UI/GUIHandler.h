#pragma once
#include <deque>

#include "Engine/Source/UI/Minimap.h"
#include "Engine/Source/UI/GUIScene.h"
#include "Engine/Source/Math/Vector.h"
#include "Engine/Source/Utility/EventSystem.h"
#include "Engine/Source/Graphics/Text/Text.h"

namespace KE_EDITOR
{
	class GUIEditor;
}

namespace P8
{
	struct PresentScoreDataEvent;
}

namespace KE
{
	class SceneManager;
	enum class eGUISceneType;
	struct GUIElement;
	struct GUITooltip;
	class Window;
	class Graphics;

	enum class eFadeState
	{
		None,
		FadeIn,
		Show,
		FadeOut,
		Done
	};

	struct FadeElement
	{
		GUIElement* myElement = nullptr;
		eFadeState myFadeState = eFadeState::None;
		float myTimer = 0.0f;
		float myFadeInTime = 0.0f;
		float myFadeOutTime = 0.0f;
		float myShowTime = 0.0f;
	};

	struct Score
	{
		GUIElement* myScoreMin = nullptr;
		GUIElement* myScoreMax = nullptr;
		GUIElement* myPortrait = nullptr;
		GUIElement* myPlayerText = nullptr;
		GUIElement* myPlayerTextBg = nullptr;
		GUIElement* myScoreContainer = nullptr;
		std::vector<GUIElement*> myScoreElements = {};
		int myScorePreviousRound = -1;
		int myScore = -1;
		int myModelID = -1;
	};

	struct LevelSelectData
	{
		std::string myLevelName;
		int myLevelIndex = -1;
	};

	class GUIHandler : public ES::IObserver
	{
		friend class KE_EDITOR::GUIEditor;
	public:
		GUIHandler();
		~GUIHandler() override;
		void Init(Graphics* aGraphics);
		void LoadGUIFromFile();
		void SetupScore(const Vector2i& resolution);
		// UTILITY
		void AssignSpriteToGUIElement(GUIElement& aElement,
			Graphics* aGraphics,
			const Vector2f& aElementSize, const std::string& aSpritePath);
		void AssignSpriteToTooltip(GUITooltip& aTooltip,
			const Window* aWindow,
			const Vector2f& aTooltipSize,
			const std::string& aSpritePath);
		void CreateGUIScene(const std::string& aSceneName, const size_t aNumberOfElementsToReserve);
		void CreateGUIScene(const std::string& aSceneName);
		void RemoveGUIScene(const std::string& aSceneName);
		void PushGUIScene(const std::string& aSceneName);
		void PopGUIScene(const std::string& aSceneName);
		void PopGUIScene();
		GUIScene* GetGUIScene(const std::string& aSceneName);
		bool IsMouseOverGUI(const Vector2f& aMousePosition);
		GUIElement* GetGUIElementFromMousePosition(const Vector2f& aMousePosition);
		GUIElement* GetAnyGUIElementFromMousePosition(const Vector2f& aMousePosition);
		void UpdateGUI();
		void RenderGUI(Graphics* aGraphics);
		void DrawGUIGrid(Graphics* aGraphics, const Vector2i& aResolution);
	private:
		void ToggleDrawGUIGrid();
		void SetVolumeSliderPosition();
		void IncreaseVolume();
		void DecreaseVolume();
		void SetResolution() const;
		void SetVolumeText();
		void SetResolutionText();
		void SetScore(P8::PresentScoreDataEvent* aEvent);
		void SetScoreForPlayer(Score* aPScore, const int aScore, const int aModelID);
		void HideScoreForNonActivePlayer(const int aPlayerIndex);
		void SetPortraitForPlayer(Score* aPSScore, const int aModelID);
		void ResetScores();
		void SetGUIText(GUIElement* aElement, const std::string& aText);
		void MoveElementBetweenScenes(const std::string& aSourceSceneName, const std::string& aDestinationSceneName, const size_t aElementIndex);
		std::string GetCurrentSceneName() const;

		void RegisterAssignedEvent(const std::string& aEventName, GUIElement* aElement);
		void UnregisterAssignedEvent(GUIElement* aElement);

		void HandleEventTimers();
		void HandleScoreAnimation();
		void HandleBlockInput();
		void HandleCountdown();
		void HandleMenuElement(const bool aIsLeft);
		void HandleVolumeSlider(const bool aIsLeft);
		void HandleLevelSelect(const bool aIsLeft);
		void HandleResolutionChange(const bool aIsLeft);
		void HandleFullscreenChange();
		void AnimateScore(Score* aScore);
		void IncreaseScoreIndex();

		// IObserver
		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		void OnDestroy() override;

	private:
		void SetTextColour(GUIElement* aElement, const Vector4f& aColour);
		void SetDrawElement(const std::string& aEventName, bool aShouldDraw);
		void SetDrawTextElement(const std::string& aEventName, const std::string& aText, bool aShouldDraw);
		void SetTextAlpha(GUIElement* aElement, const float aAlpha);
		void SetColourOfElement(const std::string& aEventName, const Vector4f& aColour);
		void SetColourOfElement(GUIElement* aElement, const Vector4f& aColour);
		void SetTextureOfElement(const std::string& aEventName, const std::string& aTexturePath);
		void SelectFirstMenuElement();
		void SelectLastMenuElement();
		void SelectNextMenuElement();
		void SelectPreviousMenuElement();

		KE::eFadeState FadeElement(KE::FadeElement* aFadeElement);

	private:
		Graphics* myGraphics = nullptr;

		std::unordered_map<std::string, GUIScene> myGUISceneMap = {};
		std::deque<GUIScene*> myActiveGUIScenes = {};
		std::vector<GUIScene*> myInactiveGUIScenes = {};
		GUIElement* myHoveredElement = nullptr;
		GUIElement* mySelectedElement = nullptr;
		bool isGUIEnabled;
		bool drawGUIGrid;
		int myGridSizeX;
		int myGridSizeY;
		bool gridSquareCells;

		int myCurrentResolutionIndex;
		int myMaxResolutions;
		inline static const Vector2i RESOLUTIONS[3] = { {1280, 720}, {1920, 1080}, {2560, 1440} };
		bool isFullscreen;
		GUIElement* myFullscreenContainer = nullptr;
		GUIElement* myResolutionContainer = nullptr;
		GUIElement* myVolumeContainer = nullptr;

		inline static int myCurrentVolume = 10;
		int myMaxVolume;
		int myMinVolume;
		int myVolumeIncrement;

		SpriteFont myFont;

		SpriteBatch myCursorSpriteBatch;
		Sprite* myCursorSprite = nullptr;

		Vector4f myCharacterColours[4] = { {1.0f, 1.0f, 1.0f, 1.0f},
										   {1.0f, 1.0f, 1.0f, 1.0f},
										   {1.0f, 1.0f, 1.0f, 1.0f},
										   {1.0f, 1.0f, 1.0f, 1.0f} };

		std::string myCharacterPointPaths[5];
		std::string myCharacterDefaultPointPath;
		std::string myCharacterScoreboardPaths[5];

		std::string myPortraitPaths[5];

		std::unordered_map<std::string, std::vector<GUIElement*>> myEventMap = {};

		KE::FadeElement myReadyFadeElement;
		KE::FadeElement myGoFadeElement;
		KE::FadeElement* myFadeInElement = nullptr;

		// Event timers
		bool isCountdownActive;
		bool shouldCountDown;
		float myCountdownTimer;
		float myCountdownMaxTime;

		bool isBlockingInputTimed = false;
		float myBlockInputTimer;
		float myBlockInputTime;

		// Extra stuff
		GUIElement* myVolumeSlider = nullptr;

		Score myP1Score;
		Score myP2Score;
		Score myP3Score;
		Score myP4Score;

		int myMaxScore;
		int myScore[4] = { -1,-1,-1,-1 };

		int myScoreAnimIndex = 0;

		bool isScoreAnimating = false;
		float myScoreAnimationTimer = 0.0f;
		float myScoreAnimationTime = 0.4f;

		std::vector<LevelSelectData> myLevelSelectData = {};
		GUIElement* myLevelSelectImage = nullptr;
		GUIElement* myLevelSelectLeft = nullptr;
		GUIElement* myLevelSelectRight = nullptr;
		GUIElement* myLevelSelectText = nullptr;
		int myLevelIndex = -1; // The one to send to SceneManager
		int myCurrentLevelSelectIndex = -1;
		std::string myLevelImageBasePath = "Data/Assets/UI/03 Level select/UI_levelSelect_";
		std::string myCurrentLevelImagePath = "Data/Assets/UI/03 Level select/UI_levelSelect_levelImage_01.dds";

		bool isBlockingInput = true;
	};
}
