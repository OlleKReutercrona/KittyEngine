#include "stdafx.h"
#include "PlayerStateBase.h"

#include "Engine/Source/Collision/PhysicsCharacterController.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/VFXComponent.h"
#include "Engine/Source/Graphics/GraphicsConstants.h"
#include "Engine/Source/Graphics/FX/VFX.h"
#include "Engine/Source/Input/InputEvents.h"
#include "Player/Player.h"
#include "Engine/Source/Audio/GlobalAudio.h"
#include "Engine/Source/ComponentSystem/GameObjectManager.h"
#include "Project/Source/Boomerang/BoomerangComponent.h"

//IDLE STATE

void P8::PlayerIdleState::Update()
{
	myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Idle);

	if (myPlayer->myMoveDirection.LengthSqr() > 0.1f)
	{
		myPlayer->SetPlayerState<PlayerMoveState>();	
	}
}

void P8::PlayerIdleState::OnEnter()
{
	
	if (myPlayer->myMoveDirection.LengthSqr() > 0.1f)
	{
		myPlayer->SetPlayerState<PlayerMoveState>();	
		return;
	}
}

void P8::PlayerIdleState::OnExit()
{

}

void P8::PlayerIdleState::ActionInput(const StateInput& aInput)
{
	switch (aInput.actionType)
	{
		case P8::eInputAction::Throw:
		{
			if (aInput.interactionType == KE::eInteractionType::Pressed)
			{
				if (myPlayer->myBoomerangs.CanThrow())
				{
					PowerupInputData powerupInput = { myPlayer, myPlayer->myBoomerangs.GetThrownBoomerang() };
					if (myPlayer->myPowerups.OnPlayerAction(PlayerActionType::Windup, ActionState::Begin, powerupInput))
					{
						break;
					}
					myPlayer->SetPlayerState<PlayerWindupState>();
				}
				else
				{
					myPlayer->SetPlayerState<PlayerRecallingState>();
				}
			}
			break;
		}

		case P8::eInputAction::Dash:
		{
			if (aInput.interactionType == KE::eInteractionType::Pressed)
			{
				myPlayer->SetPlayerState<PlayerDashState>();
			}
			break;
		}
		case P8::eInputAction::Attack:
		{
			if (aInput.interactionType == KE::eInteractionType::Pressed)
			{
				myPlayer->SetPlayerState<PlayerAttackState>();
			}
			break;
		}
		case P8::eInputAction::Taunt:
		{
			if (aInput.interactionType == KE::eInteractionType::Pressed)
			{
				myPlayer->SetPlayerState<PlayerTauntState>();
			}
			break;		
		}
	}
}

//MOVE STATE

void P8::PlayerMoveState::Update()
{
	if (myPlayer->GetPhysxController().GetVelocity().LengthSqr() < 0.1f)
	{
		myPlayer->SetPlayerState<PlayerIdleState>();	
		return;
	}

	myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Run);

}

void P8::PlayerMoveState::OnEnter()
{
	myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Run);
}

void P8::PlayerMoveState::OnExit()
{
}

void P8::PlayerMoveState::ActionInput(const StateInput& aInput)
{
	switch (aInput.actionType)
	{
		case P8::eInputAction::Throw:
		{
			if (aInput.interactionType == KE::eInteractionType::Pressed)
			{
				if (myPlayer->myBoomerangs.CanThrow())
				{
					PowerupInputData powerupInput = { myPlayer, myPlayer->myBoomerangs.GetThrownBoomerang() };
					if (myPlayer->myPowerups.OnPlayerAction(PlayerActionType::Windup, ActionState::Begin, powerupInput))
					{
						break;
					}
					myPlayer->SetPlayerState<PlayerWindupState>();
				}
				else
				{
					myPlayer->SetPlayerState<PlayerRecallingState>();
				}
			}
			break;
		}
		case P8::eInputAction::Dash:
		{
			if (aInput.interactionType == KE::eInteractionType::Pressed)
			{
				myPlayer->SetPlayerState<PlayerDashState>();
			}
			break;
		}
		case P8::eInputAction::Attack:
		{
			if (aInput.interactionType == KE::eInteractionType::Pressed)
			{
				myPlayer->SetPlayerState<PlayerAttackState>();
			}
			break;
		}
		case P8::eInputAction::Taunt:
		{
			if (aInput.interactionType == KE::eInteractionType::Pressed)
			{
				myPlayer->SetPlayerState<PlayerTauntState>();
			}
			break;		
		}
	}
}

//DASH STATE

void P8::PlayerDashState::Update()
{
	if (!myPlayer->GetPhysxController().IsDashing())
	{
		myPlayer->SetPlayerState<PlayerIdleState>();
		return;
	}

	myPlayer->GetPhysxController().DashUpdate();

}

void P8::PlayerDashState::OnEnter()
{
	//myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Dash); this animation is broken ATM
	


	PowerupInputData powerupInput = { myPlayer, myPlayer->myBoomerangs.GetThrownBoomerang() };

	if (myPlayer->myPowerups.OnPlayerAction(PlayerActionType::Dash, ActionState::Begin, powerupInput))
	{
		myPlayer->SetPlayerState<PlayerIdleState>();
		return;
	}

	myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Throw);
	KE::GlobalAudio::PlaySFX(sound::SFX::PlayerDash);

	Vector3f moveDirection = myPlayer->GetMoveDirection().GetNormalized();
	if (moveDirection.LengthSqr() < 0.1f)
	{
		moveDirection = myPlayer->GetGameObject().myTransform.GetForward();
	}
	myPlayer->GetPhysxController().Dash(moveDirection, 2.0f);
}

void P8::PlayerDashState::OnExit()
{
	PowerupInputData powerupInput = { myPlayer, myPlayer->myBoomerangs.GetThrownBoomerang() };
	if (myPlayer->myPowerups.OnPlayerAction(PlayerActionType::Dash, ActionState::End, powerupInput))
	{
		myPlayer->SetPlayerState<PlayerIdleState>();
		return;
	}
}

void P8::PlayerDashState::ActionInput(const StateInput& aInput)
{
	switch(aInput.actionType)
	{
		case P8::eInputAction::Taunt:
		{
			if (aInput.interactionType == KE::eInteractionType::Pressed)
			{
				myPlayer->SetPlayerState<PlayerTauntState>();
			}
			break;		
		}
		case P8::eInputAction::Attack:
		{
			if (aInput.interactionType == KE::eInteractionType::Pressed)
			{
				myPlayer->SetPlayerState<PlayerAttackState>();
			}
			break;
		}
	}
}

//WINDUP STATE

void P8::PlayerWindupState::Update()
{
	KE::GameObject& gObj = myPlayer->GetGameObject();
	vfxTransform = gObj.myTransform;
	vfxTransform.GetPositionRef().y = 0.5f;
	vfxTransform.TranslateLocal({0.0f, 0.0f, 1.0f});
	vfxTransform.SetScale({1.0f, 1.0f, 0.5f + (1.0f - (1.0f / myPlayer->myThrowCharge))});
	myPlayer->myThrowCharge += myPlayer->myThrowBuildupSpeed * KE_GLOBAL::deltaTime;
}

void P8::PlayerWindupState::OnEnter()
{
	myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Windup);

	KE::VFXComponent* plrVFXComponent = nullptr;
	if (KE::GameObject& gObj = myPlayer->GetGameObject(); gObj.TryGetComponent<KE::VFXComponent>(plrVFXComponent))
	{
		vfxTransform = gObj.myTransform;
		vfxTransform.GetPositionRef().y = 0.5f;
		vfxTransform.TranslateLocal({0.0f, 0.0f, 1.0f});

		myPlayer->myThrowCharge = 1.0f;

		auto vfxInput = KE::VFXRenderInput(vfxTransform, true, false);
		vfxInput.myLayer = KE::eRenderLayers::Front;
		plrVFXComponent->TriggerVFXCustom(0, vfxInput);
	}

}

void P8::PlayerWindupState::OnExit()
{
	KE::VFXComponent* plrVFXComponent = nullptr;
	if (KE::GameObject& gObj = myPlayer->GetGameObject(); gObj.TryGetComponent<KE::VFXComponent>(plrVFXComponent))
	{
		auto vfxInput = KE::VFXRenderInput(vfxTransform, true, false);
		vfxInput.myLayer = KE::eRenderLayers::Front;
		plrVFXComponent->StopVFXCustom(0, vfxInput);
	}
}

void P8::PlayerWindupState::ActionInput(const StateInput& aInput)
{
	switch (aInput.actionType)
	{
		case P8::eInputAction::Throw:
		{
			if (aInput.interactionType == KE::eInteractionType::Released)
			{
				myPlayer->SetPlayerState<PlayerThrowState>();
			}
			break;
		}
		case P8::eInputAction::Attack:
		{
			if (aInput.interactionType == KE::eInteractionType::Pressed)
			{
				myPlayer->SetPlayerState<PlayerAttackState>();
			}
			break;
		}
	}
}

//THROW STATE

void P8::PlayerThrowState::Update()
{
	lockTimer += KE_GLOBAL::deltaTime;
	if (lockTimer >= lockTime)
	{
		myPlayer->SetPlayerState<PlayerIdleState>();
	}
}

void P8::PlayerThrowState::OnEnter()
{
	KE::GlobalAudio::PlaySFX(sound::SFX::PlayerThrow);

	lockTimer = 0;
	myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Throw);

	auto& gObj = myPlayer->GetGameObject();
	auto& gObjTransform = gObj.myTransform;

	Vector3f direction = gObjTransform.GetForward();
	Vector3f velocity = direction * (1.0f - (1.0f / myPlayer->myThrowCharge)) * myPlayer->myThrowStrength;
	
	myPlayer->myBallThrowParams.throwerTransform = &gObjTransform;
	myPlayer->myBallThrowParams.throwerPlayer = myPlayer;
	myPlayer->myBoomerangs.Throw(gObjTransform.GetPosition(), velocity, myPlayer->myBallThrowParams);
	myPlayer->myMovementLocked = false;
	myPlayer->myThrowCharge = 1.0f;
}

void P8::PlayerThrowState::OnExit()
{
}

void P8::PlayerThrowState::ActionInput(const StateInput& aInput)
{
}

//RECALLING STATE

void P8::PlayerRecallingState::Update()
{
	PowerupInputData in = { myPlayer, myPlayer->myBoomerangs.GetThrownBoomerang() };
	myPlayer->GetPowerups().OnPlayerAction(PlayerActionType::Recall, ActionState::Ongoing, in);
	myPlayer->GetPowerups().OnBoomerangAction(BoomerangAction::Recall, ActionState::Ongoing, in);

	if (!myPlayer->myBoomerangs.CanRecall())
	{
		myPlayer->SetPlayerState<PlayerIdleState>();
	}
}

void P8::PlayerRecallingState::OnEnter()
{
	myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Idle);

	KE::VFXComponent* plrVFXComponent = nullptr;
	if (KE::GameObject& gObj = myPlayer->GetGameObject(); gObj.TryGetComponent<KE::VFXComponent>(plrVFXComponent))
	{
		{
			auto vfxInput = KE::VFXRenderInput(gObj.myTransform, true, false);
			vfxInput.myLayer = KE::eRenderLayers::Front;
			plrVFXComponent->TriggerVFXCustom(1, vfxInput);
		}
	}

	for (auto* boomerang : myPlayer->myBoomerangs.thrownBoomerangs)
	{
		boomerang->BeginRecall();
	}

	PowerupInputData in = { myPlayer, myPlayer->myBoomerangs.GetThrownBoomerang() };
	myPlayer->GetPowerups().OnPlayerAction(PlayerActionType::Recall, ActionState::Begin, in);
	myPlayer->GetPowerups().OnBoomerangAction(BoomerangAction::Recall, ActionState::Begin, in);

}

void P8::PlayerRecallingState::OnExit()
{
	KE::VFXComponent* plrVFXComponent = nullptr;
	if (KE::GameObject& gObj = myPlayer->GetGameObject(); gObj.TryGetComponent<KE::VFXComponent>(plrVFXComponent))
	{
		{
			auto vfxInput = KE::VFXRenderInput(gObj.myTransform, true, false);
			vfxInput.myLayer = KE::eRenderLayers::Front;
			plrVFXComponent->StopVFXCustom(1, vfxInput);
		}
	}

	for (auto* boomerang : myPlayer->myBoomerangs.thrownBoomerangs)
	{
		boomerang->EndRecall();
	}

	PowerupInputData in = { myPlayer, myPlayer->myBoomerangs.GetThrownBoomerang() };
	myPlayer->GetPowerups().OnPlayerAction(PlayerActionType::Recall, ActionState::End, in);
	myPlayer->GetPowerups().OnBoomerangAction(BoomerangAction::Recall, ActionState::End, in);

}

void P8::PlayerRecallingState::ActionInput(const StateInput& aInput)
{
	switch (aInput.actionType)
	{
		case P8::eInputAction::Throw:
		{
			if (aInput.interactionType == KE::eInteractionType::Released)
			{
				myPlayer->SetPlayerState<PlayerIdleState>();
			}
			break;
		}
		case P8::eInputAction::Attack:
		{
			if (aInput.interactionType == KE::eInteractionType::Pressed)
			{
				myPlayer->SetPlayerState<PlayerAttackState>();
			}
			break;
		}
		case P8::eInputAction::Taunt:
		{
			if (aInput.interactionType == KE::eInteractionType::Pressed)
			{
				myPlayer->SetPlayerState<PlayerTauntState>();
			}
			break;		
		}
	}
}


//ATTACK STATE

void P8::PlayerAttackState::Update()
{
	attackTimer += KE_GLOBAL::deltaTime;
	if (attackTimer >= attackTime /*|| myPlayer->GetAnimationController().AnimationFinished()*/)
	{
		myPlayer->SetPlayerState<PlayerIdleState>();
		//std::cout << "IDLE\n";
	}

	myPlayer->GetPhysxController().AttackUpdate();
}

void P8::PlayerAttackState::OnEnter()
{
	//std::cout << "ATTACK\n";
	attackTimer = 0.0f;
	attackTime = 0.25f;

	myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Throw, true);

	KE::GlobalAudio::PlaySFX(sound::SFX::PlayerMeele);
	myPlayer->GetPhysxController().ResetAttack();

	KE::VFXComponent* plrVFXComponent = nullptr;
	if (KE::GameObject& gObj = myPlayer->GetGameObject(); gObj.TryGetComponent<KE::VFXComponent>(plrVFXComponent))
	{
		{
			plrVFXComponent->TriggerVFX(3, false);
		}
	}
}

void P8::PlayerAttackState::OnExit()
{
	KE::VFXComponent* plrVFXComponent = nullptr;
	if (KE::GameObject& gObj = myPlayer->GetGameObject(); gObj.TryGetComponent<KE::VFXComponent>(plrVFXComponent))
	{
		{
			plrVFXComponent->StopVFX(3);
		}
	}
}

void P8::PlayerAttackState::ActionInput(const StateInput& aInput)
{

}

//DEATH STATE

void P8::PlayerDeathState::Update()
{
	myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Death);

	deathTimer += KE_GLOBAL::deltaTime;

	if (deathTimer >= deathTime)
	{
		//myPlayer->SetPlayerState<PlayerIdleState>();
	}
}

void P8::PlayerDeathState::OnEnter()
{
	// First death sound + character index - 1 should equal the character's voice line.
	int index = static_cast<int>(sound::SFX::PlayerDeathCandy) + (myPlayer->GetCharacterIndex() - 1);

	KE::GlobalAudio::PlaySFX(static_cast<sound::SFX>(index), 0.5f);
	deathTimer = 0;
	KE::VFXComponent* plrVFXComponent = nullptr;
	if (KE::GameObject& gObj = myPlayer->GetGameObject(); gObj.TryGetComponent<KE::VFXComponent>(plrVFXComponent))
	{
		{
			auto vfxInput = KE::VFXRenderInput(gObj.myTransform, false, false);
			vfxInput.myLayer = KE::eRenderLayers::Front;
			plrVFXComponent->TriggerVFXCustom(2, vfxInput);
		}
	}
}

void P8::PlayerDeathState::OnExit()
{
	KE::VFXComponent* plrVFXComponent = nullptr;
	if (KE::GameObject& gObj = myPlayer->GetGameObject(); gObj.TryGetComponent<KE::VFXComponent>(plrVFXComponent))
	{
		{
			auto vfxInput = KE::VFXRenderInput(gObj.myTransform, false, false);
			vfxInput.myLayer = KE::eRenderLayers::Front;
			plrVFXComponent->StopVFXCustom(2, vfxInput);
		}
	}
}

void P8::PlayerDeathState::ActionInput(const StateInput& aInput)
{
}

//FALLING STATE

void P8::PlayerFallingState::Update()
{
	fallTimer += KE_GLOBAL::deltaTime;

	auto& controller = myPlayer->GetPhysxController();

	auto footPosition = controller.GetFootPositioin();
	const float distanceToGround = controller.GetDistanceToGround();
	if (distanceToGround >= 0.0f) // negative distance means there is no ground under the player.
	{
		footPosition.y -= distanceToGround;
		controller.SetPosition(footPosition);
		myPlayer->SetPlayerState<PlayerIdleState>();
		return;
	}

	float fallProgress = fallTimer / fallTime;


	footPosition.y = sin(fallProgress * KE::PI) * 1.0f;
	controller.SetPosition(footPosition);

	if (controller.IsGrounded())
	{
		myPlayer->SetPlayerState<PlayerIdleState>();
		return;
	}


	if (fallTimer >= fallTime)
	{
		//myPlayer->SetPlayerState<PlayerDeathState>();
		myPlayer->TakeDamage(myPlayer->GetIndex(), true);
		return;
	}


}

void P8::PlayerFallingState::OnEnter()
{
	fallTimer = 0.0f;
	KE::GlobalAudio::PlaySFX(sound::SFX::PlayerFalling);
	myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Idle);



	KE::VFXComponent* plrVFXComponent = nullptr;
	if (KE::GameObject& gObj = myPlayer->GetGameObject(); gObj.TryGetComponent<KE::VFXComponent>(plrVFXComponent))
	{
		{
			plrVFXComponent->TriggerVFX(5, false);
		}
	}
}

void P8::PlayerFallingState::OnExit()
{
	KE::VFXComponent* plrVFXComponent = nullptr;
	if (KE::GameObject& gObj = myPlayer->GetGameObject(); gObj.TryGetComponent<KE::VFXComponent>(plrVFXComponent))
	{
		{
			plrVFXComponent->StopVFX(5);
		}
	}
}

void P8::PlayerFallingState::ActionInput(const StateInput& aInput)
{
	switch (aInput.actionType)
	{
		case P8::eInputAction::Dash:
		{
			
			if (!CanDash()) { break; }
			if (aInput.interactionType == KE::eInteractionType::Released)
			{
				myPlayer->myMoveDirection = myPlayer->myLastMoveDirection;

				auto footPosition = myPlayer->GetPhysxController().GetFootPositioin();
				footPosition.y = 0.1f;
				myPlayer->GetPhysxController().SetPosition(footPosition);

				myPlayer->SetPlayerState<PlayerDashState>();
			}
			break;
		}
	}
}

bool P8::PlayerFallingState::CanDash()
{
	float timeSinceLastGrounded = KE_GLOBAL::totalTime - myPlayer->lastGroundedTimestamp;

	return timeSinceLastGrounded < 0.5f;
}

//TAUNT STATE LOL
void P8::PlayerTauntState::Update()
{
	myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Taunt);
	tauntTimer += KE_GLOBAL::deltaTime;
	if (tauntTimer >= tauntTime)
	{
		myPlayer->SetPlayerState<PlayerIdleState>();
	}
}

void P8::PlayerTauntState::OnEnter()
{
	tauntTimer = 0;

	int index = static_cast<int>(sound::SFX::PlayerTauntCandy) + (myPlayer->GetCharacterIndex() - 1);

	KE::GlobalAudio::PlaySFX(static_cast<sound::SFX>(index));
}

void P8::PlayerTauntState::OnExit()
{

}

void P8::PlayerTauntState::ActionInput(const StateInput& aInput)
{
}

//HOBBY LOBBY STATE
void P8::PlayerLobbyState::Update()
{

	//myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Select);

	if (myPlayer->GetAnimationController().AnimationFinished())
	{
		myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Idle);
	}
}

void P8::PlayerLobbyState::OnEnter()
{
	SetVFXActive(true, myPlayer->GetCharacterIndex());
}

void P8::PlayerLobbyState::OnExit()
{
	SetVFXActive(false, myPlayer->GetCharacterIndex());
}

void P8::PlayerLobbyState::OnReady()
{
	myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Select);
}

void P8::PlayerLobbyState::OnUnready()
{

}

void P8::PlayerLobbyState::ActionInput(const StateInput& aInput)
{
	if (aInput.actionType == P8::eInputAction::Taunt)
	{
		if (aInput.interactionType == KE::eInteractionType::Pressed)
		{
			int index = static_cast<int>(sound::SFX::PlayerTauntCandy) + myPlayer->GetCharacterIndex() - 1;

			if (KE::GlobalAudio::IsSFXPlaying(static_cast<sound::SFX>(index))) { return; }

			KE::GlobalAudio::PlaySFX(static_cast<sound::SFX>(index));
			myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Taunt);
		}
	}
	if (aInput.actionType == P8::eInputAction::Throw)
	{
		if (aInput.interactionType == KE::eInteractionType::Pressed)
		{
			myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Dance);
		}
	}
}

void P8::PlayerLobbyState::SetVFXActive(bool aActive, int idx)
{
	KE::VFXComponent* plrVFXComponent = nullptr;
	if (KE::GameObject& gObj = myPlayer->GetGameObject(); gObj.TryGetComponent<KE::VFXComponent>(plrVFXComponent))
	{
		KE::VFXRenderInput in(gObj.myWorldSpaceTransform, true, false);
		in.bloom = false;

		{
			if (aActive)
			{
				plrVFXComponent->TriggerVFXCustom(5 + idx, in);
			}
			else
			{
				plrVFXComponent->StopVFXCustom(5 + idx, in);
			}
		}
	}
}

void P8::PlayerLobbyState::OnPlayerIndexChange()
{
	for (int i = 1; i < 6; i++)
	{
		SetVFXActive(i == myPlayer->GetCharacterIndex(), i);
	}
}


//

void P8::PlayerWinScreenState::Init(Player* aPlayer)
{
	PlayerStateBase::Init(aPlayer);

	myWinscreenVFX.manager = KE_GLOBAL::blackboard.Get<KE::VFXManager>();
	for (int i = 0; i < 4; i++)
	{
		myWinscreenVFX.AddVFX(std::format("Character0{}WinscreenFX", i+1));
	}
	for (int i = 0; i < 4; i++)
	{
		myWinscreenVFX.AddVFX(std::format("Character0{}WinscreenName", i+1));
	}
	for (int i = 0; i < 4; i++)
	{
		myWinscreenVFX.AddVFX(std::format("Player{}WinTitle", i));
	}

}

void P8::PlayerWinScreenState::Update()
{
	if (winTimer > -1.0f) { winTimer += KE_GLOBAL::trueDeltaTime; }
	if (winTimer > winTime)
	{
		if (scoreboardPosition == 0)
		{
			SetVFXActive(true, myPlayer->GetCharacterIndex()-1);
			SetVFXActive(true, myPlayer->GetCharacterIndex()-1 + 4);
			SetVFXActive(true, static_cast<int>(myPlayer->GetIndex()) + 8);
		}
		winTimer = -10.0f;
	}

	bool animFinished = myPlayer->GetAnimationController().AnimationFinished();
	if (taunting)
	{
		if (animFinished)
		{
			taunting = false;
		}
	}
	else
	{
		myPlayer->GetAnimationController().PlayAnimation(
			scoreboardPosition == 0 ? PlayerAnimation::Dance : PlayerAnimation::Podium
		);
	}
	
	float rot = 8.0f;

	if (scoreboardPosition == 0)
	{
		myPlayer->GetPhysxController().SetOrientation(-rot * KE::DegToRadImmediate);
	}
	else
	{
		myPlayer->GetPhysxController().SetOrientation(rot * KE::DegToRadImmediate);
	}
}

void P8::PlayerWinScreenState::OnEnter()
{
	for (auto* powerup : myPlayer->GetPowerups().powerups)
	{
		powerup->OnDrop();
		delete powerup;
	}
	myPlayer->GetPowerups().powerups.clear();

	auto& gameManager = myPlayer->GetGameObject().GetManager().GetGameObject(KE::ReservedGameObjects::eGameSystemManager)->GetComponent<GameManager>();
	const auto& gameData = gameManager.GetGameData();

	scoreboardPosition = myPlayer->GetScoreboardPosition(gameData);

	myPlayer->GetAnimationController().PlayAnimation(
		scoreboardPosition == 0 ? PlayerAnimation::Dance : PlayerAnimation::Podium
	);


	if (myPlayer->GetCharacterIndex() == 5)
	{
		if (scoreboardPosition == 0)
		{
			// Win
			KE::GlobalAudio::PlaySFX(sound::SFX::GodWin);
		}
		else
		{
			// Lose
			KE::GlobalAudio::PlaySFX(sound::SFX::GodLose);
		}
	}
}

void P8::PlayerWinScreenState::OnExit()
{
	//SetVFXActive(false, myPlayer->GetCharacterIndex());
}


void P8::PlayerWinScreenState::ActionInput(const StateInput& aInput)
{
	if (aInput.actionType == P8::eInputAction::Taunt)
	{
		if (!taunting && scoreboardPosition == 0 && aInput.interactionType == KE::eInteractionType::Pressed)
		{
			myPlayer->GetAnimationController().PlayAnimation(PlayerAnimation::Taunt);
			taunting = true;
		}
	}
}

void P8::PlayerWinScreenState::SetVFXActive(bool aActive, int idx)
{
	myWinscreenVFXTransform.SetPosition(myPlayer->GetGameObject().myWorldSpaceTransform.GetPosition());
	KE::VFXRenderInput in(myWinscreenVFXTransform, true);

	if (aActive)
	{
		myWinscreenVFX.TriggerVFXSequence(idx, in);
	}
	else
	{
		myWinscreenVFX.StopVFXSequence(idx, in);
	}
}

void P8::PlayerWinScreenState::OnPlayerIndexChange()
{
	for (int i = 0; i < 4; i++)
	{
		SetVFXActive(scoreboardPosition == 0 && i == myPlayer->GetCharacterIndex()-1, i);
	}

	for (int i = 0; i < 4; i++)
	{
		SetVFXActive(scoreboardPosition == 0 && i == myPlayer->GetCharacterIndex()-1, i+4);
	}

	for (int i = 0; i < 4; i++)
	{
		SetVFXActive(scoreboardPosition == 0 && i == static_cast<int>(myPlayer->GetIndex()), i+8);
	}
}
