#pragma once
#include "Engine/Source/UI/GUIBox.h"
#include "Engine/Source/UI/GUITooltip.h"
#include "Engine/Source/Utility/EventSystem.h"

namespace KE
{
	class GUIScene;

#pragma region AlignType
	enum class eAlignType
	{
		BottomLeft,
		BottomCenter,
		BottomRight,
		Center,
		CenterLeft,
		CenterRight,
		TopLeft,
		TopCenter,
		TopRight,
		Fullscreen,
		Count
	};

	inline const char* EnumToString(const eAlignType aType)
	{
		switch (aType)
		{
		case eAlignType::BottomLeft: return "BottomLeft";
		case eAlignType::BottomCenter: return "BottomCenter";
		case eAlignType::BottomRight: return "BottomRight";
		case eAlignType::Center: return "Center";
		case eAlignType::CenterLeft: return "CenterLeft";
		case eAlignType::CenterRight: return "CenterRight";
		case eAlignType::TopLeft: return "TopLeft";
		case eAlignType::TopCenter: return "TopCenter";
		case eAlignType::TopRight: return "TopRight";
		case eAlignType::Fullscreen: return "Fullscreen";
		default: return "Unknown";
		}
	}
#pragma endregion

#pragma region ElementState
	enum class eGUIElementState
	{
		Idle,
		Hovered,
		Pressed,
	};

	inline const char* EnumToString(const eGUIElementState aState)
	{
		switch (aState)
		{
		case eGUIElementState::Idle: return "Idle";
		case eGUIElementState::Hovered: return "Hovered";
		case eGUIElementState::Pressed: return "Pressed";
		default: return "Unknown";
		}
	}

#pragma endregion

#pragma region GUIElementType

	//enum class eGUIElementType
	//{
	//	// General
	//	Decoration,
	//	// HUD
	//	Minimap,
	//	MinimapSpawnpoint,
	//	MinimapPlayer,
	//	MinimapEnemy,
	//	Health,
	//	Power,
	//	Money,
	//	EnemyHealth,
	//	MoneyGain,
	//	StopPlaceTrap,
	//	Crosshair,
	//	// HUD popups
	//	BotsIncoming,
	//	PlaceTraps,
	//	ReleaseWave,
	//	// Buttons
	//	Credits,
	//	StartGame,
	//	Settings,
	//	Exit,
	//	MainMenu,
	//	Resume,
	//	ResolutionUp,
	//	ResolutionDown,
	//	Resolution1,
	//	Resolution2,
	//	Resolution3,
	//	ToggleFullscreen,
	//	VolumeUp,
	//	VolumeDown,
	//	Volume0,
	//	Volume1,
	//	Volume2,
	//	Volume3,
	//	Volume4,
	//	Volume5,
	//	Volume6,
	//	Volume7,
	//	Volume8,
	//	Volume9,
	//	Volume10,
	//	Restart,
	//	SelectWave,
	//	WaveUp,
	//	WaveDown,
	//	Back,
	//	Count
	//};

	//inline const char* EnumToString(const eGUIElementType aType)
	//{
	//	switch (aType)
	//	{
	//	case eGUIElementType::Decoration: return "Decoration";
	//	case eGUIElementType::Minimap: return "Minimap";
	//	case eGUIElementType::MinimapSpawnpoint: return "MinimapSpawnpoint";
	//	case eGUIElementType::MinimapPlayer: return "MinimapPlayer";
	//	case eGUIElementType::MinimapEnemy: return "MinimapEnemy";
	//	case eGUIElementType::Health: return "Health";
	//	case eGUIElementType::Power: return "Power";
	//	case eGUIElementType::Money: return "Money";
	//	case eGUIElementType::EnemyHealth: return "EnemyHealth";
	//	case eGUIElementType::MoneyGain: return "MoneyGain";
	//	case eGUIElementType::StopPlaceTrap: return "StopPlaceTrap";
	//	case eGUIElementType::Crosshair: return "Crosshair";
	//	case eGUIElementType::BotsIncoming: return "BotsIncoming";
	//	case eGUIElementType::PlaceTraps: return "PlaceTraps";
	//	case eGUIElementType::ReleaseWave: return "ReleaseWave";
	//	case eGUIElementType::Credits: return "Credits";
	//	case eGUIElementType::StartGame: return "StartGame";
	//	case eGUIElementType::Settings: return "Settings";
	//	case eGUIElementType::Exit: return "Exit";
	//	case eGUIElementType::MainMenu: return "MainMenu";
	//	case eGUIElementType::Resume: return "Resume";
	//	case eGUIElementType::ResolutionUp: return "ResolutionUp";
	//	case eGUIElementType::ResolutionDown: return "ResolutionDown";
	//	case eGUIElementType::Resolution1: return "Resolution1";
	//	case eGUIElementType::Resolution2: return "Resolution2";
	//	case eGUIElementType::Resolution3: return "Resolution3";
	//	case eGUIElementType::ToggleFullscreen: return "ToggleFullscreen";
	//	case eGUIElementType::VolumeUp: return "VolumeUp";
	//	case eGUIElementType::VolumeDown: return "VolumeDown";
	//	case eGUIElementType::Volume0: return "Volume0";
	//	case eGUIElementType::Volume1: return "Volume1";
	//	case eGUIElementType::Volume2: return "Volume2";
	//	case eGUIElementType::Volume3: return "Volume3";
	//	case eGUIElementType::Volume4: return "Volume4";
	//	case eGUIElementType::Volume5: return "Volume5";
	//	case eGUIElementType::Volume6: return "Volume6";
	//	case eGUIElementType::Volume7: return "Volume7";
	//	case eGUIElementType::Volume8: return "Volume8";
	//	case eGUIElementType::Volume9: return "Volume9";
	//	case eGUIElementType::Volume10: return "Volume10";
	//	case eGUIElementType::Restart: return "Restart";
	//	case eGUIElementType::SelectWave: return "SelectWave";
	//	case eGUIElementType::WaveUp: return "WaveUp";
	//	case eGUIElementType::WaveDown: return "WaveDown";
	//	case eGUIElementType::Back: return "Back";
	//	case eGUIElementType::NextWave: return "NextWave";
	//	default: return "Unknown";
	//	}
	//}

#pragma endregion

	struct GUIElementEvent final : ES::Event
	{
		GUIElementEvent() = default;
		~GUIElementEvent() override = default;

		//eGUIElementType myGUIElementType = eGUIElementType::Decoration;
		std::string myEventName = "";
	};

	struct GUIAudioVolumeEvent : public ES::Event
	{
		GUIAudioVolumeEvent(const float aVolume) : myVolume(aVolume) {}
		float myVolume;
	};

	struct GUIElement : ES::IObserver
	{
		GUIElement();
		~GUIElement() override;

		void InitObserver();

		GUIBox myBox;
		GUITooltip myTooltip;
		//eGUIElementType myType;
		GUIElementEvent myEvent;
		eGUIElementState myState = eGUIElementState::Idle;
		eAlignType myAlignType = eAlignType::BottomCenter;
		SpriteBatch mySpriteBatch;
		Texture* myDisplayTexture = nullptr;
		Texture* mySecondaryTexture = nullptr;
		GUIScene* myParentScene = nullptr;
		//Sprite* mySprite = nullptr;
		bool isButton = false;
		std::string myName = "GUIElement";
		std::string myTexturePath = "";
		float myFillFactor = 1.0f;

		bool hasText = false;
		std::string myText = "Text";
		Vector4f myTextColour = { 1.0f, 1.0f, 1.0f, 1.0f };

		// TODO Cooldown test
		eProgressionDirection myProgressionDirection = eProgressionDirection::None;
		bool isTimerActive = false;
		float myCooldownTimer = 0.0f;
		float myTotalCooldown = 2.0f;

		bool shouldDraw = true;
		bool shouldHideAtStart = false;
		bool shouldReceiveEvents = false;

		// Temp
		//Vector2f myLinePoints[4];
		Vector4f myColour;

		GUITooltip& AddTooltip();

		void Update(float aDeltaTime);
		void SetFillFactor(const float aFactor);
		void Activate();
		void Click();
		void Highlight();
		void Reset();
		void UpdateSpriteScaleAndPosition(const Vector2f& aResolution, bool aUpdateResolution);

		float GetFillFactor() const { return myFillFactor; }

		// IObserver
		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		void OnDestroy() override;
	};
}
