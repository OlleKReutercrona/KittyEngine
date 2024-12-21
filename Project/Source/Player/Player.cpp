#include "stdafx.h"

#include "Player.h"

#include <format>

#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/GameObjectManager.h"
#include "Engine/Source/Input/InputEvents.h"
#include "Engine/Source/Collision/PhysicsCharacterController.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/SkeletalModelComponent.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/CameraComponent.h"
#include "Engine/Source/Graphics/Graphics.h"


#include "Project/Source/Player/SpawnPointComponent.h"
#include "Project/Source/Managers/BallManager.h"
#include "Project/Source/Boomerang/BoomerangComponent.h"
#include "Project/Source/GameEvents/GameEvents.h"

#include "Engine/Source/SceneManagement/Scene.h" // This is a bad include
#include "Project/Source/Managers/GameManager.h" // Required in order to know if we are paused.
#include "Engine/Source/Audio/GlobalAudio.h"

namespace P8
{
	Player::Player(KE::GameObject& aGO) : KE::Component(aGO)
	{
		myPowerups.ownerPlayer = this;
		OnInit();


		{
			D3D11_BUFFER_DESC desc = {};

			auto* gfx = KE_GLOBAL::blackboard.Get<KE::Graphics>();

			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.BindFlags              = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.Usage                  = D3D11_USAGE_DYNAMIC;
			bufferDesc.CPUAccessFlags         = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags              = 0u;
			bufferDesc.ByteWidth              = sizeof(PlayerRenderBuffer);
			bufferDesc.StructureByteStride    = 0u;
			myRenderBuffer.Init(gfx->GetDevice(), &bufferDesc);
		}
		UpdateRenderBuffer();

	}

	Player::~Player()
	{

		//OnDestroy();
	}

	int Player::GetScoreboardPosition(const GameData& gameData) const
	{
		int thisPlayerScore = gameData.scores[myPlayerIndex];
		int placeInScoreboard = 0;
		for (int i = 0; i < std::size(gameData.scores); i++)
		{
			if (i == static_cast<int>(myPlayerIndex)) { continue; }

			if (gameData.scores[i] == thisPlayerScore)
			{
				if (i < static_cast<int>(myPlayerIndex))
				{
					placeInScoreboard++;
				}
			}
			else if (gameData.scores[i] > thisPlayerScore)
			{
				placeInScoreboard++;
			}
		}
		return placeInScoreboard;
	}

	void Player::Awake()
	{
		
		auto& manager = myGameObject.GetManager();


		const int MAIN_CAMERA_ID = 0;
		myCameraTransform = manager.GetGameObject(MAIN_CAMERA_ID)->myTransform;

		// CharacterController's are not colliders, but they need a component defined for the collision system to trigger onContacts.
		if (myPhysxController)
		{
			KE::PhysxShapeUserData userData;
			userData.myID = myGameObject.myID;
			userData.gameObject = &myGameObject;
			myPhysxController->GetPhysicsObject().myKECollider.SetPhysxUserData(&userData);
			myPhysxController->GetPhysicsObject().myKECollider.myComponent = this;

			// This makes the player not spawn in air /DR
			myPhysxController->GetPhysicsObject().disableColliderMovementPrecedence = true; 
		}

		myStates.CreateState<PlayerIdleState>()->Init(this);
		myStates.CreateState<PlayerMoveState>()->Init(this);
		myStates.CreateState<PlayerDashState>()->Init(this);
		myStates.CreateState<PlayerWindupState>()->Init(this);
		myStates.CreateState<PlayerThrowState>()->Init(this);
		myStates.CreateState<PlayerRecallingState>()->Init(this);
		myStates.CreateState<PlayerAttackState>()->Init(this);
		myStates.CreateState<PlayerDeathState>()->Init(this);
		myStates.CreateState<PlayerTauntState>()->Init(this);
		myStates.CreateState<PlayerLobbyState>()->Init(this);
		myStates.CreateState<PlayerFallingState>()->Init(this);
		myStates.CreateState<PlayerWinScreenState>()->Init(this);

		// This should probably be changed...
		// This should probably be changed...
		// This should probably be changed...
		const auto& sceneName = myGameObject.GetManager().GetScene()->sceneName;
		if (sceneName.find("Select") != std::string::npos)
		{
			SetPlayerState<PlayerLobbyState>();
		}
		else if (sceneName.find("Victory") != std::string::npos)
		{
			SetPlayerState<PlayerWinScreenState>();
		}
		else
		{
			SetPlayerState<PlayerIdleState>();
		}

		myInputActionMap[KE::eInputType::Up] = eInputAction::Move;
		myInputActionMap[KE::eInputType::Down] = eInputAction::Move;
		myInputActionMap[KE::eInputType::Right] = eInputAction::Move;
		myInputActionMap[KE::eInputType::Left] = eInputAction::Move;
		myInputActionMap[KE::eInputType::XboxLeftStick] = eInputAction::Move;
		myInputActionMap[KE::eInputType::Space] = eInputAction::Dash;
		myInputActionMap[KE::eInputType::XboxA] = eInputAction::Dash;
		myInputActionMap[KE::eInputType::Action1] = eInputAction::Throw;
		myInputActionMap[KE::eInputType::XboxY] = eInputAction::Throw;
		myInputActionMap[KE::eInputType::Action2] = eInputAction::Attack;
		myInputActionMap[KE::eInputType::XboxB] = eInputAction::Attack;
		myInputActionMap[KE::eInputType::Action3] = eInputAction::Taunt;
		myInputActionMap[KE::eInputType::XboxX] = eInputAction::Taunt;

		myPowerups.manager = &manager.GetGameObject(KE::eGameSystemManager)->GetComponent<PowerupManager>();

		myBallManager = &manager.GetGameObject(KE::eGameSystemManager)->GetComponent<BallManager>();

		myBallManager->RegisterPlayer(this);
		BoomerangComponent* freeBall = myBallManager->GetUnusedBall();
		freeBall->SetOriginal(true);
		myBoomerangs.RegisterHeld(freeBall);

		SetCharacterIndex(myCharacterIndex);

		// Try setting spawn position
		auto spawnPoints = manager.GetGameObjectsWithComponent<SpawnPointComponent>();

		auto* gameManager = &manager.GetGameObject(KE::eGameSystemManager)->GetComponent<GameManager>();
		const GameData& gameData = gameManager->GetGameData();

		int scorePos = GetScoreboardPosition(gameData);

		std::vector<SpawnPointComponent*> spawnPointsVec;
		for (auto& spawnPoint : spawnPoints)
		{
			if (!spawnPoint->IsActive()) { continue; }

			spawnPointsVec.push_back(&spawnPoint->GetComponent<SpawnPointComponent>());
			int idx = spawnPointsVec.back()->GetIndex();

			if (idx == 10 + scorePos || idx == (int)myPlayerIndex)
			{
				const auto& spawnTransform = spawnPoint->myWorldSpaceTransform;

				SetPosition(spawnTransform.GetPosition() + Vector3f(0.0f, 0.0f, 0.0f));
				myGameObject.myTransform.SetScale(spawnTransform.GetScale());
				//spawnPoint->SetActive(false);
				spawnPointsVec.clear();
				break;
			}
		}
		if (spawnPointsVec.size() > 0)
		{
			const auto& spawnTransform = spawnPointsVec[0]->GetGameObject().myWorldSpaceTransform;
			Vector3f spawnPos = spawnTransform.GetPosition();
			spawnPos.y = 0.0f;
			SetPosition(spawnPos /*spawnTransform.GetPosition() + Vector3f(0.0f, 0.0f, 0.0f)*/);
			myGameObject.myTransform.SetScale(spawnTransform.GetScale());
			spawnPointsVec[0]->GetGameObject().SetActive(false);
		}

		if (KE::SkeletalModelComponent* plrSkeletalModel; myGameObject.TryGetComponent<KE::SkeletalModelComponent>(plrSkeletalModel))
		{

		}

		RegisterAsCameraTarget();
	}

	void Player::Update()
	{
		//if (GetAsyncKeyState('Y') & 1)
		//{
		//	GamepadRumbleEvent rumbleEvent(myControllerID, 0.25f, 0.25f, 0.1f, KE::RumbleType::Timed);
		//	ES::EventSystem::GetInstance().SendEvent<GamepadRumbleEvent>(rumbleEvent);
		//}

		//if (GetAsyncKeyState('U') & 1)
		//{
		//	GamepadRumbleEvent rumbleEvent(myControllerID, 0.5f, 0.5f, 0.2f, KE::RumbleType::Timed);
		//	ES::EventSystem::GetInstance().SendEvent<GamepadRumbleEvent>(rumbleEvent);
		//}

		//if (GetAsyncKeyState('I') & 1)
		//{
		//	GamepadRumbleEvent rumbleEvent(myControllerID, 1.0f, 1.0f, 0.6f, KE::RumbleType::Timed);
		//	ES::EventSystem::GetInstance().SendEvent<GamepadRumbleEvent>(rumbleEvent);
		//}

		//if (GetAsyncKeyState('O') & 1)
		//{
		//	GamepadRumbleEvent rumbleEvent(myControllerID, 1.0f, 1.0f, 1.0f, KE::RumbleType::Timed);
		//	ES::EventSystem::GetInstance().SendEvent<GamepadRumbleEvent>(rumbleEvent);
		//}



		StateBehaviourFlags flags = myCurrentState->GetBehaviourFlags();

		auto* gfx = KE_GLOBAL::blackboard.Get<KE::Graphics>();
		auto& spriteManager = gfx->GetSpriteManager();

		if (IsPaused()) { return; }

		myBoomerangs.Update();

		{
			Vector3f footPos = myPhysxController->GetFootPositioin();
			Vector3f regPos = myPhysxController->GetRegularPositioin();
			Vector3f sideOffset = Vector3f(0.5f, 0.0f, 0.0f);
			if (auto* dbg = KE_GLOBAL::blackboard.Get<KE::DebugRenderer>("debugRenderer"))
			{
				dbg->RenderLine(footPos + sideOffset, regPos + sideOffset, { 1.0f, 0.0f, 1.0f, 1.0f });
			}
		}


		myCurrentState->Update();

		if (!(flags & StateBehaviourFlag::eBlockRotation))
		{
			myPhysxController->Rotate(myMoveDirection.GetNormalized());
		}

		if (!(flags & StateBehaviourFlag::eBlockMovement))
		{
			if (auto moveDir = myMoveDirection.GetNormalized(); moveDir.LengthSqr() > 0.5f)
			{
				myPhysxController->Move(moveDir, myMoveSpeed);
			}
		}

		if (flags & StateBehaviourFlag::eStopVelocity)
		{
			myPhysxController->GetVelocityRef() *= 0.0f;
		}

		myPhysxController->Update();

		if (!myPhysxController->IsGrounded() && !myPhysxController->IsDashing())
		{
			if (!IsCurrentState<PlayerLobbyState>() && !IsCurrentState<PlayerDeathState>())
			{
				if (myPhysxController->GetDistanceToGround() < 0.0f)
				{
					myPhysxController->GetVelocityRef().y = 1.0f;
					SetPlayerState<PlayerFallingState>();
				}
			}
		}

		if (myPhysxController->IsSquished())
		{
			TakeDamage(myPlayerIndex, true);
		}

		PowerupInputData in{ this, myBoomerangs.GetThrownBoomerang() };
		myPowerups.Update(in);

		myMoveDirection = {};

		UpdateRenderBuffer();
	}

	void Player::SetData(void* aDataObject)
	{
		PlayerData* data = static_cast<PlayerData*>(aDataObject);

		myPhysxController = data->controller;
		KE::MovementData& movement = myPhysxController->GetMovementData();

		movement.maxSpeed = data->maxSpeed;
		movement.linearAcceleration = data->movementSpeed;
		movement.angularAcceleration = data->rotationSpeed;
		movement.maxRotation = data->maxRotation;
		myBoomerangGoID = data->boomerangGoID;
		//myPlayerIndex = data->playerIndex;
		myCharacterIndex = data->characterIndex;
	}

	void Player::RegisterAsCameraTarget()
	{
		P8::ActionCameraEvent acmsg(myGameObject, true);
		ES::EventSystem::GetInstance().SendEvent<P8::ActionCameraEvent>(acmsg);
	}
	void Player::RemoveAsCameraTarget()
	{
		P8::ActionCameraEvent acmsg(myGameObject, false);
		ES::EventSystem::GetInstance().SendEvent<P8::ActionCameraEvent>(acmsg);
	}

	void Player::SetPosition(const Vector3f& aPosition)
	{
		myPhysxController->SetPosition(aPosition);
		myGameObject.myTransform.SetPosition(aPosition);
	}
	void Player::SetDeathzoneTime(const float aTime)
	{
		myTimeInDeathzone = aTime;
	}
	Vector3f& Player::GetVelocityRef() { return myPhysxController->GetVelocityRef(); }

	void Player::SetCollisionWithLayer(const KE::Collision::Layers& aLayer, bool aEnable)
	{
		myPhysxController->SetCollisionWithLayer(aLayer, aEnable);
	}
	void Player::OnCollisionEnter(const KE::CollisionData& aCollisionData)
	{
		std::cout << "Player hit something\n";
	}

	bool Player::IsPaused() const
	{
		if (myCurrentState)
		{
			StateBehaviourFlags flags = myCurrentState->GetBehaviourFlags();
			if (flags & StateBehaviourFlag::eRunWhilePaused) { return false; }
		}

		return P8::GameManager::IsPaused();
	}

	void Player::UpdateRenderBuffer()
	{
		PlayerRenderBuffer buf;
		buf.timeInvulnerable = -1.0f;
		buf.timeOutsideBattleArea = 3.0f - myTimeInDeathzone;

		static auto* gfx = KE_GLOBAL::blackboard.Get<KE::Graphics>();
		myRenderBuffer.MapBuffer(&buf, sizeof(buf), gfx->GetContext().Get());
	}

	void Player::OnPhysXCollision(const KE::PhysXCollisionData& aPhysXCollisionData)
	{
		Player* playerComponent = nullptr;
		BoomerangComponent* boomerangComponent = nullptr;
		if (aPhysXCollisionData.objHit->TryGetComponent<Player>(playerComponent))
		{
			//KE_LOG("Player %i collided with Player %i", myIndex, playerComponent->myIndex);
		}
		else if (aPhysXCollisionData.objHit->TryGetComponent<BoomerangComponent>(boomerangComponent))
		{
			myIsAlive = false;
			//KE_LOG("Player %i collided with Boomerang %i", myIndex, aPhysXCollisionData.objHit->myID - (-9999));
		}
	}

	void Player::TakeDamage(const unsigned int aMurdererIndex, bool aIgnorePowerups)
	{
		if (IsCurrentState<P8::PlayerDeathState>()) { return; }

		GamepadRumbleEvent rumbleEvent(myControllerID, 0.5f, 0.5f, 0.2f, KE::RumbleType::Timed);
		ES::EventSystem::GetInstance().SendEvent<GamepadRumbleEvent>(rumbleEvent);

		PowerupInputData in{ this, myBoomerangs.GetThrownBoomerang() };
		if (myPowerups.OnPlayerAction(PlayerActionType::TakeDamage, ActionState::Begin, in))
		{
			return;
		}


		SetPlayerState<P8::PlayerDeathState>();
		SlowMotionEventData smed;
		smed.timeModifier = 0.35f;
		P8::SlowMotionEvent sme(smed);
		ES::EventSystem::GetInstance().SendEvent<P8::SlowMotionEvent>(sme);

		P8::PlayerKilledEvent murderevent(myPlayerIndex, aMurdererIndex);
		ES::EventSystem::GetInstance().SendEvent<P8::PlayerKilledEvent>(murderevent);

		RemoveAsCameraTarget();
	
		for (BoomerangComponent* ball : myBoomerangs.thrownBoomerangs)
		{
			P8::ActionCameraEvent msg(ball->GetGameObject(), false);
			ES::EventSystem::GetInstance().SendEvent<P8::ActionCameraEvent>(msg);
		}
	}
#pragma region Input

	void Player::ActionInput(KE::PlayerEvent& aEvent, KE::Interaction& anInteraction)
	{
		StateInput input = { myInputActionMap[aEvent.myInputType], anInteraction.myInteractionType };

		myCurrentState->ActionInput(input);
	}
	KE::Interaction* Player::GetEventInteraction(KE::PlayerEvent* aPlayerEvent, unsigned int playerIndex, bool& aOutGamepad)
	{
		for (auto& interaction : aPlayerEvent->interactions)
		{
			if ((interaction.myControllerID == myControllerID) && (myControllerID >= 0))
			{
				if (myControllerID < 4)
				{
					aOutGamepad = true;
				}
				return &interaction;
			}
		}

		//if (!aPlayerEvent || aPlayerEvent->myKeyBindings.size() <= playerIndex) { return nullptr; }

		//for (auto& interaction : aPlayerEvent->interactions)
		//{
		//	if (interaction.myKey == aPlayerEvent->myKeyBindings[playerIndex])
		//	{
		//		return &interaction;
		//	}
		//}

		return nullptr;
	}

	void Player::CalculateScreenSpaceDirection(const Vector3f& inputMovement)
	{
		auto& cam = myGameObject.GetManager().GetGameObject(0)->GetComponent<KE::CameraComponent>();
		const Vector3f& playerWorldPos = myGameObject.myTransform.GetPositionRef();
		Vector2f playerScreenPos;
		auto* camera = cam.GetCamera();
		Vector2f resolution = {
			camera->GetProjectionData().perspective.width,
			camera->GetProjectionData().perspective.height
		};

		//if (camera->WorldToScreenPoint(playerWorldPos, playerScreenPos,resolution))
		camera->WorldToScreenPoint(playerWorldPos, playerScreenPos, resolution);
		{
			Vector2f moveToScreenPos = playerScreenPos;
			moveToScreenPos.x += inputMovement.x * 200.0f;
			moveToScreenPos.y += inputMovement.z * 200.0f;

			//calculate the point of intersection with the plane,
			//in this case we can just move along the direction until its y is 0
			Rayf moveToRay = camera->GetRay(moveToScreenPos);
			Vector3f planeHit = moveToRay.GetOrigin() + moveToRay.GetDirection() * (-moveToRay.GetOrigin().y / moveToRay.GetDirection().y);

			Vector3f moveToWorldPos = planeHit;

			Vector3f playerToMove = moveToWorldPos - playerWorldPos;

			if (playerToMove.Length() >= 0.1f)
			{
				myMoveDirection += playerToMove.GetNormalized();
			}
		}
		myLastMoveDirection = myMoveDirection;
	}

	void Player::OnReceiveEvent(ES::Event& aEvent)
	{
		if (LobbyEvent* lobbyEvent = dynamic_cast<LobbyEvent*>(&aEvent))
		{
			if (lobbyEvent->myPlayerIndex != static_cast<int>(myPlayerIndex)) { return; }
			if (lobbyEvent->myLobbyEvent == eLobbyEvents::PlayerReady)
			{
				if (PlayerLobbyState* lobbyState = dynamic_cast<PlayerLobbyState*>(myCurrentState))
				{
					int index = static_cast<int>(sound::SFX::PlayerSelectCandy) + myCharacterIndex - 1;
					KE::GlobalAudio::PlaySFX(static_cast<sound::SFX>(index));
					lobbyState->OnReady();
				}
			}
			else if (lobbyEvent->myLobbyEvent == eLobbyEvents::PlayerNotReady)
			{
				if (PlayerLobbyState* lobbyState = dynamic_cast<PlayerLobbyState*>(myCurrentState))
				{
					lobbyState->OnUnready();
				}
			}
		}

		if (KE::PlayerEvent* playerEvent = dynamic_cast<KE::PlayerEvent*>(&aEvent))
		{
			if (IsPaused()) { return; }

			Vector3f inputMovement{ 0.0f, 0.0f, 0.0f };

			bool isGamepad = false;
			auto* interaction = GetEventInteraction(playerEvent, myPlayerIndex, isGamepad);

			if (interaction)
			{
				if (myInputActionMap[playerEvent->myInputType] == eInputAction::Move)
				{
					if (isGamepad)
					{
						inputMovement.x = interaction->myStick.x;
						inputMovement.z = -interaction->myStick.y;
						myMoveSpeed = inputMovement.Length();
						myMoveSpeed = myMoveSpeed > 1.0f ? 1.0f : myMoveSpeed;
					}
					else
					{
						myMoveSpeed = 1.0f;

						switch (playerEvent->myInputType)
						{
						case::KE::eInputType::Up: { inputMovement.z -= 1.0f; break; }
						case::KE::eInputType::Down: { inputMovement.z += 1.0f; break; }
						case::KE::eInputType::Right: { inputMovement.x += 1.0f; break; }
						case::KE::eInputType::Left: { inputMovement.x -= 1.0f; break; }
						}
					}
				}
				else
				{
					ActionInput(*playerEvent, *interaction);
				}
			}

			if (inputMovement.Length() != 0)
			{
				CalculateScreenSpaceDirection(inputMovement);
			}

			myMoveDirection.y = 0.0f;
		}

		if (CameraShakeEvent* event = dynamic_cast<CameraShakeEvent*>(&aEvent))
		{
			float distanceToExploson = (myGameObject.myTransform.GetPositionRef() - event->data.explosionPosition).Length();
			float maxDistance = 15.0f;

			float rumbleIntensity = std::clamp(1.0f - (distanceToExploson / maxDistance), 0.0f, 1.0f);
			float rumbleDuration = std::clamp(0.4f * rumbleIntensity, 0.1f, 0.4f);

			GamepadRumbleEvent rumbleEvent(myControllerID, rumbleIntensity, rumbleIntensity, rumbleDuration, KE::RumbleType::Timed);
			ES::EventSystem::GetInstance().SendEvent<GamepadRumbleEvent>(rumbleEvent);
		}
	}

	void Player::OnInit()
	{
		ES::EventSystem::GetInstance().Attach<KE::PlayerEvent>(this);
		ES::EventSystem::GetInstance().Attach<LobbyEvent>(this);
		ES::EventSystem::GetInstance().Attach<CameraShakeEvent>(this);
	}
	void Player::OnDestroy()
	{
		ES::EventSystem::GetInstance().Detach<KE::PlayerEvent>(this);
		ES::EventSystem::GetInstance().Detach<LobbyEvent>(this);
		ES::EventSystem::GetInstance().Detach<CameraShakeEvent>(this);
	}

	void Player::SetCharacterIndex(int aCharacterIndex)
	{
		myCharacterIndex = aCharacterIndex;

		KE::SkeletalModelComponent* skeletalModel = nullptr;
		if (myGameObject.TryGetComponent<KE::SkeletalModelComponent>(skeletalModel))
		{


			auto* gfx = KE_GLOBAL::blackboard.Get<KE::Graphics>();
			auto* skMD = skeletalModel->GetModelData();

			const std::string playerModelPath = std::format("Data/Assets/Players/Player{}/sk_player0{}.fbx", myCharacterIndex, myCharacterIndex);
			gfx->CreateSkeletalModelData(playerModelPath, skMD);

			std::vector<std::string> animationPaths = {
				std::format("Data/Assets/Players/Player{}/anim_player0{}_idle.fbx", myCharacterIndex, myCharacterIndex),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_run.fbx", myCharacterIndex, myCharacterIndex),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_dash.fbx", myCharacterIndex, myCharacterIndex),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_throw.fbx", myCharacterIndex, myCharacterIndex),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_death.fbx", myCharacterIndex, myCharacterIndex),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_taunt.fbx", myCharacterIndex, myCharacterIndex),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_select.fbx", myCharacterIndex, myCharacterIndex),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_victoryDance.fbx", myCharacterIndex, myCharacterIndex),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_podium.fbx", myCharacterIndex, myCharacterIndex)
			};


			auto& animPlr = skeletalModel->GetAnimationPlayer();
			animPlr.AssignAnimations(gfx->GetModelLoader(), animationPaths);
			myAnimationController.Init(animPlr, myCharacterIndex);

			auto* plrModelData = skeletalModel->GetModelData();
			for (auto& rr : plrModelData->myRenderResources)
			{
				rr.myPixelShader = KE_GLOBAL::blackboard.Get<KE::Graphics>()->GetShaderLoader().GetPixelShader(
					SHADER_LOAD_PATH "Player_PS.cso"
				);
				rr.myCBuffer = &myRenderBuffer;
				rr.myCBufferPSSlot = 7;
			}
		}

		if (myCurrentState)
		{
			myCurrentState->OnPlayerIndexChange();
		}

		for (auto* boomerang : myBoomerangs.heldBoomerangs)
		{
			boomerang->SyncCharacterIndex(this);
		}
	}
	void Player::SetPlayerIndex(const unsigned int aIndex)
	{
		myPlayerIndex = aIndex;
	}

	void Player::AssignController(const int aControllerID)
	{
		myControllerID = aControllerID;
	}
	void Player::AssignTeam(const unsigned int aTeam)
	{
		myTeam = aTeam;
		myBallThrowParams.team = myTeam;
		myPhysxController->GetUserData()->team = myTeam;
	}
#pragma endregion

#pragma region OwnedBoomerangs
	void OwnedBoomerangs::Throw(const Vector3f& aPosition, const Vector3f& aVelocity, BallThrowParams& aThrowParams)
	{
		if (heldBoomerangs.empty()) { return; }

		BoomerangComponent* aBoomerang = GetUnclonedHeldBoomerang();
		if (!aBoomerang) { return; } // No uncloned boomerangs to throw.

		aBoomerang->Throw(aPosition, aVelocity, aThrowParams);

		thrownBoomerangs.push_back(aBoomerang);
		std::erase(heldBoomerangs, aBoomerang);
	}

	void OwnedBoomerangs::RegisterHeld(BoomerangComponent* aBoomerang)
	{
		heldBoomerangs.push_back(aBoomerang);
	}

	void OwnedBoomerangs::RegisterThrown(BoomerangComponent* aBoomerang)
	{
		thrownBoomerangs.push_back(aBoomerang);
	}

	void OwnedBoomerangs::DeregisterHeld(BoomerangComponent* aBoomerang)
	{
		std::erase(heldBoomerangs, aBoomerang);
		aBoomerang->GetGameObject().SetActive(false);
	}

	void OwnedBoomerangs::DeregisterThrown(BoomerangComponent* aBoomerang)
	{
		std::erase(heldBoomerangs, aBoomerang);
		aBoomerang->GetGameObject().SetActive(false);
	}

	void OwnedBoomerangs::Update()
	{
		for (auto* boomerang : heldBoomerangs)
		{
			boomerang->UpdateThrowCooldown(KE_GLOBAL::deltaTime);
		}
	}

	void OwnedBoomerangs::Pickup(BoomerangComponent* aBoomerang)
	{
		std::erase(thrownBoomerangs, aBoomerang);
		auto& throwParams = aBoomerang->GetThrowParameters();

		if (!throwParams.isCopy)
		{
			heldBoomerangs.push_back(aBoomerang);
		}

		throwParams.oldState = BoomerangState::PickedUp;
		throwParams.state = BoomerangState::PickedUp;
		aBoomerang->OnPickup();
		aBoomerang->GetGameObject().SetActive(false);

	}

	bool OwnedBoomerangs::CanThrow()
	{
		if (heldBoomerangs.empty()) { return false; }

		bool hasHeldUncloned = GetUnclonedHeldBoomerang() ? true : false;


		return hasHeldUncloned;
	}

	bool OwnedBoomerangs::CanRecall()
	{
		return !thrownBoomerangs.empty();
	}

	BoomerangComponent* OwnedBoomerangs::GetHeldBoomerang()
	{
		if (heldBoomerangs.empty()) { return nullptr; }
		return heldBoomerangs.back();
	}

	BoomerangComponent* OwnedBoomerangs::GetThrownBoomerang()
	{
		if (thrownBoomerangs.empty()) { return nullptr; }
		return thrownBoomerangs.back();
	}

	BoomerangComponent* OwnedBoomerangs::GetUnclonedHeldBoomerang()
	{
		for (auto* boomerang : heldBoomerangs)
		{
			if (boomerang->GetThrowCooldown() > 0.0f) { continue; }

			bool hasThrownClone = false;
			if (boomerang->GetThrowParameters().isCopy) { continue; }
			for (auto* thrownBoomerang : thrownBoomerangs)
			{
				if (boomerang == thrownBoomerang) { continue; }

				if (thrownBoomerang->GetThrowParameters().splitFrom == boomerang)
				{
					hasThrownClone = true;
					break;
				}

			}
			if (!hasThrownClone)
			{
				return boomerang;
			}
		}

		return nullptr;
	}
#pragma endregion

}