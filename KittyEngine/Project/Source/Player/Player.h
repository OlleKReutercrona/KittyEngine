#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Utility/EventSystem.h"
#include "Engine/Source/Input/Input.h"
#include "Project/Source/Managers/GameManager.h"

#include "Project/Source/Player/PlayerAnimationController.h"
#include "Project/Source/Player/States/PlayerStateBase.h"
#include "Project/Source/Powerups/Powerup.h"
#include "Project/Source/Boomerang/BallThrowParams.h"

namespace KE {

	class PhysicsCharacterController;
	struct PlayerEvent;

	namespace Collision {
		enum class Layers;
	}
}

namespace P8
{
	enum class ActionState;
	enum class PlayerActionType;
	class Powerup;
	class BoomerangComponent;
	class PlayerIdleState;
	class PlayerMoveState;
	class BallManager;
	class PlayerDashState;
	struct BallThrowParams;
	struct PowerupInputData;

	struct PlayerRenderBuffer
	{
		float timeInvulnerable = -1.0f;
		float timeOutsideBattleArea = -1.0f;

		float padding1;
		float padding2;
	};

	struct PlayerData
	{
		int characterIndex = 0;
		int boomerangGoID = INT_MAX;
		KE::PhysicsCharacterController* controller = nullptr;
		Vector3f spawnPosition = {};
		float maxSpeed = 0.0f;
		float movementSpeed = 0.0f;
		float maxRotation = 0.0f;
		float rotationSpeed = 0.0f;
	};
	enum class eInputAction
	{
		Move,
		Throw,
		Dash,
		Attack,
		Taunt
	};
	enum class PlayerState
	{
		Idle,
		Moving,
		PreDashing,
		Dashing,
		Windup,
		Throwing,
		Recalling,
		Attacking,
		Dead,

		Count
	};
	struct OwnedBoomerangs
	{
		bool throwingBlocked = false; // Useful for powerups and stuff.
		std::vector<BoomerangComponent*> heldBoomerangs;
		std::vector<BoomerangComponent*> thrownBoomerangs;

		void RegisterHeld(BoomerangComponent* aBoomerang);
		void RegisterThrown(BoomerangComponent* aBoomerang);
		void DeregisterHeld(BoomerangComponent* aBoomerang);
		void DeregisterThrown(BoomerangComponent* aBoomerang);

		void Update();
		void Pickup(BoomerangComponent* aBoomerang);
		void Throw(const Vector3f& aPosition, const Vector3f& aVelocity, BallThrowParams& aThrowParams);
		bool CanThrow();
		bool CanRecall();
		BoomerangComponent* GetHeldBoomerang();
		BoomerangComponent* GetThrownBoomerang();

		BoomerangComponent* GetUnclonedHeldBoomerang();
	};

	class Player : public KE::Component, public ES::IObserver
	{
		KE_EDITOR_FRIEND

		friend class PlayerIdleState;
		friend class PlayerMoveState;
		friend class PlayerDashState;
		friend class PlayerWindupState;
		friend class PlayerThrowState;
		friend class PlayerRecallingState;
		friend class PlayerAttackState;
		friend class PlayerDeathState;
		friend class PlayerFallingState;

	public:
		Player(KE::GameObject& aGO);
		~Player();

		void Awake() override;
		void Update() override;
		void SetData(void* aDataObject = nullptr) override;

		void ActionInput(KE::PlayerEvent& aEvent, KE::Interaction& anInteraction);
		void CalculateScreenSpaceDirection(const Vector3f& inputMovement);
		template<typename T> void SetPlayerState()
		{
			if (myCurrentState == myStates.states[typeid(T)]) {return;}

			if( myCurrentState) myCurrentState->OnExit();
			myCurrentState = myStates.states[typeid(T)];
			myCurrentState->OnEnter();
		}
		template<typename T> bool IsCurrentState()
		{
			return myCurrentState == myStates.states[typeid(T)];
		}
		void SetCollisionWithLayer(const KE::Collision::Layers& aLayer, bool aEnable);
		void OnCollisionEnter(const KE::CollisionData& aCollisionData);
		void TakeDamage(const unsigned int aMurdererIndex, bool aIgnorePowerups = false);

		void AssignController(const int aControllerID);
		void AssignTeam(const unsigned int aTeam);

		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		void OnDestroy() override;

		void SetCharacterIndex(int aCharacterIndex);
		void SetPlayerIndex(const unsigned int aIndex);
		void SetPosition(const Vector3f& aPosition);
		void SetDeathzoneTime(const float aTime);

		inline float GetDeathzoneTime() const { return myTimeInDeathzone; }
		inline unsigned int GetIndex() const { return myPlayerIndex; }
		inline unsigned int GetTeam() const  { return myTeam; }
		inline int GetCharacterIndex() const { return myCharacterIndex; }
		inline int GetControllerID() const { return myControllerID; }
		Vector3f& GetVelocityRef();
		PowerupList& GetPowerups() { return myPowerups; }
		BallManager* GetBallManager() {return myBallManager;}
		OwnedBoomerangs* GetBoomerangs() {return &myBoomerangs;}
		const Vector3f& GetMoveDirection() const { return myMoveDirection; }
		KE::Interaction* GetEventInteraction(KE::PlayerEvent* aPlayerEvent, unsigned int playerIndex, bool& aOutGamepad);
		PlayerAnimationController& GetAnimationController() { return myAnimationController; }
		inline KE::PhysicsCharacterController& GetPhysxController() { return *myPhysxController; }

		int GetScoreboardPosition(const GameData& gameData) const;

		bool IsPaused() const;

		const Vector3f& GetMoveDirection() { return myMoveDirection; }
		const Vector3f& GetLastMoveDirection() { return myLastMoveDirection; }
		
		void UpdateRenderBuffer();
	protected:
		void OnPhysXCollision(const KE::PhysXCollisionData& aPhysXCollisionData) override;

	private:
		void RegisterAsCameraTarget();
		void RemoveAsCameraTarget();

		KE::CBuffer myRenderBuffer = {};

		PlayerAnimationController myAnimationController;
		KE::PhysicsCharacterController* myPhysxController = nullptr;
		BallManager* myBallManager = nullptr;
		PlayerStateBase* myCurrentState = nullptr;

		BallThrowParams myBallThrowParams;
		OwnedBoomerangs myBoomerangs;
		PlayerStateList myStates;
		PowerupList myPowerups;

		Transform myCameraTransform = {};
		Vector3f myMoveDirection = {};
		Vector3f myLastMoveDirection = {};
		float myMoveSpeed = 0.0f;
		float myThrowCharge = 0.0f;
		float myThrowStrength = 50.0f;
		float myThrowBuildupSpeed = 5.0f;
		float myMaxForce = 5.0f;
		float myTimeInDeathzone = -1.0f;
		int myBoomerangGoID = INT_MAX;
		bool myMovementLocked = false;
		bool myIsAlive = true;


		float lastGroundedTimestamp = -1.0;

		int myCharacterIndex = 1; // The character model we are using
		unsigned int myPlayerIndex = 0; // The player index, like player 1, player 2, etc
		unsigned int myTeam = 0;
		int myControllerID = -1;

		std::unordered_map<KE::eInputType, eInputAction> myInputActionMap = {};
	};
}