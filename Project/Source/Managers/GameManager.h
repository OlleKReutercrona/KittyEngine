#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Utility/EventSystem.h"
#include "Project/Source/GameEvents/GameEvents.h"

namespace P8
{

	static bool IsGamePaused(const eGameStates aGameState)
	{
		return aGameState != eGameStates::ePlayLiveGame && aGameState != eGameStates::eDebugTesting;
	}

	// Game Data
	struct GameData
	{
		int numberOfPlayers = 0;
		int maxPoints = 10;

		int pointsPerKill = 1;

		/* 
			index based scoring
			[0] - Player one
			[1] - Player two
			[2] - Player three
			[3] - Player four
		*/
		int scores[4];
		int playersAlive[4];
		int playerIDs[4];
		int modelIDs[4];
	};

	struct PlayerKilledEvent;

	class GameManager : public KE::Component, public ES::IObserver
	{
		KE_EDITOR_FRIEND
	public:
		GameManager(KE::GameObject& aGameObject);
		~GameManager();

		void Awake() override;
		void Update() override;

		// Inherited via IObserver
		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnSceneChange() override;

		inline static bool IsPaused() { return OurIsPaused; }

		inline const GameData& GetGameData() const { return myGameData; }

		inline static eGameStates GetCurrentGameState() { return myCurrentGameState; }
		inline static eGameStates GetOldGameState() { return myOldGameState; }

	private:
		void ChangeGameState(const eGameStates aNewGameState);
		void OnStateExit(const eGameStates aOldGameState);

		void AssignScore(const P8::PlayerKilledEvent* someData);

		void HandlePlayerDeath(const int aDeadPlayerIndex);
		void ResetMatch();
		void ResetGame();

		// Inherited via IObserver
		void OnInit() override;
		void OnDestroy() override;


		void DEBUGIncrementLevelTest();

		static inline eGameStates myCurrentGameState = eGameStates::eUnknown;
		static inline eGameStates myOldGameState = eGameStates::eUnknown;


		// Game Data
		GameData myGameData;
		const float myGameOverSlowMoTime = 2.0f;
		float mySuddenDeathTime = 60.0f * 2.0f;
		float myTime = 0.0f;

		inline static bool OurIsPaused = false;

		int myStartLevelIndex = -1;
	};
}

