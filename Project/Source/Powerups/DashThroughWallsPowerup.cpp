#include "stdafx.h"
#include "DashThroughWallsPowerup.h"

#include "Engine/Source/Collision/Layers.h"
#include "Engine/Source/Collision/PhysicsCharacterController.h"
#include "Player/Player.h"

namespace P8
{
	void DashThroughWallsPowerup::Update(const PowerupInputData& aInputData)
	{
	}

	void DashThroughWallsPowerup::OnPickup()
	{
		
	}

	void DashThroughWallsPowerup::OnDrop()
	{
		
	}

	void DashThroughWallsPowerup::OnEnable()
	{
		
	}

	void DashThroughWallsPowerup::OnDisable()
	{
		
	}

	bool DashThroughWallsPowerup::OnPlayerAction(PlayerActionType aAction, ActionState aState, PowerupInputData& input)
	{
		if (aAction == PlayerActionType::Dash && aState == ActionState::Begin)
		{
			if (!input.player) { return false; }
			
			input.player->SetCollisionWithLayer(KE::Collision::Layers::Wall, false);
			input.player->GetPhysxController().SetDashThroughWalls(true);
		}
		else if (aAction == PlayerActionType::Dash && aState == ActionState::End)
		{
			if (!input.player) { return false; }

			input.player->SetCollisionWithLayer(KE::Collision::Layers::Wall, true);
			input.player->GetPhysxController().SetDashThroughWalls(false);

		}
		return false;
	}

	bool DashThroughWallsPowerup::OnBoomerangAction(BoomerangAction aAction, ActionState aState, PowerupInputData& input)
	{

		return false;
	}
}