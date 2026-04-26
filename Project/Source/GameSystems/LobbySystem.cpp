#include "stdafx.h"
#include "LobbySystem.h"
#include <Engine/Source/SceneManagement/Scene.h>
#include <Engine/Source/LevelImporter/PrefabHandler.h>
#include "Project/Source/Player/Player.h"
#include "Project/Source/GameEvents/GameEvents.h"
#include "Engine/Source/Audio/GlobalAudio.h"

P8::LobbySystem::LobbySystem(KE::GameObject& aGameObject) : KE::Component(aGameObject)
{
	OnInit();

	InputBinding upAction; 
	InputBinding downAction; 
	InputBinding aAction;
	InputBinding bAction;
	InputBinding xAction;
	InputBinding yAction;

	downAction.myInputs = { KE::eInputType::Down, KE::eInputType::XboxDPadDown, KE::eInputType::XboxLeftTriggerToggledDown };
	upAction.myInputs = { KE::eInputType::Up, KE::eInputType::XboxDPadUp, KE::eInputType::XboxLeftTriggerToggledUp };

	aAction.myInputs = { KE::eInputType::Space, KE::eInputType::XboxA };
	bAction.myInputs = { KE::eInputType::Action2, KE::eInputType::XboxB };
	xAction.myInputs = { KE::eInputType::Action3, KE::eInputType::XboxX };
	yAction.myInputs = { KE::eInputType::Action1, KE::eInputType::XboxY };


	// Creates and assigns cheats with inputs
	myCheatcodes[Cheats::God].cheatcodeList =
	{
		upAction,
		upAction,
		upAction,
		upAction,
		downAction
	};

	myCheatcodes[Cheats::powerUp1].cheatcodeList =
	{
		upAction,
		downAction,
		upAction,
		downAction,
		upAction,
	};

	myCheatcodes[Cheats::powerUp2].cheatcodeList =
	{
		upAction,
		downAction,
		upAction,
		downAction,
		downAction,
	};

	myCheatcodes[Cheats::powerUp3].cheatcodeList =
	{
		upAction,
		downAction,
		upAction,
		downAction,
		aAction,
	};

	myCheatcodes[Cheats::powerUp4].cheatcodeList =
	{
		upAction,
		downAction,
		upAction,
		downAction,
		bAction,
	};

	myCheatcodes[Cheats::powerUp5].cheatcodeList =
	{
		upAction,
		downAction,
		upAction,
		downAction,
		yAction,
	};

	myCheatcodes[Cheats::powerUp6].cheatcodeList =
	{
		upAction,
		downAction,
		upAction,
		downAction,
		xAction,
	};
}

P8::LobbySystem::~LobbySystem()
{
	myPlayers.clear();

	OnDestroy();
}

void P8::LobbySystem::Awake()
{
	for (int i = 0; i < 4; i++)
	{
		if (myPlayerSlots[i].isActive)
		{
			Player* newPlayer = AddPlayer(&myPlayerSlots[i]);

			newPlayer->AssignController(myPlayerSlots[i].controllerID);
		}
	}
}

void P8::LobbySystem::Update()
{
	if (isBlockingInputTimed)
	{
		isBlockingInputTimed = false;
		isLobbyActive = true;
	}
#ifndef KITTYENGINE_SHIP
	if (GetAsyncKeyState('P') == SHRT_MIN + 1)
	{
		if (Player* newPlayer = AddPlayer())
		{
			newPlayer->SetPosition({ (float)myPlayers.size(), 4.0f, (float)myPlayers.size() });
		}
	}
#endif
}

void P8::LobbySystem::OnSceneChange()
{
	for (int i = 0; i < 4; i++)
	{
		PlayerSlotInfo& slotInfo = myPlayerSlots[i];
		if (!slotInfo.isActive) { continue; }

		Player* player = myPlayers[slotInfo.playerID];
		slotInfo.powerupMask = player->GetPowerups().GetPowerupMask();
	}

	myPlayers.clear();
}

void P8::LobbySystem::ResetLobby()
{
	for (int i = 0; i < 4; i++)
	{
		if (myPlayerSlots[i].isActive)
		{
			RemovePlayer(myPlayerSlots[i].playerID);
		}
	}

	myCheatPowerupMask = 0;

	for (auto& cheat : myCheatcodes)
	{
		cheat.second.isActive = false;
		cheat.second.validInputStreak = 0;
	}
}

P8::Player* P8::LobbySystem::AddPlayer(PlayerSlotInfo* someData)
{
	if (myPlayers.size() >= myNumberOfModels)
	{
		KE_LOG("Trying to add more players even though there are 4");
		return nullptr;
	}

	KE::GameObject* player = KE::Scene::myPrefabHandler->Instantiate("Player");

	if (!player)
	{
		KE_LOG("Failed to instantiate Player prefab");
		return nullptr;
	}

	P8::Player* playerComp = &player->GetComponent<P8::Player>();

	KE::GameObject* managers = player->GetManager().GetGameObject(KE::ReservedGameObjects::eGameSystemManager);

	PowerupManager* powerupManager = &managers->GetComponent<PowerupManager>();

	if (someData == nullptr)
	{
		for (unsigned int i = 0; i < myMaxNumberOfPlayers; i++)
		{
			// If slot is not in use, assign new player to this slot
			if (myPlayerSlots[i].isActive == false)
			{
				playerComp->SetPlayerIndex(i);
				playerComp->AssignTeam(i);

				break;
			}
		}

		myPlayers[playerComp->GetIndex()] = playerComp;

		const int modelIndex = GiveValidModelIndex();

		if (modelIndex > 0)
		{
			myPlayerSlots[static_cast<int>(playerComp->GetIndex())].modelID = modelIndex;
			playerComp->SetCharacterIndex(modelIndex);
		}

		auto& playerData = myPlayerSlots[playerComp->GetIndex()];
		playerData.isActive = true;
		playerData.modelID = modelIndex;
		playerData.playerID = playerComp->GetIndex();
		playerData.team = playerComp->GetTeam();

		if (myCheatPowerupMask > 0)
		{
			auto& powerups = playerComp->GetPowerups();
			powerups.manager = powerupManager;
			powerups.SetPowerupsFromMask(myCheatPowerupMask);
		}
	}
	else
	{
		playerComp->SetCharacterIndex(someData->modelID);
		playerComp->SetPlayerIndex(someData->playerID);
		playerComp->AssignTeam(someData->team);
		myPlayers[playerComp->GetIndex()] = playerComp;


		int powerupmask = someData->powerupMask | myCheatPowerupMask;

		auto& powerups = playerComp->GetPowerups();
		powerups.manager = powerupManager;
		powerups.SetPowerupsFromMask(powerupmask);
	}

	//std::cout << "\nPlayer " << playerComp->GetIndex() << " added\n";

	return playerComp;
}

P8::Player* P8::LobbySystem::AddPlayerWithController(KE::Interaction& aInteraction)
{
	if (auto* player = AddPlayer())
	{
		player->SetPosition({ (float)player->GetIndex(), 4.0f, (float)player->GetIndex() });
		player->AssignController(aInteraction.myControllerID);
		myPlayerSlots[player->GetIndex()].controllerID = aInteraction.myControllerID;

		LobbyEvent lobbyEvent(eLobbyEvents::PlayerJoined, player->GetIndex());
		ES::EventSystem::GetInstance().SendEvent(lobbyEvent);

		KE::GlobalAudio::PlaySFX(sound::SFX::MenuSelect);

		player->SetPlayerState<PlayerLobbyState>();

		return player;

		// Old down below

		//unsigned int index = player->GetIndex();
		//myPlayers[index]->SetPosition({ (float)myPlayers.size(), 4.0f, (float)myPlayers.size() });
		//myPlayers[index]->AssignController(aInteraction.myControllerID);
		//myRegisteredControllers[aInteraction.myControllerID] = myPlayers[index];
		//std::cout << "\nPlayer " << index << " added with controller ID: " << aInteraction.myControllerID;

		//LobbyEvent lobbyEvent(eLobbyEvents::PlayerJoined, index);
		//ES::EventSystem::GetInstance().SendEvent(lobbyEvent);

		//player->SetPlayerState<PlayerLobbyState>();

		//return myPlayers[index];
	}
	else
	{
		KE_LOG("Failed to add player");
		return nullptr;
	}
}

void P8::LobbySystem::RemovePlayer(const unsigned int anID)
{
	if (myPlayers.count(anID) == 0) return;

	myPlayerSlots[anID] = PlayerSlotInfo();

	KE::GameObject* go = &myPlayers[anID]->GetGameObject();

	go->GetManager().DestroyGameObject(go->myID);

	go = nullptr;
	myPlayers.erase(anID);

	// tells gamemanager that a player left
	//P8::PlayerDataMessage pdmsg(anID, false);
	//ES::EventSystem::GetInstance().SendEvent<P8::PlayerDataMessage>(pdmsg);

	// Tells lobby that a player left
	LobbyEventData lobbyEventData;

	//for (int i = 0; i < myNumberOfModels; i++)
	//{

	//}


	for (int i = 0; i < myMaxNumberOfPlayers; i++)
	{
		if (myPlayerSlots[i].modelID > 0)
		{
			lobbyEventData.mySelectedCharactersPlayerIndices[myPlayerSlots[i].modelID - 1] = i;
		}
	}

	P8::LobbyEvent lemsg(eLobbyEvents::PlayerRemoved, anID, &lobbyEventData);
	ES::EventSystem::GetInstance().SendEvent(lemsg);
}

bool P8::LobbySystem::CheckIfValidModel(const int aModelIndex)
{
	if (aModelIndex > myNumberOfModels) return false;

	for (int i = 0; i < myMaxNumberOfPlayers; i++)
	{
		if (aModelIndex == myPlayerSlots[i].modelID) return false;
	}

	return true;
}

const int P8::LobbySystem::GiveValidModelIndex()
{
	int arr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

	for (int i = 0; i < myNumberOfModels; i++)
	{
		bool isFound = false;
		for (int j = 0; j < myMaxNumberOfPlayers; j++)
		{
			if (myPlayerSlots[j].modelID != arr[i]) continue;

			isFound = true;
		}

		if (!isFound)
		{
			return arr[i];
		}
	}

	return -1;
}

bool P8::LobbySystem::IsControllerRegistered(const unsigned int aControllerID)
{
	for (int i = 0; i < myMaxNumberOfPlayers; i++)
	{
		if (myPlayerSlots[i].controllerID == (int)aControllerID)
		{
			return true;
		}
	}

	//for (auto& controller : myRegisteredControllers)
	//{
	//	if (controller.first == (int)aControllerID)
	//	{
	//		return true;
	//	}
	//}
	return false;
}

void P8::LobbySystem::HandleControllerInput(KE::Interaction& aInteraction)
{
	// Process cheats
	if (aInteraction.myInteractionType == KE::eInteractionType::Pressed)
	{
		for (int i = 0; i < myCheatcodes.size(); i++)
		{
			CheatCode& cheat = myCheatcodes.at((Cheats)i);

			if (cheat.isActive == false)
			{
				if (cheat.cheatcodeList[cheat.validInputStreak] != aInteraction.myInputType)
				{

					cheat.validInputStreak = 0;
					continue;
				}

				cheat.validInputStreak++;

				if (cheat.validInputStreak >= cheat.cheatcodeList.size())
				{
					cheat.isActive = true;

					if ((Cheats)i == Cheats::God)
					{
						std::cout << "GOD CHEAT ACTIVE\n";

						ActivateGodCheat(aInteraction.myControllerID);
					}
					else
					{
						ActivateCheat((Cheats)i);
					}
				}
			}
		}
	}


	if (aInteraction.myInteractionType == KE::eInteractionType::Pressed)
	{
		if (aInteraction.myInputType == KE::eInputType::XboxA ||
			aInteraction.myInputType == KE::eInputType::Action1)
		{
			const int playerID = FindPlayerByControllerID(aInteraction.myControllerID);

			if (!IsControllerRegistered(aInteraction.myControllerID))
			{
				if (AreAllPlayersReady())
				{
					LobbyEvent lobbyEvent(eLobbyEvents::AllNotReady, -1);
					ES::EventSystem::GetInstance().SendEvent(lobbyEvent);
				}

				Player* player = AddPlayerWithController(aInteraction);

				if (player == nullptr)
				{
					return;
				}

				// Assign character to player
				// Maybe redundant???? -- Olle

				//int validCharacterIndex = GetValidCharacterIndex(player);
				//myPlayerSlots[i].modelID = i + 1;
				//aPlayer->SetCharacterIndex(i + 1)

				//if (validCharacterIndex < 0)
				//{
				//	KE_LOG("Failed to assign character to player");
				//	return;
				//}

				LobbyEventData lobbyEventData;

				for (int i = 0; i < myMaxNumberOfPlayers; i++)
				{
					if (myPlayerSlots[i].modelID > 0)
					{
						lobbyEventData.mySelectedCharactersPlayerIndices[myPlayerSlots[i].modelID - 1] = i;
					}
				}

				LobbyEvent lobbyEvent(eLobbyEvents::CharacterSelection, player->GetCharacterIndex(), &lobbyEventData);
				ES::EventSystem::GetInstance().SendEvent(lobbyEvent);
			}
			else if (myPlayerSlots[playerID].isReady == false
				/*!myReadyPlayers.test(myRegisteredControllers[aInteraction.myControllerID]->GetIndex())*/)
			{
				SetPlayerReady(aInteraction);
			}

			if (AreAllPlayersReady())
			{
				LobbyEvent lobbyEvent(eLobbyEvents::AllReady, -1);
				ES::EventSystem::GetInstance().SendEvent(lobbyEvent);
				//std::cout << "\nAll players are ready! -- LobbySystem";
			}
		}
		else if (aInteraction.myInputType == KE::eInputType::XboxB ||
			aInteraction.myInputType == KE::eInputType::Action2 ||
			aInteraction.myInputType == KE::eInputType::Esc)
		{
			if (aInteraction.myControllerID > -1)
			{
				for (int i = 0; i < myMaxNumberOfPlayers; i++)
				{
					if (myPlayerSlots[i].controllerID == aInteraction.myControllerID)
					{
						if (myPlayerSlots[i].isReady == false)
						{
							RemovePlayer(myPlayerSlots[i].playerID);

							if (AreAllPlayersReady())
							{
								LobbyEvent lobbyEvent(eLobbyEvents::AllReady, -1);
								ES::EventSystem::GetInstance().SendEvent(lobbyEvent);
							}

							return;
						}
					}
				}
			}

			if (!IsAnyPlayerActive())
			{
				ChangeGameState changeGameStateEvent(eGameStates::eMenuMain);
				ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);
				isLobbyActive = false;
				return;
			}
			else if (aInteraction.myInputType != KE::eInputType::Esc)
			{
				if (!IsControllerRegistered(aInteraction.myControllerID))
				{
					return;
				}
				if (AreAllPlayersReady())
				{
					LobbyEvent lobbyEvent(eLobbyEvents::AllNotReady, -1);
					ES::EventSystem::GetInstance().SendEvent(lobbyEvent);
				}
				SetPlayerNotReady(aInteraction);
			}

		}
		else if (aInteraction.myInputType == KE::eInputType::XboxDPadLeft ||
			aInteraction.myInputType == KE::eInputType::XboxLeftTriggerToggledLeft ||
			aInteraction.myInputType == KE::eInputType::Left)
		{
			if (!IsControllerRegistered(aInteraction.myControllerID))
			{
				return;
			}
			bool moveRight = false;

			ChangeCharacter(aInteraction, moveRight);
		}
		else if (aInteraction.myInputType == KE::eInputType::XboxDPadRight ||
			aInteraction.myInputType == KE::eInputType::XboxLeftTriggerToggledRight ||
			aInteraction.myInputType == KE::eInputType::Right)
		{
			if (!IsControllerRegistered(aInteraction.myControllerID))
			{
				return;
			}
			bool moveRight = true;

			ChangeCharacter(aInteraction, moveRight);
		}
		/// TODO TEMP TEMP TEMP TEMP TEMP TEMP
		else if (aInteraction.myInputType == KE::eInputType::XboxStart ||
			aInteraction.myInputType == KE::eInputType::Enter)
		{
			if (isLobbyActive && AreAllPlayersReady() /*&& IsControllerActive(aInteraction.myControllerID)*/)
			{
				P8::PlayerModelDataEvent pmdmsg;

				for (int i = 0; i < myMaxNumberOfPlayers; i++)
				{
					pmdmsg.playerIndex[i] = myPlayerSlots[i].playerID;
					pmdmsg.modelIndex[i] = myPlayerSlots[i].modelID;
				}

				ES::EventSystem::GetInstance().SendEvent<P8::PlayerModelDataEvent>(pmdmsg);

				for (auto& player : myPlayers)
				{
					P8::PlayerDataMessage pdmsg(player.second->GetIndex(), true);
					ES::EventSystem::GetInstance().SendEvent<P8::PlayerDataMessage>(pdmsg);
				}


				LobbyEvent lobbyEvent(eLobbyEvents::GameStart, -1);
				ES::EventSystem::GetInstance().SendEvent(lobbyEvent);

				ChangeGameState changeGameStateEvent(eGameStates::ePlayTutorial);
				ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);

				//ChangeSceneByBuildIndex csbbiMSG(2);
				//ES::EventSystem::GetInstance().SendEvent(csbbiMSG);
				isLobbyActive = false;
			}
		}
		/// TODO TEMP TEMP TEMP TEMP TEMP TEMP
	/*	else if(aInteraction.myInputType == KE::eInputType::XboxBack ||)*/



	}

}

const int P8::LobbySystem::FindPlayerByControllerID(const int aControllerID)
{
	for (int i = 0; i < myMaxNumberOfPlayers; i++)
	{
		if (aControllerID == myPlayerSlots[i].controllerID) return myPlayerSlots[i].playerID;
	}

	return -1;
}

void P8::LobbySystem::ChangeCharacter(KE::Interaction& aInteraction, bool aMoveRight)
{
	int playerIndex = FindPlayerByControllerID(aInteraction.myControllerID);

	if (playerIndex < 0) return;

	PlayerSlotInfo& playerData = myPlayerSlots[playerIndex];

	if (playerData.isReady) return;

	int oldModelIndex = playerData.modelID;
	int newModelIndex = GetNextValidCharacterIndex(myPlayers.at(playerIndex), aMoveRight);
	if (newModelIndex > 0)
	{
		// Todo 
		// Set modelData in here, not in func over this
		myPlayers.at(playerIndex)->SetCharacterIndex(newModelIndex);
		myPlayerSlots[playerIndex].modelID = newModelIndex;

		//std::cout << "Changed model to : " << newModelIndex << "\n";

		LobbyEventData lobbyEventData;

		for (int i = 0; i < myMaxNumberOfPlayers; i++)
		{
			if (myPlayerSlots[i].modelID > 0)
			{
				lobbyEventData.mySelectedCharactersPlayerIndices[myPlayerSlots[i].modelID - 1] = i;
			}
		}

		LobbyEvent lobbyEvent(eLobbyEvents::CharacterSelection, playerIndex, &lobbyEventData);
		ES::EventSystem::GetInstance().SendEvent(lobbyEvent);
	}
	else
	{
		KE_LOG("Failed to get next valid character index");
	}
}

void P8::LobbySystem::SetPlayerNotReady(KE::Interaction& aInteraction)
{
	if (IsControllerRegistered(aInteraction.myControllerID))
	{
		auto& playerSlot = myPlayerSlots[FindPlayerByControllerID(aInteraction.myControllerID)];

		if (playerSlot.isReady)
		{
			playerSlot.isReady = false;

			LobbyEvent lobbyEvent(eLobbyEvents::PlayerNotReady, playerSlot.playerID);
			ES::EventSystem::GetInstance().SendEvent(lobbyEvent);
		}
	}
}

void P8::LobbySystem::SetPlayerReady(KE::Interaction& aInteraction)
{
	auto& playerSlot = myPlayerSlots[FindPlayerByControllerID(aInteraction.myControllerID)];

	playerSlot.isReady = true;

	LobbyEvent lobbyEvent(eLobbyEvents::PlayerReady, playerSlot.playerID);
	ES::EventSystem::GetInstance().SendEvent(lobbyEvent);
}

bool P8::LobbySystem::AreAllPlayersReady()
{
	bool allReady = true;
	int counter = 0;
	for (int i = 0; i < myMaxNumberOfPlayers; i++)
	{
		if (myPlayerSlots[i].isReady == false)
		{
			if (!myPlayerSlots[i].isActive)
			{
				counter++;
				continue;
			}
			allReady = false;
			break;
		}
	}

#ifndef _DEBUG
	// Prevents one player from starting the game in anything but DEBUG mode
	if (counter == 3)
	{
		return false;
	}
#endif

	if (counter == 4)
	{
		return false;
	}
	return allReady;
}

bool P8::LobbySystem::IsAnyPlayerActive()
{
	for (int i = 0; i < myMaxNumberOfPlayers; i++)
	{
		if (myPlayerSlots[i].isActive)
		{
			return true;
		}
	}
	return false;
}

bool P8::LobbySystem::IsControllerActive(const int aControllerID)
{
	if (aControllerID < 0)
	{
		return false;
	}
	for (int i = 0; i < myMaxNumberOfPlayers; i++)
	{
		if (myPlayerSlots[i].controllerID == aControllerID)
		{
			return true;
		}
	}
	return false;
}

int P8::LobbySystem::GetValidCharacterIndex(P8::Player* aPlayer)
{
	for (int i = 0; i < myMaxNumberOfPlayers; i++)
	{
		if (myPlayerSlots[i].isActive == false)
		{
			return i;
		}
	}
	return -1;
}

int P8::LobbySystem::GetNextValidCharacterIndex(P8::Player* aPlayer, bool aMoveRight)
{
	// Character index starts at 1 and not 0
	const int characterIndex = aPlayer->GetCharacterIndex();

	int counter = myNumberOfModels;
	int newModelIndex = characterIndex;
	if (aMoveRight)
	{
		newModelIndex = characterIndex + 1 > myNumberOfModels ? 1 : characterIndex + 1;
	}
	else
	{
		newModelIndex = characterIndex - 1 < 1 ? myNumberOfModels : characterIndex - 1;
	}

	while (counter >= 0)
	{
		if (CheckIfValidModel(newModelIndex))
		{
			return newModelIndex;
		}

		if (aMoveRight)
		{
			newModelIndex = newModelIndex + 1 > myNumberOfModels ? 1 : newModelIndex + 1;
		}
		else
		{
			newModelIndex = newModelIndex - 1 < 1 ? myNumberOfModels : newModelIndex - 1;
		}

		counter--;
	}


	return -1;
}

void P8::LobbySystem::OnReceiveEvent(ES::Event& aEvent)
{
	if (GameStateHasChanged* gameStateEvent = dynamic_cast<GameStateHasChanged*>(&aEvent))
	{
		isBlockingInputTimed = gameStateEvent->newGameState == eGameStates::eMenuLobby;

		if (gameStateEvent->newGameState == eGameStates::eMenuMain)
		{
			ResetLobby();
			//for (int i = 0; i < 4; i++)
			//{
			//	myPlayerSlots[i] = PlayerSlotInfo();
			//}
		}
	}

	if (KE::PlayerEvent* playerEvent = dynamic_cast<KE::PlayerEvent*>(&aEvent))
	{
		if (!isActive)
		{
			return;
		}
		if (!isLobbyActive)
		{
			return;
		}

		for (auto& interaction : playerEvent->interactions)
		{
			HandleControllerInput(interaction);
		}
	}
}

void P8::LobbySystem::OnInit()
{
	ES::EventSystem::GetInstance().Attach<KE::PlayerEvent>(this);
	ES::EventSystem::GetInstance().Attach<OnLevelLoadedEvent>(this);
	ES::EventSystem::GetInstance().Attach<GameStateHasChanged>(this);
}

void P8::LobbySystem::OnDestroy()
{
	ES::EventSystem::GetInstance().Detach<KE::PlayerEvent>(this);
	ES::EventSystem::GetInstance().Detach<OnLevelLoadedEvent>(this);
	ES::EventSystem::GetInstance().Detach<GameStateHasChanged>(this);
}

void P8::LobbySystem::ActivateGodCheat(const int aControllerID)
{
	KE::GlobalAudio::PlaySFX(sound::SFX::GodActive);

	isCheatLobbyActive = true;

	myNumberOfModels = myNumberofCheatModels;

	if (myPlayers.size() > 0)
	{
		for (int i = 0; i < myMaxNumberOfPlayers; i++)
		{
			if (myPlayerSlots[i].controllerID == aControllerID)
			{
				myPlayerSlots[i].modelID = 5;
				myPlayers.at(myPlayerSlots[i].playerID)->SetCharacterIndex(5);
				return;
			}
		}

	}
}

void P8::LobbySystem::ActivateCheat(const P8::Cheats aCheat)
{
	KE::GlobalAudio::PlaySFX(sound::SFX::MenuSelect);

	switch (aCheat)
	{
	case Cheats::powerUp1:
	{
		std::cout << "TELEPORT CHEAT ACTIVE\n";
		myCheatPowerupMask = myCheatPowerupMask | 1 << 1;
		break;
	}
	case Cheats::powerUp2:
	{
		std::cout << "DASH THROUGH WALL CHEAT ACTIVE\n";
		myCheatPowerupMask = myCheatPowerupMask | 1 << 2;

		break;
	}
	case Cheats::powerUp3:
	{
		std::cout << "SPLIT CHEAT ACTIVE\n";
		myCheatPowerupMask = myCheatPowerupMask | 1 << 3;

		break;
	}
	case Cheats::powerUp4:
	{
		std::cout << "SHIELD CHEAT ACTIVE\n";
		myCheatPowerupMask = myCheatPowerupMask | 1 << 4;

		break;
	}
	case Cheats::powerUp5:
	{
		std::cout << "DOUBLE TROUBLE CHEAT ACTIVE\n";
		myCheatPowerupMask = myCheatPowerupMask | 1 << 5;

		break;
	}
	case Cheats::powerUp6:
	{
		std::cout << "HYPED CHEAT ACTIVE\n";
		myCheatPowerupMask = myCheatPowerupMask | 1 << 6;

		break;
	}
	default:
		break;
	}

	for (int i = 0; i < myMaxNumberOfPlayers; i++)
	{
		if (myPlayerSlots[i].isActive)
		{
			P8::Player* player = myPlayers.at(myPlayerSlots[i].playerID);

			int powerupmask = myPlayerSlots[i].powerupMask | myCheatPowerupMask;

			auto& powerups = player->GetPowerups();
			powerups.SetPowerupsFromMask(powerupmask);
		}
	}

	std::cout << "Powerupmask val = " << myCheatPowerupMask << "\n";
}
