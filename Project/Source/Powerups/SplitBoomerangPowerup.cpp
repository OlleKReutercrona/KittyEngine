#include "stdafx.h"
#include "SplitBoomerangPowerup.h"

#include "Boomerang/BallThrowParams.h"
#include "Boomerang/BoomerangComponent.h"
#include "Boomerang/BoomerangPhysxController.h"
#include "Engine/Source/Math/KittyMath.h"
#include "Engine/Source/Math/Matrix3x3.h"
#include "Managers/BallManager.h"
#include "Player/Player.h"
#include "Engine/Source/Audio/GlobalAudio.h"

namespace P8
{
	void SplitBoomerangPowerup::Update(const PowerupInputData& aInputData)
	{
	}

	void SplitBoomerangPowerup::OnPickup()
	{
		
	}

	void SplitBoomerangPowerup::OnDrop()
	{
		
	}

	void SplitBoomerangPowerup::OnEnable()
	{
		
	}

	void SplitBoomerangPowerup::OnDisable()
	{
		
	}

	bool SplitBoomerangPowerup::OnPlayerAction(PlayerActionType aAction, ActionState aState, PowerupInputData& input)
	{
		return false;
	}

	bool SplitBoomerangPowerup::OnBoomerangAction(BoomerangAction aAction, ActionState aState, PowerupInputData& input)
	{
		if (aAction == BoomerangAction::Fly && aState == ActionState::Ongoing)
		{
			if (!input.player || !input.boomerang) { return false; }

			auto& throwParams = input.boomerang->GetThrowParameters();
			if (throwParams.state != BoomerangState::Flying) { return false; }
			if (throwParams.flightTime >= 0.15f && !throwParams.isCopy && !throwParams.hasSplit)
			{
				throwParams.hasSplit = true;

				KE::GlobalAudio::PlaySFX(sound::SFX::PowerupSplitBall);

				for (int i = 0; i < 4; i++)
				{
					auto* splitBall = input.player->GetBallManager()->GetUnusedBall();
					splitBall->SyncCharacterIndex(input.player);

					const float angleAmount = 45.0f;
					int ii = i - 2;
					if (ii >= 0) {ii++;} 

					float angle = angleAmount * static_cast<float>(ii);
					float rad = angle * KE::DegToRadImmediate;

					constexpr float velocityLossFactor = 1.0f;
					constexpr float scaleFactor = 0.5f;

					Matrix3x3f throwTransform = Matrix3x3f::CreateRotationAroundY(rad);
					Vector3f veloc = input.boomerang->GetController()->GetVelocity();
					input.boomerang->GetController()->SetVelocity(veloc * velocityLossFactor);

					Vector3f dir = veloc.GetNormalized();
					dir = throwTransform * dir;
					dir.Normalize();
					dir *= veloc.Length() * velocityLossFactor;

					BallThrowParams newThrowParams = input.boomerang->GetThrowParameters();
					newThrowParams.isCopy = true;
					newThrowParams.splitFrom = input.boomerang;
					newThrowParams.sizeMult = scaleFactor;

					throwParams.sizeMult = scaleFactor;

					splitBall->Throw(
						input.boomerang->GetController()->GetPosition(),
						dir, 
						newThrowParams
					);

					input.boomerang->SetSize(throwParams.size);

					input.player->GetBoomerangs()->RegisterThrown(splitBall);

				}


				return true;
			}


		}

		return false;
	}
}
