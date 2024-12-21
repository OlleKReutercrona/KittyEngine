#include "stdafx.h"
#include "JuggernautPowerup.h"

#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/Graphics/FX/VFX.h"
#include "Player/Player.h"

namespace P8
{
	void JuggernautPowerup::Update(const PowerupInputData& aInputData)
	{

	}

	void JuggernautPowerup::OnPickup()
	{
		SetVFXActive(true, 0, true);
	}

	void JuggernautPowerup::OnDrop()
	{
		SetVFXActive(false, 0, true);

	}

	void JuggernautPowerup::OnEnable()
	{

	}

	void JuggernautPowerup::OnDisable()
	{
		
	}

	void JuggernautPowerup::SetVFXActive(bool aActive, int idx, bool loop)
	{
		KE::VFXRenderInput vfxInput(ownerPlayer->GetGameObject().myWorldSpaceTransform, true, false);

		if (aActive)
		{
			vfxInterface->TriggerVFXSequence(idx, vfxInput);
		}
		else
		{
			vfxInterface->StopVFXSequence(idx, vfxInput);
		}

	}

	bool JuggernautPowerup::OnPlayerAction(PlayerActionType aAction, ActionState aState, PowerupInputData& input)
	{
		if (aAction == PlayerActionType::Windup)
		{
			return true;
		}

		return false;
	}

}
