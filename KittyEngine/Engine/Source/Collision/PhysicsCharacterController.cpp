#include "stdafx.h"
#include "PhysicsCharacterController.h"

#include <External/Include/physx/PxActor.h>
#include <External/Include/physx/PxRigidDynamic.h>
#include <External/Include/physx/PxRigidStatic.h>
#include <External/Include/physx/PxRigidActor.h>

#include "Engine/Source/Math/KittyMath.h"
#include "Layers.h"
#include "ComponentSystem/GameObject.h"
#include "Project/Source/Boomerang/BoomerangPhysxController.h"
#include "Project/Source/GameEvents/GameEvents.h"

#include "Project/Source/Player/Player.h"
#include "Project/Source/Boomerang/BoomerangComponent.h"
#include "Project/Source/MovingObject/MovingObject.h"

namespace P8
{
	struct SlowMotionEvent;
}

namespace KE
{
	bool CharacterControllerFilter::filter(const physx::PxController& a, const physx::PxController& b)
	{
		const auto& objAData = *static_cast<CharacterControllerUserData*>(a.getActor()->userData);
		const auto& objBData = *static_cast<CharacterControllerUserData*>(b.getActor()->userData);

		if (objAData.team == objBData.team)
		{
			return false;
		}
		if (
			objAData.objType == CharacterControllerUserData::Type::Boomerang &&
			objBData.objType == CharacterControllerUserData::Type::Player ||
			objAData.objType == CharacterControllerUserData::Type::Player &&
			objBData.objType == CharacterControllerUserData::Type::Boomerang
			)
		{
			return false;
		}

		return true;
	}

	physx::PxQueryHitType::Enum CharacterControllerFilter::preFilter(const physx::PxFilterData& filterData,
		const physx::PxShape* shape, const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags)
	{
		return physx::PxQueryHitType::eBLOCK;
	}

	physx::PxQueryHitType::Enum CharacterControllerFilter::postFilter(
		const physx::PxFilterData& filterData,
		const physx::PxQueryHit& hit,
		const physx::PxShape* shape,
		const physx::PxRigidActor* actor
	)
	{
		auto fd = shape->getQueryFilterData();

		if (fd.word0 & static_cast<int>(KE::Collision::Layers::MovingObject))
		{
			auto* UD = (PhysxShapeUserData*)shape->userData;
			
			if (UD && UD->gameObject)
			{
				if (P8::MovingObject* movingObj; UD->gameObject->TryGetComponent(movingObj))
				{
					if (movingObj->IsMoving()) { return physx::PxQueryHitType::eBLOCK; }

					/*return physx::PxQueryHitType::eTOUCH;*/
				}
			}
		}

		if (
			filterData.word0 & static_cast<int>(KE::Collision::Layers::PlayerAttack)
			)
		{
			if (fd.word0 & static_cast<int>(KE::Collision::Layers::Player))
			{
				return physx::PxQueryHitType::eTOUCH;
			}
		}

		if (!(filterData.word1 & shape->getQueryFilterData().word0))
		{
			return physx::PxQueryHitType::eNONE;
		}
		if (!(filterData.word0 & shape->getQueryFilterData().word1))
		{
			return physx::PxQueryHitType::eNONE;
		}

		return physx::PxQueryHitType::eBLOCK;
	}


	//void GameObjectCollisionInterface::OnCollision(GameObject* aGameObject, const PhysXCollisionData& aData)
	//{
	//	aGameObject->OnPhysXCollision(aData);
	//}

	PhysicsCharacterController::PhysicsCharacterController() {}
	PhysicsCharacterController::~PhysicsCharacterController() {}

	std::pair<int, PhysicsObject*> PhysicsCharacterController::Create(
		physx::PxPhysics* aPhysics,
		physx::PxControllerManager* aControllerManager,
		const PhysxControllerData& aData,
		const CharacterControllerUserData& aUserData
	)
	{
		myUserData = aUserData;
		myUserData.myID = myUserData.gameObject->myID;
		myMaterial = aPhysics->createMaterial(aData.physicsMaterial.x, aData.physicsMaterial.y, aData.physicsMaterial.z);
		myTransform = &myUserData.gameObject->myTransform;
		Vector3f& positionRef = myTransform->GetPositionRef();

		physx::PxCapsuleControllerDesc desc;
		desc.slopeLimit = 0.707f; // Default value
		desc.stepOffset = aData.stepOffset;
		desc.contactOffset = 0.001f;
		desc.height = aData.halfHeight;
		desc.radius = aData.radius;
		desc.position = physx::PxExtendedVec3(positionRef.x, positionRef.y, positionRef.z);
		desc.material = myMaterial;
		desc.climbingMode = physx::PxCapsuleClimbingMode::eCONSTRAINED; // TODO Expose this
		desc.reportCallback = this;

		myPhysicsObject.myActor->userData;

		myPhysicsObject.myKECollider.myComponent;




		myPhysicsObject.disableColliderMovementPrecedence = false;
		myPhysicsObject.ID = myUserData.myID;

		myController = aControllerManager->createController(desc);


		//myController->getActor()->userData = (void*)&myPhysicsObject.ID;
		myPhysicsObject.myActor = myController->getActor();
		myPhysicsObject.myActor->userData = &myUserData;

		filterData.word0 = static_cast<int>(KE::Collision::Layers::Player);
		filterData.word1 = static_cast<int>(KE::Collision::Layers::Wall);
		filterData.word1 |= static_cast<int>(KE::Collision::Layers::Ground);
		filterData.word1 |= static_cast<int>(KE::Collision::Layers::MovingObject);
		
		myController->getActor()->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, true);

		{
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
				physx::PxQueryFlag::eDYNAMIC |
				//physx::PxQueryFlag::ePREFILTER  |
				physx::PxQueryFlag::ePOSTFILTER;

			myFilterData.mCCTFilterCallback = &myFilterCallback;
			myFilterData.mFilterCallback = &myFilterCallback;
		}

		//attack shape

		physx::PxSphereGeometry sphere(1.0f);
		myAttackShape = aPhysics->createShape(sphere, *myMaterial);

		physx::PxFilterData attackFilter;
		attackFilter.word0 = static_cast<int>(KE::Collision::Layers::PlayerAttack);
		attackFilter.word1 = static_cast<int>(KE::Collision::Layers::Player);
		attackFilter.word1 |= static_cast<int>(KE::Collision::Layers::Projectile);
		attackFilter.word1 |= static_cast<int>(KE::Collision::Layers::Trigger);

		myAttackShape->setFlag(physx::PxShapeFlag::eSCENE_QUERY_SHAPE, true);
		myAttackShape->setFlag(physx::PxShapeFlag::eSIMULATION_SHAPE, false);
		myAttackShape->setFlag(physx::PxShapeFlag::eTRIGGER_SHAPE, false);

		myAttackShape->setSimulationFilterData(attackFilter);
		myAttackShape->setQueryFilterData(attackFilter);
		myAttackShape->userData = &myAttackUserData;
		//

		return std::make_pair(myPhysicsObject.ID, &myPhysicsObject);
	}

	void PhysicsCharacterController::DashUpdate()
	{
		float timeToTarget = 0.15f;
		float force = 15.0f;
		Vector3f diff = myDashTarget - myDashOrigin;

		const float cappedTimer = std::min(myDashTimer, timeToTarget);

		float t = (cappedTimer / timeToTarget);
		float progress = 1.0f - std::powf(1.0f - t, force);
		Vector3f velocity = (diff * progress) / timeToTarget;

		//std::cout << velocity.Length() << std::endl;

		physx::PxVec3 displacement(velocity.x, 1.5f, velocity.z);

		if (myDashThroughWalls)
		{
			physx::PxExtendedVec3 dispPos = myController->getFootPosition();
			dispPos.x += displacement.x * KE_GLOBAL::deltaTime;
			dispPos.y = 0.25f;
			dispPos.z += displacement.z * KE_GLOBAL::deltaTime;


			const bool tp = myController->setFootPosition(
				dispPos
			);

			physx::PxQueryFilterData tpFilter;
			tpFilter.data.word0 = filterData.word0;
			tpFilter.data.word1 = static_cast<int>(KE::Collision::Layers::Wall);
			tpFilter.data.word1 |= static_cast<int>(KE::Collision::Layers::MovingObject);

			tpFilter.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::ePOSTFILTER;

			bool isPointInGeometry = false;
			physx::PxOverlapBuffer hit;
			physx::PxShape* shape;
			myController->getActor()->getShapes(&shape, 1, 0);
			bool overlap = myPhysicsObject.myActor->getScene()->overlap(
				shape->getGeometry(),
				myPhysicsObject.myActor->getGlobalPose(),
				hit,
				tpFilter,
				&myFilterCallback
			);

			myDashTimer += KE_GLOBAL::deltaTime;
			if (cappedTimer >= timeToTarget && !overlap)
			{
				myIsDashing = false;
				physx::PxExtendedVec3 finalPos = myController->getFootPosition();
				finalPos.y = 0.0f;	
				myController->setFootPosition(finalPos);

				myTransform->SetPosition({(float)finalPos.x, (float)finalPos.y, (float)finalPos.z});
			}

		}
		else
		{
			const physx::PxControllerCollisionFlags collisionFlags = myController->move(
				displacement * KE_GLOBAL::deltaTime,
				0.0f,
				KE_GLOBAL::deltaTime,
				myFilterData,
				NULL
			);

			physx::PxExtendedVec3 newPos = myController->getFootPosition();

			myController->setFootPosition({newPos.x, 0.0f, newPos.z});

			myTransform->SetPosition({ (float)newPos.x, (float)newPos.y - 0.5f, (float)newPos.z });

			if ((myTransform->GetPosition() - myDashTarget).Length() < 0.1f)
			{
				myIsDashing = false;
				myDashTimer = 0.0f;

				physx::PxExtendedVec3 finalPos = myController->getFootPosition();
				finalPos.y = 0.0f;	
				myController->setFootPosition(finalPos);
				myTransform->SetPosition({(float)finalPos.x, (float)finalPos.y, (float)finalPos.z});
			}

			myDashTimer += KE_GLOBAL::deltaTime;
			if (cappedTimer >= timeToTarget /*timeToTarget*/)
			{
				myIsDashing = false;

				physx::PxExtendedVec3 finalPos = myController->getFootPosition();
				finalPos.y = 0.0f;	
				myController->setFootPosition(finalPos);
				myTransform->SetPosition({(float)finalPos.x, (float)finalPos.y, (float)finalPos.z});
			}
		}


	}

	bool PhysicsCharacterController::CheckAgainstMovingObjects()
	{
		if (myIsDashing && myDashThroughWalls) { return false; }

		//check overlap with MovingObject
		physx::PxQueryFilterData tpFilter;
		tpFilter.data.word0 = filterData.word0;
		tpFilter.data.word1 = static_cast<int>(KE::Collision::Layers::MovingObject);
		//tpFilter.data.word1 |= static_cast<int>(KE::Collision::Layers::Ground);

		tpFilter.flags = physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::ePOSTFILTER;

		const int maxIter = 15;
		int iter = 0;

		while(iter < maxIter)
		{
			physx::PxOverlapBuffer hit;
			physx::PxShape* shape;

			auto globalPose = myPhysicsObject.myActor->getGlobalPose();
			myController->getActor()->getShapes(&shape, 1, 0);
			bool overlap = myPhysicsObject.myActor->getScene()->overlap(
				shape->getGeometry(),
				globalPose,
				hit,
				tpFilter,
				&myFilterCallback
			);

			if (!overlap)
			{
				break;
			} 

			auto UD = (PhysxShapeUserData*)hit.block.shape->userData;

			if (!UD || !UD->gameObject) { break; }

			if (P8::MovingObject* movingObj; UD->gameObject->TryGetComponent(movingObj))
			{
				Vector3f velocity = movingObj->GetVelocity();

				//if we are dashing, make sure we are not dashing into the moving object
				if (myIsDashing)
				{
					float dot = velocity.Dot(myDashTarget - myTransform->GetPosition());
					//if the dot product is negative, we are dashing into the moving object
					if (dot < 0.0f) { return false; }
				}

				//remove all player velocity that is in the opposite direction of the moving object
				myVelocity = myVelocity - (myVelocity.Dot(velocity) * velocity);


				if (!movingObj->IsMoving()) { break; }

				auto footPose = myController->getFootPosition();
				globalPose.p.x += velocity.x; 
				globalPose.p.y += velocity.y; 
				globalPose.p.z += velocity.z; 


				myPhysicsObject.myActor->setGlobalPose(globalPose);
				myTransform->TranslateWorld(velocity);
				myController->setFootPosition({
					footPose.x + velocity.x,
					footPose.y + velocity.y,
					footPose.z + velocity.z
				});

				//
				physx::PxQueryFilterData postCheckFilter;
				postCheckFilter.data.word0 = filterData.word0;
				postCheckFilter.data.word1 = static_cast<int>(KE::Collision::Layers::Wall);
				postCheckFilter.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::ePOSTFILTER;

				auto postCheckGlobalPose = myPhysicsObject.myActor->getGlobalPose();

				physx::PxOverlapBuffer postHit;
				bool postCheckOverlap = myPhysicsObject.myActor->getScene()->overlap(
					shape->getGeometry(),
					postCheckGlobalPose,
					postHit,
					postCheckFilter,
					&myFilterCallback
				);

				if (postCheckOverlap)
				{
					return true;
				}
			}
			else
			{
				break;
			}

			iter++;
		}
		if (iter >= maxIter)
		{
			return true;
		}

		return false;
	}

	void PhysicsCharacterController::Update()
	{
		myIsSquished = CheckAgainstMovingObjects();

		myCoyoteTimer = myIsGrounded ? 0.0f : myCoyoteTimer + (myIsDashing ? 0.0f : KE_GLOBAL::deltaTime);

		physx::PxVec3 displacement = myVelocity.LengthSqr() > 0.1f ?
			physx::PxVec3{ myVelocity.x, -5.0f, myVelocity.z } :
			physx::PxVec3{ 0.0f, -5.0f, 0.0f };

		displacement.y = myCoyoteTimer > myCoyoteTime || myIsGrounded ? displacement.y : 0.0f;

		const physx::PxControllerCollisionFlags collisionFlags = myController->move(
			displacement * KE_GLOBAL::deltaTime,
			0.0f,
			KE_GLOBAL::deltaTime,
			myFilterData,
			NULL
		);

		collisionFlags& physx::PxControllerCollisionFlag::eCOLLISION_DOWN ? myIsGrounded = true : myIsGrounded = false;

		physx::PxExtendedVec3 newPos = myController->getFootPosition();

		myTransform->SetPosition({ (float)newPos.x, (float)newPos.y, (float)newPos.z });

		if (myLinearDecayActive) {
			float decay = 1.0f - (myMovementData.decayMagnitude * KE_GLOBAL::deltaTime);
			myVelocity *= decay;
		}

		if (mAngularDecayActive) {
			myRotation *= 1.0f - (myMovementData.angularDecayMagnitude * KE_GLOBAL::deltaTime);
		}

		myLinearDecayActive = true;
		mAngularDecayActive = true;

		myOrientation += myRotation * KE_GLOBAL::deltaTime;
		myTransform->SetRotation(myOrientation);
		return;
	}

	void PhysicsCharacterController::Move(const Vector3f& aDirection, float aSpeedScale)
	{
		if (!myIsGrounded) return;

		myLinearDecayActive = false;
		myVelocity += (aDirection * myMovementData.linearAcceleration) * KE_GLOBAL::deltaTime;

		if (myVelocity.Length() > myMovementData.maxSpeed)
		{
			myVelocity.Normalize();
			myVelocity *= myMovementData.maxSpeed;
		}

		myVelocity *= aSpeedScale;
	}

	void PhysicsCharacterController::Dash(const Vector3f& aDirection, float aDistance)
	{
		myDashTimer = 0.0f;
		myIsDashing = true;


		Vector3f realDashTarget = myTransform->GetPosition() + (aDirection * aDistance);



		const float maxExtraDistance = 1.25f;

		Vector3f workingTarget = realDashTarget;

		//is realDashTarget not over ground?
		if (GetDistanceToGround(&workingTarget) < 0.0f)
		{
			//if not, move up to maxExtraDistance along aDirection
			for (float i = 0.0f; i < maxExtraDistance; i += 0.1f)
			{
				workingTarget = myTransform->GetPosition() + (aDirection * (aDistance + i));
				if (GetDistanceToGround(&workingTarget) >= 0.0f)
				{
					workingTarget = myTransform->GetPosition() + (aDirection * (aDistance + i + 0.2f));
					break;
				}
			}

		}

		myDashTarget = workingTarget;


		myDashOrigin = myTransform->GetPosition();
		float targetOrientation = atan2f(aDirection.x, aDirection.z);
		myOrientation = targetOrientation;
	}

	void PhysicsCharacterController::Rotate(const Vector3f& aDirection)
	{
		if (aDirection.GetNormalized().LengthSqr() < 0.1f) { return; }

		mAngularDecayActive = false;

		float targetOrientation = atan2f(aDirection.x, aDirection.z);
		float rotation = targetOrientation - myOrientation;

		rotation = MapToRange(rotation);
		float rotationSize = abs(rotation);

		if (rotationSize < targetRadius) {
			myRotation = 0.0f;
			return;
		}

		float speed = rotationSize > slowRadius ? myMovementData.angularAcceleration : myMovementData.maxRotation * (rotationSize / slowRadius);

		float finalRotation = rotation *= speed;
		float rotationDelta = finalRotation * KE_GLOBAL::deltaTime;

		if (abs(rotationDelta) > abs(rotation))
		{
			finalRotation = rotation / KE_GLOBAL::deltaTime;
		}

		myRotation = finalRotation;
	}

	void PhysicsCharacterController::ApplyVelocity(const Vector3f& aVelocity)
	{
		myVelocity = aVelocity;
	}

	/* When the character hits a shape (both static and dynamic), the onShapeHit callback is called. */
	void PhysicsCharacterController::onShapeHit(const physx::PxControllerShapeHit& aHit)
	{
		auto hitFilter = aHit.shape->getQueryFilterData();
		if (!(filterData.word1 & hitFilter.word0))
		{
			//return;
		}

		if (aHit.worldNormal.y < 0.2f)
		{
			myIsDashing = false;
		}
	}

	/* When the character hits another character controller the onControllerHit callback is called. */
	void PhysicsCharacterController::onControllerHit(const physx::PxControllersHit& aHit)
	{
		const auto& hitUserData = *static_cast<CharacterControllerUserData*>(aHit.other->getActor()->userData);

		PhysXCollisionData collisionData;
		collisionData.objHit = hitUserData.gameObject;

		GameObjectCollisionInterface::OnCollision(myUserData.gameObject, collisionData);

	}

	/* When the character hits a user-defined obstacle, the onObstacleHit callback is called. */
	void PhysicsCharacterController::onObstacleHit(const physx::PxControllerObstacleHit& aHit)
	{

	}

	void PhysicsCharacterController::SetCollisionWithLayer(Collision::Layers aLayer, bool aEnable)
	{
		if (aEnable)
		{
			filterData.word1 |= static_cast<int>(aLayer);
		}
		else
		{
			filterData.word1 &= ~static_cast<int>(aLayer);
		}

		physx::PxShape* shape;

		myController->getActor()->getShapes(&shape, 1, 0);
		shape->setQueryFilterData(filterData);
		shape->setSimulationFilterData(filterData);
	}

	Vector3f PhysicsCharacterController::GetFootPositioin()
	{
		Vector3f result =
		{
			(float)myController->getFootPosition().x,
			(float)myController->getFootPosition().y,
			(float)myController->getFootPosition().z
		};

		return result;
	}

	Vector3f PhysicsCharacterController::GetRegularPositioin()
	{
		Vector3f result =
		{
			(float)myController->getPosition().x,
			(float)myController->getPosition().y,
			(float)myController->getPosition().z
		};

		return result;
	}

	float PhysicsCharacterController::GetDistanceToGround(Vector3f* aPosition)
	{
		physx::PxQueryFilterData groundFilter;
		groundFilter.data.word0 = static_cast<int>(KE::Collision::Layers::Player);
		groundFilter.data.word1 = static_cast<int>(KE::Collision::Layers::Ground);

		const physx::PxU32 bufferSize = 256;        // [in] size of 'hitBuffer'
		physx::PxRaycastHit hitBuffer[bufferSize];  // [out] User provided buffer for results
		physx::PxRaycastBuffer buf(hitBuffer, bufferSize); // [out] Blocking and touching hits stored here




		auto footPos = myController->getFootPosition();
		if (aPosition)
		{
			footPos = {
				static_cast<float>(aPosition->x),
				static_cast<float>(aPosition->y),
				static_cast<float>(aPosition->z)
			};
		}


		physx::PxVec3 wFootPos = {
			static_cast<float>(footPos.x),
			static_cast<float>(footPos.y),
			static_cast<float>(footPos.z)
		};

		bool overlap = myPhysicsObject.myActor->getScene()->raycast(
			wFootPos,
			physx::PxVec3(0.0f, -1.0f, 0.0f),
			100.0f,
			buf,
			physx::PxHitFlag::eDEFAULT
		);

		if (overlap && buf.getNbTouches() > 0)
		{
			float dist = buf.getTouch(0).distance;
			for (physx::PxU32 i = 0; i < buf.getNbTouches(); i++)
			{
				if (buf.getTouch(i).distance < dist)
				{
					dist = buf.getTouch(i).distance;
				}
			}
			return dist;
		}

		return -1.0f;
	}

	void PhysicsCharacterController::AttackUpdate()
	{
		physx::PxQueryFilterData tpFilter;
		tpFilter.data.word0 = static_cast<int>(KE::Collision::Layers::Player) | static_cast<int>(KE::Collision::Layers::Projectile);
		tpFilter.data.word1 = static_cast<int>(KE::Collision::Layers::Player) | static_cast<int>(KE::Collision::Layers::Projectile);

		tpFilter.flags = physx::PxQueryFlag::eSTATIC | physx::PxQueryFlag::eDYNAMIC | physx::PxQueryFlag::eNO_BLOCK;

		const physx::PxU32 bufferSize = 256;        // [in] size of 'hitBuffer'
		physx::PxOverlapHit hitBuffer[bufferSize];  // [out] User provided buffer for results
		physx::PxOverlapBuffer buf(hitBuffer, bufferSize); // [out] Blocking and touching hits stored here
		bool overlap = myPhysicsObject.myActor->getScene()->overlap(
			myAttackShape->getGeometry(),
			myPhysicsObject.myActor->getGlobalPose(),
			buf,
			tpFilter
		);

		const float maximumHitAngle = 1.0f;
		auto& dbg = KE_GLOBAL::blackboard.Get<KE::Graphics>()->GetDebugRenderer();

			Matrix3x3f leftRot = Matrix3x3f::CreateRotationAroundY(-maximumHitAngle);
			Vector3f leftmostAngle = myTransform->GetForward() * leftRot;
			Matrix3x3f rightRot = Matrix3x3f::CreateRotationAroundY(maximumHitAngle);
			Vector3f rightmostAngle = myTransform->GetForward() * rightRot;

			Vector3f plrPos = myTransform->GetPosition();
			plrPos.y = 0.0f;
			Vector3f off = { 0.0f, 0.5f, 0.0f };
			dbg.RenderLine(
				plrPos + off,
				plrPos + off + leftmostAngle.GetNormalized() * 1.0f,
				{ 1.0f, 1.0f, 0.0f, 1.0f }
			);
			dbg.RenderLine(
				plrPos + off,
				plrPos + off + rightmostAngle.GetNormalized() * 1.0f,
				{ 1.0f, 1.0f, 0.0f, 1.0f }
			);
				dbg.RenderLine(
				plrPos + off,
				plrPos + off + myTransform->GetForward().GetNormalized() * 1.0f,
				{ 1.0f, 1.0f, 0.0f, 1.0f }
			);

		for (physx::PxU32 i = 0; i < buf.nbTouches; i++)
		{
			auto filter = buf.touches[i].shape->getQueryFilterData();
			auto& hit = buf.touches[i];
			if (!(
				filter.word0 & static_cast<int>(KE::Collision::Layers::Player) ||
				filter.word0 & static_cast<int>(KE::Collision::Layers::Projectile)
				))
			{
				continue;
			}

			auto hitUserData = *static_cast<CharacterControllerUserData*>(hit.actor->userData);
			if (hitUserData.gameObject->myID == myUserData.gameObject->myID) { continue; }
			if (myAttackUserData.myHitIDs.find(hitUserData.gameObject->myID) != myAttackUserData.myHitIDs.end()) { continue; }

			myAttackUserData.myHitIDs[hitUserData.gameObject->myID] = true;

			auto hitPositionPX = hit.actor->getGlobalPose().p;
			Vector3f hitPosition = { hitPositionPX.x, hitPositionPX.y, hitPositionPX.z };


			Vector3f hitDirection = (hitPosition - myTransform->GetPosition()).GetNormalized();
			float angle = acosf(hitDirection.Dot(myTransform->GetForward()));
			//std::cout << angle << std::endl;
			if (angle > maximumHitAngle) { continue; }



			if (hitUserData.objType == CharacterControllerUserData::Type::Player)
			{
				PhysXCollisionData collisionData;
				collisionData.objHit = hitUserData.gameObject;
				auto& playerComp = collisionData.objHit->GetComponent<P8::Player>();

				if (playerComp.GetTeam() == myUserData.team) { continue; }
				
				playerComp.TakeDamage(myUserData.gameObject->GetComponent<P8::Player>().GetIndex());
			}
			else if (hitUserData.objType == CharacterControllerUserData::Type::Boomerang)
			{
				PhysXCollisionData collisionData;
				collisionData.objHit = hitUserData.gameObject;

				Vector3f bounceNormal = myTransform->GetForward();

				if (hitUserData.team == myUserData.team) { continue; }

				hitUserData.gameObject->GetComponent<P8::BoomerangComponent>().Deflect(bounceNormal);

			}
		}
	}
}
