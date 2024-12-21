#include "stdafx.h"
#include "BoomerangPhysxController.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/Math/KittyMath.h"
#include "Engine/Source/Graphics/DebugRenderer.h"
#include "Project/Source/Boomerang/BoomerangComponent.h"

#include "Project/Source/GameEvents/GameEvents.h"
#include "Engine/Source/Utility/EventSystem.h"
#include "Player/Player.h"
#include "Engine/Source/Audio/GlobalAudio.h"

namespace P8
{
	std::pair<int, KE::PhysicsObject*> P8::BoomerangPhysicsController::Create(
		physx::PxPhysics* aPhysics,
		physx::PxControllerManager* aManager,
		const PhysxControllerData& aControllerData,
		const KE::CharacterControllerUserData& aUserData
	)
	{
		myUserData = aUserData;
		myUserData.myID = myUserData.gameObject->myID;
		myMaterial = aPhysics->createMaterial(
			aControllerData.physicsMaterial.x,
			aControllerData.physicsMaterial.y,
			aControllerData.physicsMaterial.z
		);
		myTransform = &myUserData.gameObject->myTransform;
		Vector3f& positionRef = myTransform->GetPositionRef();

		physx::PxCapsuleControllerDesc desc;
		desc.stepOffset = 0.0f; //aControllerData.myStepOffset;
		desc.contactOffset = aControllerData.skinWidth;
		desc.height = aControllerData.halfHeight;
		desc.radius = aControllerData.radius;
		desc.position = physx::PxExtendedVec3(positionRef.x, positionRef.y, positionRef.z);
		desc.material = myMaterial;
		desc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED; // TODO Expose this
		desc.reportCallback = this;

		myPhysicsObject.disableColliderMovementPrecedence = false;
		myPhysicsObject.ID = myUserData.myID;

		myController = aManager->createController(desc);
		//myController->getActor()->userData = (void*)&myPhysicsObject.ID;
		myController->getActor()->userData = &myUserData;
		myPhysicsObject.myActor = myController->getActor();

		filterData.word0 = static_cast<int>(KE::Collision::Layers::Projectile);
		filterData.word1 = static_cast<int>(KE::Collision::Layers::Wall);
		filterData.word1 |= static_cast<int>(KE::Collision::Layers::Player);
		filterData.word1 |= static_cast<int>(KE::Collision::Layers::Trigger);


		physx::PxU32 nShapes = myController->getActor()->getNbShapes();
		for (physx::PxU32 i = 0; i < nShapes; i++)
		{
			physx::PxShape* shape;

			myController->getActor()->getShapes(&shape, 1, i);
			shape->setQueryFilterData(filterData);
			shape->setSimulationFilterData(filterData);

			shape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
			shape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, true);
			shape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);

		}
		myFilterData.mFilterData = &filterData;
		myFilterData.mFilterFlags = physx::PxQueryFlag::eSTATIC |
			physx::PxQueryFlag::eDYNAMIC;

		myFilterData.mCCTFilterCallback = &myFilterCallback;

		return std::make_pair(myPhysicsObject.ID, &myPhysicsObject);
	}

	void BoomerangPhysicsController::DebugRender()
	{
		if (!myThrowParams) { return; }
		auto& ct = *myThrowParams;
		if (!ct.throwerTransform) { return; }

		auto* dbg = KE_GLOBAL::blackboard.Get<KE::DebugRenderer>();
		const Transform* t = myThrowParams->throwerTransform;
		Vector3f origin = t->GetPosition();
		Vector3f facing = t->GetForward();
		float distance = 10.0f;

		dbg->RenderLine(origin, origin + facing * distance, { 1.0f,1.0f,1.0f,1.0f });
		dbg->RenderSphere(origin, ct.pickupRadius, { 0.0f,1.0f,1.0f,1.0f });
	}

	void BoomerangPhysicsController::Update(float aTimeDelta)
	{
		if(!myThrowParams) { return; }	
		auto& ct = *myThrowParams;

		if (ct.state == BoomerangState::Flying)
		{
			PowerupInputData powerupInput = {ct.throwerPlayer, myBoomerangComponent};
			if (
				ct.throwerPlayer->GetPowerups().OnBoomerangAction(
				BoomerangAction::Fly,
				ActionState::Ongoing, 
				powerupInput
			)) { return; }
		}

		UpdatePosition(aTimeDelta);

		if(ct.state != BoomerangState::Die && ct.state != BoomerangState::Telekinesis) 
		{
			CalculateVelocity(aTimeDelta);
		}


		myFlightTime += aTimeDelta;
		ct.flightTime += aTimeDelta;
		myBounceSoundTimer += aTimeDelta;

		float decayMagnitude = myBounceValue >= ct.dyingThreshold ? myMaxDecay : myDefaultDecay;
		float decay = 1.0f - (decayMagnitude * aTimeDelta);
		myVelocity *= decay;
	}

	void BoomerangPhysicsController::CalculateVelocity(float aTimeDelta)
	{
		auto& ct = *myThrowParams;
		const Transform& target = *ct.throwerTransform;
		
		Vector3f targetPos = target.GetPosition() - myTransform->GetPosition();
		targetPos.y = 0.0f;
		Vector3f directionBack = targetPos.GetNormalized();
		float distance = (myTransform->GetPosition() - target.GetPosition()).Length();
		
		const float adjustedReturnDelay = ct.returnDelay + myBounceCount * 0.1f;
		const float t = std::min((myFlightTime / adjustedReturnDelay), 1.0f);
		const float percentage = t * t * t * t * t;

		float distanceToMax = distance - (ct.flightMaxDistance - ct.returnMultiplierRadius);
		float targetSpeed = ct.maxSpeed * percentage;
		const float returnMultiplier = targetSpeed * (std::max(0.0f, distanceToMax) / 3.0f);


		Vector3f targetVelocity = directionBack * (targetSpeed + returnMultiplier);
		
		myVelocity += targetVelocity * aTimeDelta;
	}

	void BoomerangPhysicsController::SetRadius(float aRadius)
	{
		physx::PxShape* shape;
		myController->getActor()->getShapes(&shape, 1, 0);
		shape->setGeometry(physx::PxCapsuleGeometry(aRadius, 0.05f));
	}

	void BoomerangPhysicsController::UpdatePosition(float aTimeDelta)
	{
		auto& ct = *myThrowParams;

		Vector3f diff = myTransform->GetPosition() - ct.throwerTransform->GetPosition();
		diff.y = 0.0f;
		float distance = diff.Length();
		bool withinPickupRange = (distance < ct.pickupRadius);
		bool minFlightTimeExceeded = myFlightTime > myMinFlightBeforePickup;

		if (ct.state == BoomerangState::PickupAnim)
		{
			const float pickupTime = 0.25f;
			myPickupAnimationTimer += aTimeDelta;

			Vector3f finalPos = CommonUtilities::Lerp(
				myTransform->GetPosition(), 
				ct.throwerTransform->GetPosition(),
				myPickupAnimationTimer / pickupTime
			);

			myBoomerangComponent->SetSize(
				(std::max)(ct.size * ct.sizeMult * (1.0f - myPickupAnimationTimer / pickupTime), 0.1f)
			);

			finalPos.y = 0.0f;
			SetPosition(finalPos);

			if (myPickupAnimationTimer / pickupTime >= 1.0f)
			{
				ct.state = BoomerangState::PickedUp;
				myPickupAnimationTimer = 0.0f;
			}

			return;

		}
		else if (withinPickupRange && minFlightTimeExceeded && ct.state != BoomerangState::PickedUp) 
		{
			ct.state = BoomerangState::PickupAnim;
			return;
		}

		// Cap movement to max speed.
		if (myVelocity.Length() > ct.maxSpeed) {
			myVelocity.Normalize();
			myVelocity *= ct.maxSpeed;
		}

		physx::PxVec3 displacement(myVelocity.x, 0, myVelocity.z);
		const physx::PxControllerCollisionFlags collisionFlags = myController->move(
			displacement * aTimeDelta,
			0.0f,
			aTimeDelta,
			myFilterData,
			NULL
		);

		physx::PxExtendedVec3 newPos = myController->getFootPosition();
		myTransform->SetPosition({
			static_cast<float>(newPos.x),
			static_cast<float>(newPos.y) + 0.256f, // This weird value makes the ball look like it's on the ground.
			static_cast<float>(newPos.z)
			}
		);
	}

	void BoomerangPhysicsController::Throw(const Vector3f& aVelocity, const Vector3f& aOrigin, BallThrowParams& aThrowParams)
	{
		myThrowParams = &aThrowParams;
		myThrowParams->state = BoomerangState::Flying;
		myVelocity = aVelocity;
		myFlightTime = 0.0f;
		myBounceValue = 0.0f;
		myBounceCount = 0;

		myBoomerangComponent->SetSize(0.25f);

		SetPosition(aOrigin/* + aVelocity.GetNormalized() * 1.0f*/);
	}

	void BoomerangPhysicsController::Deflect(const Vector3f& aNormal)
	{
		if (!myThrowParams) { return; }
		auto& ct = *myThrowParams;

		PowerupInputData powerupInput = { ct.throwerPlayer, myBoomerangComponent };
		if (
			ct.throwerPlayer->GetPowerups().OnBoomerangAction(
				BoomerangAction::Bounce,
				ActionState::Begin,
				powerupInput
			)) {
			return;
		}

		const Vector3f forward = myVelocity.GetNormalized();
		const float currentSpeed = myVelocity.Length();

		const float wallDot = forward.Dot(aNormal);
		const Vector3f bounceDirection = forward - (2.0f * wallDot * aNormal);

		const float bounceDot = forward.Dot(bounceDirection);
		const float magnitude = abs(std::min(0.0f, bounceDot));

		float bounceValue = std::min(magnitude * (ct.maxSpeed - currentSpeed), ct.maxBounceValue);
		float timeMultiplier = std::min(1.0f, myFlightTime / 0.5f);
		myBounceValue += ct.minBounceValue + (bounceValue * timeMultiplier);

		const float maxFallOff = myBounceValue >= ct.dyingThreshold ? 0.5f : 0.30f;
		const float speedToKeep = 1.0f - (maxFallOff * (magnitude * magnitude));
		const float bounceSpeed = currentSpeed * speedToKeep;

		myVelocity = bounceDirection.GetNormalized() * bounceSpeed;
		myBounceCount++;

		// If we bounced too much we stop calculating new velocity.
		if (myBounceValue >= ct.dyingThreshold)
		{
			ct.state = BoomerangState::Die;
		}
	}

	void BoomerangPhysicsController::Bounce(const Vector3f& aNormal)
	{
		auto& ct = *myThrowParams;

		PowerupInputData powerupInput = {ct.throwerPlayer, myBoomerangComponent};
		if (
			ct.throwerPlayer->GetPowerups().OnBoomerangAction(
			BoomerangAction::Bounce,
			ActionState::Begin, 
			powerupInput
		)) { return; }

		const Vector3f forward = myVelocity.GetNormalized();
		const float currentSpeed = myVelocity.Length();

		const float wallDot = forward.Dot(aNormal);
		const Vector3f bounceDirection = forward - (2.0f * wallDot * aNormal);

		const float bounceDot = forward.Dot(bounceDirection);
		const float magnitude = abs(std::min(0.0f, bounceDot));

		float bounceValue = std::min(magnitude * (ct.maxSpeed - currentSpeed), ct.maxBounceValue);
		float timeMultiplier = std::min(1.0f, myFlightTime / 0.5f);
		myBounceValue += ct.minBounceValue + (bounceValue * timeMultiplier);

		const float maxFallOff = myBounceValue >= ct.dyingThreshold ? 0.5f : 0.30f;
		const float speedToKeep = 1.0f - (maxFallOff * (magnitude * magnitude));
		const float bounceSpeed = currentSpeed * speedToKeep;
		
		myVelocity = bounceDirection.GetNormalized() * bounceSpeed;
		myBounceCount++;

		// If we bounced too much we stop calculating new velocity.
		if(myBounceValue >= ct.dyingThreshold) 
		{
			ct.state = BoomerangState::Die;
		}

		// Bounce is sometimes called multiple times within a few frames but we dont want sound on all of those hits.

		bool soundOffCooldown  = myBounceSoundTimer > 0.04f;

		float speedTowardsWall = myVelocity.Dot(aNormal);
		float reqSpeedToPlaySound = ct.maxSpeed * 0.06f;

		if (soundOffCooldown && (speedTowardsWall > reqSpeedToPlaySound))
		{
			//float volume = std::max(currentSpeed / ct.maxSpeed, 0.2f);

			// Scaled volume is not working atm, therefor commented out.
			KE::GlobalAudio::PlaySFX(sound::SFX::BallBounce/*, volume*/);
			myBounceSoundTimer = 0.0f;
		}
	}

	void BoomerangPhysicsController::Recall()
	{
		if (!myThrowParams) { return; }
		auto& ct = *myThrowParams;

		if (ct.state == BoomerangState::PickedUp || !ct.throwerTransform) { return; }
		if (myFlightTime < ct.returnDelay) { return; }
		myFlightTime = 0.25f;
		myBounceValue = 0.0f;

		const Transform& target = *ct.throwerTransform;
		Vector3f targetPos = target.GetPosition() - myTransform->GetPosition();
		targetPos.y = 0.0f;
		Vector3f dirToTarget = targetPos.GetNormalized();

		const float distance = targetPos.Length();
		const float magnitude = ct.maxSpeed * (distance / ct.slowRadius);
		const float targetSpeed = distance > ct.slowRadius ? ct.maxSpeed : magnitude;

		Vector3f targetVelocity = dirToTarget * targetSpeed;
		Vector3f result = targetVelocity - myVelocity;

		myVelocity += result * KE_GLOBAL::deltaTime;
	}

	void BoomerangPhysicsController::onShapeHit(const physx::PxControllerShapeHit& aHit)
	{
		Vector3f normal = { aHit.worldNormal.x, 0.0f, aHit.worldNormal.z };
		normal.Normalize();
		Bounce(normal);

		const auto& hitUserData = *static_cast<const KE::PhysxShapeUserData*>(aHit.shape->userData);

		KE::PhysXCollisionData collisionData;
		collisionData.objHit = myUserData.gameObject;

		KE::GameObjectCollisionInterface::OnCollision(hitUserData.gameObject, collisionData);
	}

	void BoomerangPhysicsController::onControllerHit(const physx::PxControllersHit& aHit)
	{
		const auto& hitUserData = *static_cast<KE::CharacterControllerUserData*>(aHit.other->getActor()->userData);

		const Vector3f hitNormal = { aHit.worldNormal.x, aHit.worldNormal.y, aHit.worldNormal.z };

		if (hitUserData.objType == KE::CharacterControllerUserData::Type::Boomerang)
		{
			const Vector3f boomerangForward = myVelocity.GetNormalized();
			const float dot = boomerangForward.Dot(hitNormal);
			const Vector3f reflected = boomerangForward - (2.0f * dot * hitNormal);
			myVelocity = reflected * myVelocity.Length();
		}

		KE::PhysXCollisionData collisionData;
		collisionData.objHit = hitUserData.gameObject;

		KE::GameObjectCollisionInterface::OnCollision(myUserData.gameObject, collisionData);
	}
	void BoomerangPhysicsController::onObstacleHit(const physx::PxControllerObstacleHit& aHit)
	{

	}
}

//void KE::GameObjectCollisionInterface::OnCollision(GameObject* aGameObject, const PhysXCollisionData& aData)
//{
//	aGameObject->OnPhysXCollision(aData);
//}