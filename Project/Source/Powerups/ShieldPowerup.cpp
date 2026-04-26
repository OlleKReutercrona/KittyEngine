#include "stdafx.h"
#include "ShieldPowerup.h"

#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/VFXComponent.h"
#include "Engine/Source/Graphics/FX/VFX.h"
#include "Player/Player.h"
#include "Engine/Source/Audio/GlobalAudio.h"

namespace P8
{
	void ShieldPowerup::Update(const PowerupInputData& aInputData)
	{
		if (invulnerabilityTimer > 0.0f)
		{
			invulnerabilityTimer -= KE_GLOBAL::deltaTime;
			if (invulnerabilityTimer <= 0.0f)
			{
				aInputData.player->GetPowerups().RemovePowerup(this);
			}
		}
	}

	void ShieldPowerup::OnPickup()
	{
		SetVFXActive(true, 0, true);
		KE::GlobalAudio::PlaySFX(sound::SFX::PowerupShieldActivate);
	}

	void ShieldPowerup::OnDrop()
	{
		SetVFXActive(false, 0, true);
		KE::GlobalAudio::PlaySFX(sound::SFX::PowerupShieldDeActivate);
	}

	void ShieldPowerup::OnEnable()
	{

	}

	void ShieldPowerup::OnDisable()
	{
		
	}

	void ShieldPowerup::SetVFXActive(bool aActive, int idx, bool loop)
	{
		KE::VFXRenderInput vfxInput(ownerPlayer->GetGameObject().myWorldSpaceTransform, loop, false);

		if (aActive)
		{
			vfxInterface->TriggerVFXSequence(idx, vfxInput);
		}
		else
		{
			vfxInterface->StopVFXSequence(idx, vfxInput);
		}
	}

	bool ShieldPowerup::OnPlayerAction(PlayerActionType aAction, ActionState aState, PowerupInputData& input)
	{
		if (aAction == PlayerActionType::TakeDamage)
		{
			if (!input.player) { return false; }

			if (invulnerabilityTimer > 0.0f)
			{
				return true;
			}

			SetVFXActive(true, 1, false);
			SetVFXActive(false, 1);

			invulnerabilityTimer = invulnerabilityTime;
			return true;
		}

		return false;
	}

}
