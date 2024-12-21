#pragma once
#include <Engine/Source/ComponentSystem/Components/Component.h>
#include <Engine/Source/Utility/EventSystem.h>
#include <Engine/Source/Input/InputEvents.h>
#include <map>
#include <bitset>

namespace P8
{
	class Player;

	struct PlayerSlotInfo
	{
		bool isActive		= false;
		bool isReady		= false;
		int playerID		= -1;
		int controllerID	= -1;
		int modelID			= -1;
		unsigned int team	= 999;
		int powerupMask		= 0;
	};

	enum class Cheats
	{
		God,
		powerUp1,	// Teleport
		powerUp2,	// Dash through Walls
		powerUp3,	// Split Boomerang
		powerUp4,	// Shield
		powerUp5,	// Double Trouble
		powerUp6,	// Hyped

		Count,
	};


	struct InputBinding
	{
		std::vector<KE::eInputType> myInputs;

		//KE::eInputType keyboardInput;
		//KE::eInputType gamepadInput;

		bool operator==(const KE::eInputType aInput)
		{
			for (int i = 0; i < myInputs.size(); i++)
			{
				if (myInputs[i] == aInput)
				{
					return true;
				}
			}

			return false;
		}
	};

	struct CheatCode
	{
		std::vector<InputBinding> cheatcodeList;
		int validInputStreak = 0;

		bool isActive = false;
	};

	class LobbySystem : public KE::Component, ES::IObserver
	{
		KE_EDITOR_FRIEND

	public:
		LobbySystem(KE::GameObject& aGameObject);
		~LobbySystem();

		void Awake() override;
		void Update() override;
		void OnSceneChange() override;

		void ResetLobby();
	private:
		P8::Player* AddPlayer(PlayerSlotInfo* someData = nullptr);
		P8::Player* AddPlayerWithController(KE::Interaction& aInteraction);
		void RemovePlayer(const unsigned int anID);

		bool CheckIfValidModel(const int aModelIndex);

		const int GiveValidModelIndex();
		bool IsControllerRegistered(const unsigned int aControllerID);
		void HandleControllerInput(KE::Interaction& aInteraction);
		const int FindPlayerByControllerID(const int aControllerID);
		void ChangeCharacter(KE::Interaction& aInteraction, bool aMoveRight);
		void SetPlayerNotReady(KE::Interaction& aInteraction);
		void SetPlayerReady(KE::Interaction& aInteraction);

		bool AreAllPlayersReady();
		bool IsAnyPlayerActive();
		bool IsControllerActive(const int aControllerID);

		int GetValidCharacterIndex(P8::Player* aPlayer);
		int GetNextValidCharacterIndex(P8::Player* aPlayer, bool aMoveRight);

		// Inherited via IObserver
		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		void OnDestroy() override;



		/// Cheat Funcs
		void ActivateGodCheat(const int aControllerID);
		void ActivateCheat(const Cheats aCheat);

		// Key = Player Index 
		std::unordered_map<unsigned int, P8::Player*> myPlayers;

		int myNumberOfModels = 4;

		int myNumberOfOrdinaryModels = 4;
		const static int myMaxNumberOfPlayers = 4;
		std::array<PlayerSlotInfo, myMaxNumberOfPlayers> myPlayerSlots;

		bool isLobbyActive = false;
		bool isBlockingInputTimed = true;

		bool isCheatLobbyActive = false;
		int myNumberofCheatModels = 5;
		int myCheatPowerupMask = 0;

		std::map<Cheats, CheatCode> myCheatcodes;

		inline static const float freq = 5;
	};
}

