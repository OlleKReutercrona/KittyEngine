#include "stdafx.h"
#include "TelekinesisPowerup.h"

#include "Boomerang/BoomerangPhysxController.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/VFXComponent.h"
#include "Player/Player.h"

#include "Project/Source/Boomerang/BoomerangComponent.h"
#include "Engine/Source/Audio/GlobalAudio.h"

namespace P8
{
	void TelekinesisPowerup::Update(const PowerupInputData& aInputData)
	{

	}

	void TelekinesisPowerup::OnPickup()
	{
		
	}

	void TelekinesisPowerup::OnDrop()
	{
		
	}

	void TelekinesisPowerup::OnEnable()
	{

	}

	void TelekinesisPowerup::OnDisable()
	{
		
	}

	void TelekinesisPowerup::SetVFXActive(bool aActive, int idx, bool loop)
	{

	}

	bool TelekinesisPowerup::OnPlayerAction(PlayerActionType aAction, ActionState aState, PowerupInputData& input)
	{
		if (aAction == PlayerActionType::Recall) 
		{
			

			if (!KE::GlobalAudio::IsSFXPlaying(sound::SFX::PowerupTelekinesis))
			{
				std::cout << "RECALL TELEKINESIS\n";
				KE::GlobalAudio::PlaySFX(sound::SFX::PowerupTelekinesis);
			} 

			auto& thrownBalls = input.player->GetBoomerangs()->thrownBoomerangs;
			switch(aState)
			{
			case ActionState::Begin:
			{
				myHasMoved = false;

			}
			case ActionState::Ongoing:
			{
				Vector3f moveDir = ownerPlayer->GetMoveDirection();
				if (moveDir.LengthSqr() > 0.1f && !myHasMoved)
				{
					myHasMoved = true;
					for (auto* ball : thrownBalls)
					{
						ball->EndRecall();
					}
				}

				if (myHasMoved)
				{
					for (auto* ball : thrownBalls)
					{

						auto& state = ball->GetThrowParameters().state;
						if (state == BoomerangState::PickupAnim) { continue; }
						state = BoomerangState::Telekinesis;
						//ball->Kill();

						Vector3f ballVelocity = ball->GetVelocity();
						float speed = ballVelocity.Length();

						Vector3f addedVelocity = moveDir.GetNormalized() * KE_GLOBAL::deltaTime * 50.0f;

						Vector3f ballPos        = ball->GetController()->GetPosition();
						Vector3f playerPos      = ownerPlayer->GetGameObject().myWorldSpaceTransform.GetPosition();
						Vector3f toPlayer = (playerPos - ballPos);

						Vector3f finalMoveDirection = addedVelocity;

						auto* gfx = KE_GLOBAL::blackboard.Get<KE::Graphics>();
						auto& dbg = gfx->GetDebugRenderer();

						constexpr float maxDist = 10.0f;

						if (toPlayer.Length() > maxDist)
						{
							ball->GetController()->AddVelocity(toPlayer.GetNormalized() * 2.0f);
						}

						ball->GetController()->AddVelocity(finalMoveDirection);
						ball->GetController()->CapVelocity(10.0f);
								
						dbg.RenderSphere(playerPos, maxDist);
						dbg.RenderLine(ballPos, ballPos + toPlayer.GetNormalized() * 2.0f, {0.0f, 1.0f, 1.0f, 1.0f});
						dbg.RenderLine(ballPos, ballPos + finalMoveDirection * 2.0f, {1.0f, 0.0f, 1.0f, 1.0f});

					}
				}
				break;
			}
			case ActionState::End:
			{
				std::cout << "END TELEKINESIS\n";
				KE::GlobalAudio::StopSFX(sound::SFX::PowerupTelekinesis);
				for (auto* ball : thrownBalls)
				{
					ball->GetThrowParameters().state = BoomerangState::Die;
					ball->Kill();

				}
				break;
			}
			}


		}

		return false;
	}

}
