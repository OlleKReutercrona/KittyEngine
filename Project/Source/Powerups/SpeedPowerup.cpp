#include "stdafx.h"
#include "SpeedPowerup.h"

#include "Engine/Source/Collision/PhysicsCharacterController.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/VFXComponent.h"
#include "Engine/Source/Graphics/FX/VFX.h"
#include "Player/Player.h"

namespace P8
{
	void SpeedPowerup::Update(const PowerupInputData& aInputData)
	{

	}

	void SpeedPowerup::OnPickup()
	{
		auto& movementData = ownerPlayer->GetPhysxController().GetMovementData();
		movementData.maxSpeed = movementData.maxSpeed * 2.0f;
		SetVFXActive(true, 0, true);
	}

	void SpeedPowerup::OnDrop()
	{
		auto& movementData = ownerPlayer->GetPhysxController().GetMovementData();
		movementData.maxSpeed = movementData.maxSpeed / 2.0f;		
		SetVFXActive(false, 0, true);
	}

	void SpeedPowerup::OnEnable()
	{

	}

	void SpeedPowerup::OnDisable()
	{
		
	}

	void SpeedPowerup::SetVFXActive(bool aActive, int idx, bool loop)
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

	bool SpeedPowerup::OnPlayerAction(PlayerActionType aAction, ActionState aState, PowerupInputData& input)
	{
		return false;
	}

}
