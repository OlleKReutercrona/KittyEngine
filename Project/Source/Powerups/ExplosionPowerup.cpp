#include "stdafx.h"
#include "ExplosionPowerup.h"

#include "Boomerang/BoomerangPhysxController.h"
#include "Project/Source/Boomerang/BoomerangComponent.h"
#include "Project/Source/Player/Player.h"
#include "Engine/Source/Math/KittyMath.h"
#include "Engine/Source/Collision/PhysicsCharacterController.h"
#include "Engine/Source/Graphics/FX/VFX.h"
#include "Managers/BallManager.h"
#include "Engine/Source/Audio/GlobalAudio.h"

namespace P8
{
	ExplosionBoomerangPowerup::ExplosionBoomerangPowerup()
	{
		if (!ourExplosionVFXBufferInitialised)
		{
			ourExplosionVFXBufferInitialised = true;

			auto* gfx = KE_GLOBAL::blackboard.Get<KE::Graphics>();

			D3D11_BUFFER_DESC bufferDesc = {};
			bufferDesc.BindFlags              = D3D11_BIND_CONSTANT_BUFFER;
			bufferDesc.Usage                  = D3D11_USAGE_DYNAMIC;
			bufferDesc.CPUAccessFlags         = D3D11_CPU_ACCESS_WRITE;
			bufferDesc.MiscFlags              = 0u;
			bufferDesc.ByteWidth              = sizeof(ExplosionVFXBufferData);
			bufferDesc.StructureByteStride    = 0u;

			ourExplosionVFXBuffer.Init(gfx->GetDevice().Get(), &bufferDesc);

			gfx->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH "Explosion_VFX_PS.cso");
		}


	}

	void ExplosionBoomerangPowerup::Update(const PowerupInputData& aInputData)
	{
		for (auto& explosion : explosions)
		{
			UpdateExplosion(explosion);
		}
	}

	void ExplosionBoomerangPowerup::ExplodeBall(PowerupInputData& input, BallThrowParams& throwParams)
	{
		throwParams.hasExploded = true;
		input.boomerang->Kill();
		input.boomerang->GetController()->CapVelocity(0.0f);


		auto* physxScene = input.player->GetPhysxController().GetPhysicsObject().myActor->getScene();
		if (!physxScene)
		{
			return;
		}

		float maxDistance = 3.0f * throwParams.sizeMult * throwParams.sizeMult;
		Vector3f position = input.boomerang->GetGameObject().myTransform.GetPosition();
		position.y += 0.5f;
		physx::PxVec3 physxPosition = { position.x, position.y, position.z };

		Explosion& explosion = GetFreeExplosion();
		InitExplosion(explosion);
		explosion.position = position;
		explosion.vfxTransform.SetPosition(position);

		CalculateRanges(explosion, physxScene);

		CameraShakeEvent msg;
		msg.data.shakefactor = 0.030f;
		msg.data.shakeTime = 0.25f;
		msg.data.explosionPosition = position;
		ES::EventSystem::GetInstance().SendEvent(msg);

		KE::GlobalAudio::PlaySFX(sound::SFX::PowerupExplosiveBall);

		//for (int i = 0; i < explosion.slices; i++)
		//{
		//	// Set the position of the explosion transforms
		//	explosion.transforms[i]->SetScale({ 0.1f, 0.5f, 0.5f });
		//	explosion.transforms[i]->SetPosition(position);
		//
		//	Vector3f forward = explosion.transforms[i]->GetForward();
		//	physx::PxVec3 physxDirection = { forward.x, forward.y, forward.z };
		//
		//	physx::PxRaycastBuffer hitBuffer;
		//	if(physxScene->raycast(physxPosition, physxDirection, maxDistance, hitBuffer))
		//	{
		//		if (hitBuffer.hasBlock)
		//		{
		//			explosion.ranges[i] = (std::min)(hitBuffer.block.distance, maxDistance);
		//		}
		//	}
		//	else
		//	{
		//		explosion.ranges[i] = maxDistance;
		//	}
		//	explosion.ranges[i] += 0.5f;
		//}

		// Trigger VFX
		KE::VFXRenderInput vfxInput(explosion.vfxTransform, false, false);
		vfxInput.customBufferInput = {
			&ourExplosionVFXBuffer,
			&explosion.vfxBufferData,
			7,
			sizeof(ExplosionVFXBufferData)
		};

		vfxInterface->TriggerVFXSequence(0, vfxInput);

		input.boomerang->SetThrowCooldown(1.0f);
		input.player->GetBoomerangs()->Pickup(input.boomerang);
	}

	bool ExplosionBoomerangPowerup::OnBoomerangAction(BoomerangAction aAction, ActionState aState, PowerupInputData& input)
	{
		if (!ValidateConditions(aAction, aState, input)) { return false; };

		auto& throwParams = input.boomerang->GetThrowParameters();

		if (throwParams.flightTime >= 0.5f && !throwParams.hasExploded)
		{
			ExplodeBall(input, throwParams);
		}

		return false;
	}

	void ExplosionBoomerangPowerup::InitExplosion(Explosion& anExplosion)
	{
		float increment = 360.0f / Explosion::slices;

		//for (int i = 0; i < Explosion::slices; i++)
		//{
		//	float rad = KE::DegToRad(increment * i);
		//	anExplosion.transforms[i]->SetRotation(rad);
		//}
		anExplosion.explosionTimer = 0.0f;
		anExplosion.vfxTransform.SetScale({1.0f, 1.0f, 1.0f});
		//anExplosion.hitRangesTemp.clear();
	}

	void ExplosionBoomerangPowerup::UpdateExplosion(Explosion& anExplosion)
	{
		anExplosion.explosionDuration = 0.5f;
		if (anExplosion.explosionTimer >= anExplosion.explosionDuration) { return; }
		anExplosion.explosionTimer += KE_GLOBAL::deltaTime;
		if (anExplosion.explosionTimer >= anExplosion.explosionDuration)
		{
			KE::VFXRenderInput vfxInput(anExplosion.vfxTransform, false, false);
			vfxInterface->StopVFXSequence(0, vfxInput);
		}

		float highestRange = 0.0f;

		float timeFactor = anExplosion.explosionTimer / anExplosion.explosionDuration;

		float targetSize = anExplosion.explosionRange * anExplosion.scaleFactor * timeFactor;
		auto scl = anExplosion.vfxTransform.GetScale();
		anExplosion.vfxTransform.SetScale( Vector3f
		{
			targetSize,
			1.0f,
			targetSize
		});

#pragma region debugRendering
		//auto* gfx = KE_GLOBAL::blackboard.Get<KE::Graphics>();
		//auto& dbg = gfx->GetDebugRenderer();
		//auto& blocks = anExplosion.vfxBlocksTemp;
		//for (int i = 0; i < blocks.size(); i++)
		//{
		//	auto& block = blocks[i];
		//	if (block.rangeMax < 0.0f) { continue; }
		//	if (block.rangeMin < 0.0f) { continue; }
		//
		//	Vector4f color = i % 2 == 0 ? Vector4f{ 1.0f, 0.0f, 1.0f, 1.0f } : Vector4f{ 0.0f, 1.0f, 1.0f, 1.0f };
		//
		//	Vector3f minAnglePoint = anExplosion.vfxTransform.GetPosition() + Vector3f
		//	{
		//		sin(block.angleMin) * block.rangeMin,
		//		0.0f,
		//		cos(block.angleMin) * block.rangeMin
		//	};
		//	Vector3f maxAnglePoint = anExplosion.vfxTransform.GetPosition() + Vector3f
		//	{
		//		sin(block.angleMax) * block.rangeMax,
		//		0.0f,
		//		cos(block.angleMax) * block.rangeMax
		//	};
		//
		//	dbg.RenderLine(minAnglePoint, maxAnglePoint, color);
		//}
		//
		//for (auto& [angle, range, normal] : anExplosion.hitRangesTemp)
		//{
		//	if (range < 0.0f) { continue; }
		//	Vector3f point = anExplosion.vfxTransform.GetPosition() + Vector3f
		//	{
		//		sin(angle) * range,
		//		0.0f,
		//		cos(angle) * range
		//	};
		//
		//	//dbg.RenderLine(point, point + normal, { 1.0f, 1.0f, 0.0f, 1.0f });
		//}

		//for (int i = 0; i < Explosion::slices; i++)
		//{
		//	// Set the position of the explosion transforms
		//	float dist = (anExplosion.transforms[i]->GetPositionRef() - anExplosion.position).Length();

		//	if (dist >= anExplosion.ranges[i])
		//	{
		//		KE::VFXRenderInput vfxInput(*anExplosion.transforms[i], false, false);
		//		vfxInterface->StopVFXSequence(0, vfxInput);
		//		continue;
		//	}
		//	highestRange = (std::max)(highestRange, dist);

		//	anExplosion.transforms[i]->TranslateLocal({ 0.0f, 0.0f, 2.0f * KE_GLOBAL::deltaTime });

		//	auto scl = anExplosion.transforms[i]->GetScale();
		//	anExplosion.transforms[i]->SetScale(scl + Vector3f
		//	{
		//		KE_GLOBAL::deltaTime * 1.75f,
		//		0.0f,
		//		KE_GLOBAL::deltaTime
		//	});
		//}

		//dbg.RenderSphere(anExplosion.position, highestRange, { 1.0f, 0.0f, 1.0f, 1.0f});
#pragma endregion

		//iterate through all players
		for (auto& player : ownerPlayer->GetBallManager()->GetPlayers())
		{
			auto& playerPos = player->GetGameObject().myTransform.GetPosition();
			Vector3f toPlr = (playerPos - anExplosion.position);
			float dist = toPlr.Length();

			if (dist > targetSize / 1.5f) { continue; }

			float angle = atan2(toPlr.z, toPlr.x);
			angle -= KE::PI / 2.0f;
			if (angle < 0.0f) { angle += KE::PI * 2.0f; }

			bool blocked = false;
			for (int i = 0; i < std::size(anExplosion.vfxBufferData.blocks); i++)
			{
				auto& block = anExplosion.vfxBufferData.blocks[i];
				if (block.rangeMax < 0.0f) { continue; }
				if (block.rangeMin < 0.0f) { continue; }

				if (angle >= block.angleMin && angle <= block.angleMax)
				{
					float angleT = (angle - block.angleMin) / (block.angleMax - block.angleMin);
					float range = std::lerp(block.rangeMin, block.rangeMax, angleT);

					if (dist < range)
					{
						blocked = true;
						break;
					}
				}
			}

			if (blocked) { continue; }
			player->TakeDamage(ownerPlayer->GetIndex(), false);

		}


	}

	Explosion& ExplosionBoomerangPowerup::GetFreeExplosion()
	{
		for (auto& explosion : explosions)
		{
			if (explosion.explosionTimer >= explosion.explosionDuration)
			{
				return explosion;
			}
		}

		return explosions.emplace_back();
	}

	void ExplosionBoomerangPowerup::CalculateRanges(Explosion& anExplosion, physx::PxScene* aScene)
	{


		float angle = 0.01f;
		const float maxDistance = 3.0f * anExplosion.scaleFactor;

		std::vector<HitRange> hitRanges;
		

		while (angle <= KE::PI * 2.0f)
		{

			Vector3f position = anExplosion.vfxTransform.GetPosition();
			Vector3f direction = { sin(angle), 0.0f, cos(angle) };

			physx::PxVec3 physxPosition = { position.x, position.y, position.z };
			physx::PxVec3 physxDirection = { direction.x, direction.y, direction.z };

			float hitRange = -1.0f;

			Vector3f hitNormal = { 0.0f, 0.0f, 0.0f };

			physx::PxQueryFilterData filter;
			filter.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC;
			filter.data.word0 |= (int)KE::Collision::Layers::Wall;
			physx::PxRaycastBuffer hitBuffer;
			if(aScene->raycast(
				physxPosition,
				physxDirection, 
				maxDistance, 
				hitBuffer,
				physx::PxHitFlag::eDEFAULT,
				filter
				))
			{
				if (hitBuffer.hasBlock && hitBuffer.block.distance < maxDistance)
				{
					hitRange = hitBuffer.block.distance;
					hitNormal = { hitBuffer.block.normal.x, hitBuffer.block.normal.y, hitBuffer.block.normal.z };
				}
			}

			hitRanges.push_back({angle, hitRange, hitNormal});

			angle += KE::PI / 128.0f; //overkill!
		}

		//anExplosion.hitRangesTemp = hitRanges;

		using VFXBlock = ExplosionVFXBufferData::ExplosionVFXBlock;
		std::vector<VFXBlock> blocks;
		VFXBlock* currentBlock = nullptr;

		Vector3f workingNormal = { 0.0f, 0.0f, 0.0f };

		for (int i = 0; i < hitRanges.size(); i++)
		{
			bool hasHit = hitRanges[i].range >= 0.0f;
			float normalDot = hitRanges[i].normal.Dot(workingNormal);

			bool shouldBeginBlock = hasHit && !currentBlock;
			bool shouldEndBlock = (!hasHit || normalDot < 0.8f) && currentBlock;

			if (shouldBeginBlock)
			{
				currentBlock = &blocks.emplace_back();
				currentBlock->angleMin = hitRanges[i].angle;
				currentBlock->rangeMin = hitRanges[i].range;
				workingNormal = hitRanges[i].normal;
			}
			else if (shouldEndBlock)
			{
				if (hasHit)
				{
					currentBlock->angleMax = hitRanges[i].angle;
					currentBlock->rangeMax = hitRanges[i].range;
				}
				else
				{
					for (int j = i; j >= 0; j--)
					{
						if (hitRanges[j].range >= 0.0f)
						{
							currentBlock->angleMax = hitRanges[j].angle;
							currentBlock->rangeMax = hitRanges[j].range;
							break;
						}
					}

				}
				currentBlock = nullptr;
			}
		}

		//anExplosion.vfxBlocksTemp = blocks;

		int count = 0;
		memset(&anExplosion.vfxBufferData, 0, sizeof(ExplosionVFXBufferData));
		for (auto& block : blocks)
		{
			if (block.rangeMax < 0.0f) { continue; }
			if (block.rangeMin < 0.0f) { continue; }

			anExplosion.vfxBufferData.blocks[count] = block;
			count++;
			if (count >= explosionRangeBlockCount) { break; }
		}

	}

	bool ExplosionBoomerangPowerup::ValidateConditions(BoomerangAction aAction, ActionState aState, PowerupInputData& input)
	{
		auto& throwParams = input.boomerang->GetThrowParameters();

		if (!input.player || !input.boomerang) { return false; }
		if (!(
			throwParams.state == BoomerangState::Flying ||
			throwParams.state == BoomerangState::Recalled ||
			throwParams.state == BoomerangState::Telekinesis
		))
		{
			return false;
		}
			
		if (!(
			aAction == BoomerangAction::Fly ||
			aAction == BoomerangAction::Recall
		))
		{
			return false;
		}

		return true;
	}
}
