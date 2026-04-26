#include "stdafx.h"
#include "MovingObject.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/Components/Collider/BoxColliderComponent.h"
#include "Engine/Source/Collision/Collider.h"
#include "Engine/Source/Collision/PhysicsObject.h"
#include "Engine/Source/Collision/PhysicsCharacterController.h"
#include "Project/Source/Player/Player.h"

namespace P8
{
	P8::MovingObject::MovingObject(KE::GameObject& aGo) : KE::Component(aGo), myGoTransform(aGo.myTransform)
	{
		height = myGoTransform.GetPosition().y;
	}

	P8::MovingObject::~MovingObject()
	{

	}
	void MovingObject::Awake()
	{
		if (auto* boxCollider = &myGameObject.GetComponent<KE::BoxColliderComponent>())
		{
			myCollider = boxCollider->GetCollider();
			myCollider->SetKinematic(true);
			myCollider->DisableGravity(false);
			//myCollider->myPhysicsObject->disableColliderMovementPrecedence = false;
		}
	}
	void MovingObject::SetData(void* aData)
	{
		if (MovingObjectData* data = static_cast<MovingObjectData*>(aData))
		{
			myTimeToTarget = data->timeToTarget;
			myTargetTransform[0].SetPosition(myGameObject.myTransform.GetPosition());
			myTargetTransform[1].SetPosition(data->endPosition);
		}
	}
	void MovingObject::Update()
	{
		if (!myIsMoving) 
		{ 
			auto* rigidDynamic = static_cast<physx::PxRigidDynamic*>(myCollider->myPhysicsObject->myActor);
			rigidDynamic->setLinearVelocity({ 0.0f, 0.0f, 0.0f });

			myGameObject.myTransform.SetPosition(myTargetTransform[myTarget].GetPosition());
			myCollider->UpdatePosition(myTargetTransform[myTarget].GetPosition());

			//myCollider->SetLayer(static_cast<int>(KE::Collision::Layers::Wall));
			//myGameObject.myLayer = KE::Collision::Layers::Wall;

			//myCollider->SetKinematic(true);

			return; 
		}

		myTimer += KE_GLOBAL::deltaTime;
		float t = myTimer / myTimeToTarget;
		float progress = t * t * t;

		Vector3f position1 = myGameObject.myTransform.GetPosition();

		

		Vector3f newPosition = GetTargetPosition(progress/*, &myvelocity*/);
		newPosition.y = height;

		myVelocity = (newPosition - position1);

		myGameObject.myTransform.SetPosition(newPosition);
		myCollider->UpdatePosition(newPosition);

		if (progress > myThreshold) {
		
			myIsMoving = false;
			myGameObject.myTransform.SetPosition(myTargetTransform[!myTarget].GetPosition());
			myTarget = !myTarget;
			myCollider->UpdatePosition(myTargetTransform[!myTarget].GetPosition());

		}

	

	}
	void MovingObject::Trigger()
	{
		if (myIsMoving) { return; }

		myIsMoving = true;
		myTimer = 0.0f;

		myMovePlayerToTarget = myTargetTransform[!myTarget].GetPosition();
		//myCollider->SetLayer(static_cast<int>(KE::Collision::Layers::MovingObject));
		//myGameObject.myLayer = KE::Collision::Layers::MovingObject;
			//myCollider->SetKinematic(false);

	}

	const Vector3f MovingObject::GetTargetPosition(const float t, Vector3f* aOutVelocity)
	{
		const Vector3f& fromPos = myTargetTransform[myTarget].GetPositionRef();
		const Vector3f& toPos = myTargetTransform[!myTarget].GetPositionRef();

		if (aOutVelocity)
		{
			*aOutVelocity = (toPos - fromPos) / myTimeToTarget;
		}
		return fromPos + ((toPos - fromPos) * t);
	}
	void MovingObject::OnCollisionEnter(const KE::CollisionData& aCollisionData)
	{
		//if (auto* player = &aCollisionData.hitGameObject.GetComponent<P8::Player>())
		//{
		//	std::cout << "Collided with player\n";
		//
		//	Vector3f playerPos = player->GetGameObject().myTransform.GetPosition();
		//	Vector3f position = myGameObject.myTransform.GetPosition();
		//	Vector3f movingDir = (myMovePlayerToTarget - position).GetNormalized();
		//	// Check the dot product of the player's forward vector and the direction to the target
		//
		//	const Vector3f dirToPlayer = (playerPos - position).GetNormalized();
		//	float dot = movingDir.Dot(dirToPlayer);
		//
		//	//std::cout << "Dot: " << dot << std::endl;
		//	/*if (dot < 0.5f) { return; }*/
		//
		//	Vector3f direction = (myMovePlayerToTarget - position);
		//	//direction.Normalize();
		//
		//	position.y += 0.4f;
		//	//Vector3f& playerVelocity = player->GetVelocityRef();
		//	//
		//	//playerVelocity += myVelocity;
		//	playerPos.y = 0.0f;
		//
		//	player->SetPosition(playerPos + myVelocity * 2.0f);
		//}
	}
	void MovingObject::OnCollisionStay(const KE::CollisionData& aCollisionData)
	{
		 //<<<[THIS FUNCTION IS NEVER CALLED]>>> //
		 //<<<[THIS FUNCTION IS NEVER CALLED]>>> //
		 //<<<[THIS FUNCTION IS NEVER CALLED]>>> //

		//if (auto* player = &aCollisionData.hitGameObject.GetComponent<P8::Player>())
		//{
		//	std::cout << "Collided with player\n";
		//
		//	Vector3f playerPos = player->GetGameObject().myTransform.GetPosition();
		//	Vector3f position = myGameObject.myTransform.GetPosition();
		//	Vector3f movingDir = (myMovePlayerToTarget - position).GetNormalized();
		//	// Check the dot product of the player's forward vector and the direction to the target
		//
		//	const Vector3f dirToPlayer = (playerPos - position).GetNormalized();
		//	float dot = movingDir.Dot(dirToPlayer);
		//
		//	//std::cout << "Dot: " << dot << std::endl;
		//	/*if (dot < 0.5f) { return; }*/
		//
		//	Vector3f direction = (myMovePlayerToTarget - position);
		//	//direction.Normalize();
		//
		//	position.y += 0.4f;
		//	//Vector3f& playerVelocity = player->GetVelocityRef();
		//	//
		//	//playerVelocity += myVelocity;
		//	playerPos.y = 0.0f;
		//
		//	player->SetPosition(playerPos + myVelocity * 2.0f);
		//}
	}
	void MovingObject::OnCollisionExit(const KE::CollisionData& aCollisionData)
	{
		//if (auto* player = &aCollisionData.hitGameObject.GetComponent<P8::Player>())
		//{
		//	std::cout << "Collided with player\n";
		//
		//	Vector3f playerPos = player->GetGameObject().myTransform.GetPosition();
		//	Vector3f position = myGameObject.myTransform.GetPosition();
		//	Vector3f movingDir = (myMovePlayerToTarget - position).GetNormalized();
		//	// Check the dot product of the player's forward vector and the direction to the target
		//
		//	const Vector3f dirToPlayer = (playerPos - position).GetNormalized();
		//	float dot = movingDir.Dot(dirToPlayer);
		//
		//	//std::cout << "Dot: " << dot << std::endl;
		//	/*if (dot < 0.5f) { return; }*/
		//
		//	Vector3f direction = (myMovePlayerToTarget - position);
		//	//direction.Normalize();
		//
		//	position.y += 0.4f;
		//	//Vector3f& playerVelocity = player->GetVelocityRef();
		//	//
		//	//playerVelocity += myVelocity;
		//	playerPos.y = 0.0f;
		//
		//	player->SetPosition(playerPos + myVelocity * 2.0f);
		//}
	}
}