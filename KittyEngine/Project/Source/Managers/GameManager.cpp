#include "stdafx.h"
#include "GameManager.h"

#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/SceneManagement/SceneManager.h"
#include "Engine/Source/Utility/Event.h"
#include "Project/Source/GameEvents/GameEvents.h"
#include "Engine/Source/Input/InputEvents.h"
#include "Engine/Source/Audio/GlobalAudio.h"

P8::GameManager::GameManager(KE::GameObject& aGameObject) : KE::Component(aGameObject)
{
	OnInit();

	ResetGame();
}

P8::GameManager::~GameManager()
{
	OnDestroy();
}

void P8::GameManager::Awake()
{
	if (myCurrentGameState == eGameStates::eUnknown)
	{
		//ChangeGameState(eGameStates::eDebugTesting);
		ChangeGameState(eGameStates::eMenuMain);
	}
	else if (myCurrentGameState == eGameStates::eLoading)
	{
		if (myOldGameState == eGameStates::eMenuLobby)
		{			
			// if loading from another scene and old gamestate is main menu, tutorial is shown
			ChangeGameState(eGameStates::ePlayTutorial);
			return;
		}
		else if (myOldGameState == eGameStates::ePlayPresentScores)
		{
			// if loading from another scene and old gamestate is present score, countdown is shown
			ChangeGameState(eGameStates::ePlayCountdown);
		}

	}
}

void P8::GameManager::Update()
{
#ifndef KITTYENGINE_NO_EDITOR
	// Only for testing, should not be in live game
	if (GetAsyncKeyState(VK_F9) == SHRT_MIN + 1)
	{
		if (myCurrentGameState != eGameStates::eDebugTesting)
		{
			ChangeGameState(eGameStates::eDebugTesting);

			P8::ChangeSceneByBuildIndex sceneMsg(3);
			ES::EventSystem::GetInstance().SendEvent<P8::ChangeSceneByBuildIndex>(sceneMsg);
		}
		else
		{
			myCurrentGameState = eGameStates::ePlayLiveGame;
			ChangeGameState(eGameStates::ePlayLiveGame);
		}
	}
#endif

	if (myCurrentGameState == eGameStates::ePlayGameOver)
	{
		myTime += KE_GLOBAL::trueDeltaTime;

		if (myTime >= myGameOverSlowMoTime)
		{
			myTime = 0.0f;
			ChangeGameState(eGameStates::ePlayPresentScores);
		}
	}
	else if (myCurrentGameState == eGameStates::ePlayMatchEnd)
	{
		myTime += KE_GLOBAL::trueDeltaTime;

		if (myTime >= 1.75f)
		{
			myTime = 0.0f;
			ChangeGameState(eGameStates::ePlayPresentScores);
		}
	}
	else if (myCurrentGameState == eGameStates::ePlayLiveGame)
	{
		myTime += KE_GLOBAL::trueDeltaTime;

		if (myTime >= mySuddenDeathTime)
		{
			// SUDDEN DEATH!
		}
	}
}

void P8::GameManager::OnReceiveEvent(ES::Event& aEvent)
{
	if (P8::ChangeGameState* cgsMsg = dynamic_cast<P8::ChangeGameState*>(&aEvent))
	{
		ChangeGameState(cgsMsg->newGameState);
	}
	else if (P8::PlayerKilledEvent* pkMsg = dynamic_cast<P8::PlayerKilledEvent*>(&aEvent))
	{
		AssignScore(pkMsg);

		HandlePlayerDeath(pkMsg->myKilledPlayerIndex);
	}
	else if (P8::PlayerDataMessage* pdMsg = dynamic_cast<P8::PlayerDataMessage*>(&aEvent))
	{
		if (pdMsg->isNew)
		{
			myGameData.numberOfPlayers++;
			myGameData.scores[pdMsg->index] = 0;
			myGameData.playersAlive[pdMsg->index] = 0;
			return;
		}

		myGameData.numberOfPlayers--;
		myGameData.scores[pdMsg->index] = -1;
		myGameData.playersAlive[pdMsg->index] = -1;
	}
	else if (P8::PlayerModelDataEvent* pmdMsg = dynamic_cast<P8::PlayerModelDataEvent*>(&aEvent))
	{
		for (int i = 0; i < 4; i++)
		{
			myGameData.playerIDs[i] = pmdMsg->playerIndex[i];
			myGameData.modelIDs[i] = pmdMsg->modelIndex[i];
		}
	}
	else if (P8::StartingLevelEvent* sleMsg = dynamic_cast<P8::StartingLevelEvent*>(&aEvent))
	{
		myStartLevelIndex = sleMsg->myLevelIndex;
	}
	//else if (KE::PlayerEvent* playerEvent = dynamic_cast<KE::PlayerEvent*>(&aEvent))
	//{
	//	for (auto& interaction : playerEvent->interactions)
	//	{
	//		if (interaction.myInputType == KE::eInputType::Esc ||
	//			interaction.myInputType == KE::eInputType::XboxStart)
	//		{
	//			if (myCurrentGameState == eGameStates::ePlayLiveGame)
	//			{
	//				ChangeGameState(eGameStates::eMenuPause);
	//			}
	//			else if (myCurrentGameState == eGameStates::eMenuPause)
	//			{
	//				ChangeGameState(eGameStates::ePlayLiveGame);
	//			}
	//		}
	//	}
	//}

}

void P8::GameManager::OnSceneChange()
{

}

void P8::GameManager::ChangeGameState(const eGameStates aNewGameState)
{
	OurIsPaused = IsGamePaused(aNewGameState);

#ifdef _DEBUG
	if (myCurrentGameState == eGameStates::eDebugTesting) return;
#endif // _DEBUG

	if (myCurrentGameState == aNewGameState) { return; }

	std::cout << "Changed gamestate from: " << EnumToString(myCurrentGameState) << " | to: " << EnumToString(aNewGameState) << "\n";

	// Handle Exit logic before Enter Logic
	OnStateExit(myCurrentGameState);

	myOldGameState = myCurrentGameState;
	myCurrentGameState = aNewGameState;

	myTime = 0.0f;

	switch (myCurrentGameState)
	{
	case eGameStates::eMenuMain:
	{
		ResetGame();

		P8::ChangeSceneByBuildIndex sceneMsg(0);
		ES::EventSystem::GetInstance().SendEvent<P8::ChangeSceneByBuildIndex>(sceneMsg);

		KE::GlobalAudio::PlayMusic(sound::Music::Menu);

		// TODO Set scene to main menu scene? Does not happen now while pressing Menu from pause menu
		break;
	}
	case eGameStates::eMenuLobby:
	{
		P8::ChangeSceneByBuildIndex csbbiMSG(1);
		ES::EventSystem::GetInstance().SendEvent(csbbiMSG);
		break;
	}
	case eGameStates::eMenuSetting:
	{
		break;
	}
	case eGameStates::ePlayTutorial:
	{
		KE::GlobalAudio::PlaySFX(sound::SFX::Vignette);

		if (myStartLevelIndex < 0)
		{
			//ChangeGameState(eGameStates::ePlayTutorial);
			P8::ChangeSceneByBuildIndex csbbiMSG(2);
			ES::EventSystem::GetInstance().SendEvent(csbbiMSG);
		}
		else
		{
			P8::ChangeSceneByBuildIndex sceneMsg(myStartLevelIndex);
			ES::EventSystem::GetInstance().SendEvent<P8::ChangeSceneByBuildIndex>(sceneMsg);
			//ChangeGameState(eGameStates::ePlayLiveGame);
		}

		// wait for player input i GuiHandler to change to countdown
		break;
	}
	case eGameStates::eMenuPause:
	{
		break;
	}
	case eGameStates::eLoading:
	{
		// awake handles loading state changes
		// This depends on something else to trigger scene change

		ChangeSceneByBuildIndex csbbiMSG;
		ES::EventSystem::GetInstance().SendEvent(csbbiMSG);
		break;
	}
	case eGameStates::ePlayCountdown:
	{
		// Should be triggered from tutorial or loading
		ResetMatch();

		//auto* sceneManager = KE_GLOBAL::blackboard.Get<KE::SceneManager>();

		//ChangeSceneEvent changeSceneEvent(sceneManager->GetStartupScene()->sceneName);

		//ES::EventSystem::GetInstance().SendEvent(changeSceneEvent);
		break;
	}
	case eGameStates::ePlayLiveGame:
	{
		break;
	}
	case eGameStates::ePlayMatchEnd:
	{
		// trigger slowmotion and wait until slowmo is done, then change state to 


		P8::SlowMotionEvent slowMoEvent;
		ES::EventSystem::GetInstance().SendEvent<P8::SlowMotionEvent>(slowMoEvent);

		break;
	}
	case eGameStates::ePlayPresentScores:
	{
		P8::PresentScoreDataEvent scoreData;
		scoreData.scores[0] = myGameData.scores[0];
		scoreData.scores[1] = myGameData.scores[1];
		scoreData.scores[2] = myGameData.scores[2];
		scoreData.scores[3] = myGameData.scores[3];

		scoreData.playerID[0] = myGameData.playerIDs[0];
		scoreData.playerID[1] = myGameData.playerIDs[1];
		scoreData.playerID[2] = myGameData.playerIDs[2];
		scoreData.playerID[3] = myGameData.playerIDs[3];

		scoreData.modelID[0] = myGameData.modelIDs[0];
		scoreData.modelID[1] = myGameData.modelIDs[1];
		scoreData.modelID[2] = myGameData.modelIDs[2];
		scoreData.modelID[3] = myGameData.modelIDs[3];

		ES::EventSystem::GetInstance().SendEvent<P8::PresentScoreDataEvent>(scoreData);
		// Wait for GUIHandler to send Loading state change
		// with either empty "ChangeSceneByBuildIndex" event or if going to main menu: set index to 0

		break;
	}
	case eGameStates::ePlayGameOver:
	{
		// A player won game and game is over,
		// present score a final time and change state to main menu after

		KE::GlobalAudio::PlaySFX(sound::SFX::AnnouncerWinner);

		P8::SlowMotionEventData data;
		data.resetCurvePower = 1;
		data.timeDelayUntilReset = myGameOverSlowMoTime;
		data.resetTime = 0.0f;
		P8::SlowMotionEvent slowMoEvent(data);
		ES::EventSystem::GetInstance().SendEvent<P8::SlowMotionEvent>(slowMoEvent);

		break;
	}
	case eGameStates::eWinScreen:
	{
		KE::GlobalAudio::PlaySFX(sound::SFX::AnnouncerChampion);
		

		P8::ChangeSceneByBuildIndex csbbiMSG(3);
		ES::EventSystem::GetInstance().SendEvent(csbbiMSG);

		break;
	}
	case eGameStates::eDebugTesting:
	{
		ResetGame();
		break;
	}
	default:
		break;
	}

	P8::GameStateHasChanged msg(myCurrentGameState);
	ES::EventSystem::GetInstance().SendEvent(msg);
}

void P8::GameManager::OnStateExit(const eGameStates aOldGameState)
{
	switch (aOldGameState)
	{
	case eGameStates::eMenuMain:
	{
		break;
	}
	{
		break;
	}
	case eGameStates::eMenuLobby:
	{
		break;
	}
	case eGameStates::eMenuSetting:
	{
		break;
	}
	case eGameStates::ePlayTutorial:
	{
		break;
	}
	case eGameStates::ePlayCountdown:
	{
		break;
	}
	case eGameStates::ePlayLiveGame:
	{
		break;
	}
	case eGameStates::ePlayMatchEnd:
	{

		break;
	}
	case eGameStates::ePlayPresentScores:
	{
		break;
	}
	case eGameStates::eWinScreen:
	{
		KE::GlobalAudio::PlaySFX(sound::SFX::Vignette);
		break;
	}
	case eGameStates::ePlayGameOver:
	{
		
		break;
	}
	default:
		break;
	}
}

void P8::GameManager::AssignScore(const P8::PlayerKilledEvent* someData)
{
	if (someData->myKilledPlayerIndex == someData->myMurdererIndex)
	{
		// Handle if player killed itself
		return;
	}

	myGameData.scores[someData->myMurdererIndex] += myGameData.pointsPerKill;

	std::cout << "Player " << someData->myMurdererIndex << " killed player " << someData->myKilledPlayerIndex << " and has a point total of [" << myGameData.scores[someData->myMurdererIndex] << "] / " << myGameData.maxPoints << std::endl;

	if (myGameData.scores[someData->myMurdererIndex] >= myGameData.maxPoints)
	{
		// Handle Win
		ChangeGameState(P8::eGameStates::ePlayGameOver);
	}

}

void P8::GameManager::HandlePlayerDeath(const int aDeadPlayerIndex)
{
	myGameData.playersAlive[aDeadPlayerIndex] = 0;

	int playersAlive = 0;
	for (unsigned int i = 0; i < 4; i++)
	{
		if (myGameData.playersAlive[i] > 0)
		{
			playersAlive++;
		}
	}

	if (myCurrentGameState != eGameStates::ePlayGameOver && playersAlive <= 1)
	{
		ResetMatch();
		ChangeGameState(eGameStates::ePlayMatchEnd);
	}
}

void P8::GameManager::ResetMatch()
{
	for (int i = 0; i < myGameData.numberOfPlayers; i++)
	{
		if (myGameData.playersAlive[i] == 0)
		{
			myGameData.playersAlive[i] = 1;
		}
	}
}

void P8::GameManager::ResetGame()
{
	myGameData.numberOfPlayers = 0;
	for (unsigned int i = 0; i < 4; i++)
	{
		// Reset values 
		myGameData.scores[i] = -1;
		myGameData.playersAlive[i] = -1;
		myGameData.playerIDs[i] = -1;
		myGameData.modelIDs[i] = -1;
	}
}

void P8::GameManager::OnInit()
{
	ES::EventSystem::GetInstance().Attach<P8::ChangeGameState>(this);
	ES::EventSystem::GetInstance().Attach<P8::PlayerKilledEvent>(this);
	ES::EventSystem::GetInstance().Attach<P8::PlayerDataMessage>(this);
	ES::EventSystem::GetInstance().Attach<P8::PlayerModelDataEvent>(this);
	ES::EventSystem::GetInstance().Attach<P8::StartingLevelEvent>(this);
	//ES::EventSystem::GetInstance().Attach<KE::PlayerEvent>(this);
}

void P8::GameManager::OnDestroy()
{
	ES::EventSystem::GetInstance().Detach<P8::ChangeGameState>(this);
	ES::EventSystem::GetInstance().Detach<P8::PlayerKilledEvent>(this);
	ES::EventSystem::GetInstance().Detach<P8::PlayerDataMessage>(this);
	ES::EventSystem::GetInstance().Detach<P8::PlayerModelDataEvent>(this);
	ES::EventSystem::GetInstance().Detach<P8::StartingLevelEvent>(this);
	//ES::EventSystem::GetInstance().Detach<KE::PlayerEvent>(this);
}

void P8::GameManager::DEBUGIncrementLevelTest()
{
	ChangeSceneByBuildIndex csbbiMSG;
	ES::EventSystem::GetInstance().SendEvent(csbbiMSG);
}
