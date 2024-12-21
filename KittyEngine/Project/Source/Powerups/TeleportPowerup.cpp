#include "stdafx.h"
#include "TeleportPowerup.h"

#include "Boomerang/BallThrowParams.h"
#include "Boomerang/BoomerangComponent.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Player/Player.h"
#include "Engine/Source/Audio/GlobalAudio.h"

namespace P8
{
	void TeleportPowerup::Update(const PowerupInputData& aInputData)
	{
	}

	void TeleportPowerup::OnPickup()
	{
		
	}

	void TeleportPowerup::OnDrop()
	{
		
	}

	void TeleportPowerup::OnEnable()
	{
		
	}

	void TeleportPowerup::OnDisable()
	{
		
	}

	bool TeleportPowerup::OnPlayerAction(PlayerActionType aAction, ActionState aState, PowerupInputData& input)
	{
		if (aAction == PlayerActionType::Dash && aState == ActionState::Begin)
		{
			if (!input.player || !input.boomerang) { return false; }
			if (input.boomerang->GetThrowParameters().state == BoomerangState::PickedUp) { return false; }	


			Vector3f pos = input.boomerang->GetGameObject().myWorldSpaceTransform.GetPosition();
			pos.y = 0.0f;
			input.player->SetPosition(pos);

			KE::GlobalAudio::PlaySFX(sound::SFX::PowerupTeleport);

			return true;
		}
		return false;
	}

	bool TeleportPowerup::OnBoomerangAction(BoomerangAction aAction, ActionState aState, PowerupInputData& input)
	{

		return false;
	}
}