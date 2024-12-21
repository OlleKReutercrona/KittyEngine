#pragma once
#include <Engine/Source/Utility/Event.h>
#include <Engine/Source/Input/Gamepad.h>

namespace P8
{
	struct ActionCameraEvent : ES::Event
	{
		ActionCameraEvent(const KE::GameObject& aGameObject, const bool shouldAdd) : gameObject(aGameObject), addTarget(shouldAdd) {}

		const KE::GameObject& gameObject;
		const bool addTarget;
	};

	struct SlowMotionEventData
	{
		int resetCurvePower = 2;
		float resetTime = 1.0f;
		float timeDelayUntilReset = 0.75f;
		float minModifierValue = 0.01f;
		float maxModifierValue = 1.0f;
		float timeModifier = 0.05f;
	};

	struct SlowMotionEvent : ES::Event
	{
		SlowMotionEvent(const SlowMotionEventData someData = SlowMotionEventData())
			: data(someData) 
		{}

		const SlowMotionEventData data;
	};

	struct CameraShakeData
	{
		float shakefactor = 0.05f;
		float shakeTime = 1.0f;
		Vector3f explosionPosition = { 0.0f, 0.0f, 0.0f };
	};

	struct CameraShakeEvent : ES::Event
	{
		CameraShakeEvent(CameraShakeData someData = CameraShakeData()) : data(someData) {}

		CameraShakeData data;
	};



	struct PlayerKilledEvent : ES::Event
	{
		PlayerKilledEvent(const int aKilledPlayerIndex, const int aMurdererIndex) : myKilledPlayerIndex(aKilledPlayerIndex), myMurdererIndex(aMurdererIndex) {};

		const int myKilledPlayerIndex = -1;
		const int myMurdererIndex = -1;
	};

	struct PlayerDataMessage : ES::Event
	{
		PlayerDataMessage(const int anIndex, const bool aIsNew) : index(anIndex), isNew(aIsNew) {}

		const int index = -1;
		const bool isNew = true;
	};

	struct PlayerModelDataEvent : ES::Event
	{
		int playerIndex[4];
		int modelIndex[4];
	};

	enum class eGameStates
	{
		// For full gamestate flowchart, check miro
		// @ https://miro.com/app/board/uXjVN7as-yc=/

		eUnknown,
		eMenuMain,
		eMenuLobby,
		eMenuSetting,
		eMenuPause,
		eLoading,
		ePlayTutorial,
		ePlayCountdown,
		ePlayLiveGame,
		ePlayMatchEnd,
		ePlayPresentScores,
		ePlayGameOver,
		eDebugTesting,
		eWinScreen,
		Count,
	};

	static std::string const EnumToString(const eGameStates aGameState)
	{
		switch (aGameState)
		{
		case eGameStates::eUnknown:
		{
			return "Unknown";
		}
		case eGameStates::eMenuMain:
		{
			return "Main Menu";
		}
		case eGameStates::eMenuLobby:
		{
			return "Menu Lobby";
		}
		case eGameStates::eMenuSetting:
		{
			return "Menu Settings";
		}	
		case eGameStates::eLoading:
		{
			return "Loading";
		}
		case eGameStates::ePlayTutorial:
		{
			return "Tutorial";
		}
		case eGameStates::ePlayCountdown:
		{
			return "Countdown";
		}
		case eGameStates::ePlayLiveGame:
		{
			return "Live Game";
		}
		case eGameStates::ePlayMatchEnd:
		{
			return "Match End";
		}
		case eGameStates::ePlayPresentScores:
		{
			return "Present Scores";
		}
		case eGameStates::ePlayGameOver:
		{
			return "Game Over";
		}
		case eGameStates::eDebugTesting:
		{
			return "Debug Testing";
		}
		case eGameStates::eWinScreen:
		{
			return "Win Screen";
		}
		default:
			return "Unknown";
		}
	};

	// Event Messages
	struct ChangeGameState : ES::Event
	{
		ChangeGameState(const eGameStates aNewGameState) : newGameState(aNewGameState) {}

		const eGameStates newGameState;
	};

	struct GameStateHasChanged : ES::Event
	{
		GameStateHasChanged(const eGameStates aNewGameState) : newGameState(aNewGameState) {}

		const eGameStates newGameState;
	};

	struct PresentScoreDataEvent : ES::Event
	{
		int scores[4];
		int playerID[4];
		int modelID[4];
	};

	enum class eLobbyEvents
	{
		PlayerJoined,
		PlayerReady,
		PlayerNotReady,
		PlayerRemoved,
		CharacterSelection,
		AllReady,
		AllNotReady,
		GameStart,
	};

	struct LobbyEventData
	{
		//int myCharacterIndex = -1;
		int mySelectedCharactersPlayerIndices[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
	};

	struct LobbyEvent : ES::Event
	{
		LobbyEvent(const eLobbyEvents aLobbyEvent, int aPlayerIndex, void* aData = nullptr)
			:
			myLobbyEvent(aLobbyEvent),
			myPlayerIndex(aPlayerIndex),
			myData(aData)
		{}

		const eLobbyEvents myLobbyEvent;
		int myPlayerIndex;
		void* myData = nullptr;
	};


	// TEMP TEMP TEMP TEMP
	struct ChangeSceneEvent : ES::Event
	{
		ChangeSceneEvent(const std::string& aSceneName) : sceneName(aSceneName) {}
		std::string sceneName;
	};

	struct ChangeSceneByBuildIndex : ES::Event
	{
		ChangeSceneByBuildIndex(const int aBuildIndex = -1) : buildIndex(aBuildIndex) {}

		// Set buildindex to 0 to change to main menu
		// Leave as -1 to change the current level index randomly
		const int buildIndex = -1;
	};

	struct OnLevelLoadedEvent : ES::Event
	{
		OnLevelLoadedEvent(const std::string& aLevelName) : levelName(aLevelName) {}
		std::string levelName;
	};

	struct LevelSelectData
	{
		std::string myLevelName;
		int myLevelIndex = -1;
	};

	struct LevelSelectDataEvent : ES::Event
	{
		std::vector<LevelSelectData> myLevelData;
	};

	struct StartingLevelEvent : ES::Event
	{
		int myLevelIndex = -1;
	};

	struct GamepadRumbleEvent : ES::Event
	{
		GamepadRumbleEvent(const int aGamepadIndex, const float aLeftMotor, const float aRightMotor, const float aDuration, const KE::RumbleType aRumbleType = KE::RumbleType::Timed)
			: gamepadIndex(aGamepadIndex), leftMotor(aLeftMotor), rightMotor(aRightMotor), duration(aDuration), rumbleType(aRumbleType)
		{
		}
		const int gamepadIndex = -1;
		const float leftMotor = 0.0f;
		const float rightMotor = 0.0f;
		const float duration = 0.0f;
		KE::RumbleType rumbleType = KE::RumbleType::None;
	};
}