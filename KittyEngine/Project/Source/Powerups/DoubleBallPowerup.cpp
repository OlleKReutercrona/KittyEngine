#include "stdafx.h"
#include "DoubleBallPowerup.h"

#include "Boomerang/BoomerangComponent.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/GameObjectManager.h"
#include "Managers/BallManager.h"
#include "Player/Player.h"

#include "Engine/Source/Graphics/FX/VFX.h"

namespace P8
{
	void DoubleBallPowerup::Update(const PowerupInputData& aInputData)
	{

	}

	void DoubleBallPowerup::OnPickup()
	{
		OwnedBoomerangs* plrBalls = ownerPlayer->GetBoomerangs();
		//BallManager* ballManager = ownerPlayer->GetBallManager();
		BallManager* ballManager = &ownerPlayer->GetGameObject().GetManager().GetGameObject(KE::ReservedGameObjects::eGameSystemManager)->GetComponent<BallManager>();



		BoomerangComponent* freeBall = ballManager->GetUnusedBall();
		freeBall->SetFromPowerup(true);
		freeBall->SyncCharacterIndex(ownerPlayer);
		plrBalls->RegisterHeld(freeBall);
	}

	void DoubleBallPowerup::OnDrop()
	{
		OwnedBoomerangs* plrBalls = ownerPlayer->GetBoomerangs();
		BallManager* ballManager = ownerPlayer->GetBallManager();

		//let's be nice and prioritize thrown balls

		bool erased = false;

		for (auto* ball : plrBalls->thrownBoomerangs)
		{
			if (ball->GetThrowParameters().isCopy) { continue; }
			if (!ball->IsFromPowerup()) { continue; }
			if (ball->IsPickedUp()) { continue; }

			//ball->SetImportant(false);
			plrBalls->DeregisterThrown(ball);
			erased = true;

			break;
		}
		

		if (erased) { return; }

		for (auto* ball : plrBalls->heldBoomerangs)
		{
			if (ball->GetThrowParameters().isCopy) { continue; }
			if (!ball->IsFromPowerup()) { continue; }

			//ball->SetImportant(false);
			plrBalls->DeregisterHeld(ball);
			break;
		}
	}

	void DoubleBallPowerup::OnEnable()
	{

	}

	void DoubleBallPowerup::OnDisable()
	{
		
	}

	void DoubleBallPowerup::SetVFXActive(bool aActive, int idx, bool loop)
	{

	}

	bool DoubleBallPowerup::OnPlayerAction(PlayerActionType aAction, ActionState aState, PowerupInputData& input)
	{

		return false;
	}

}
