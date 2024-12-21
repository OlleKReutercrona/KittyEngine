#pragma once
#include "Engine/Source/Collision/Collider.h"
#include "Engine/Source/ComponentSystem/GameObject.h" // THIS SHOULD PROPABLY BE REMOVED /DR

namespace physx
{
	class PxRigidActor;
}

namespace KE
{
	struct PhysicsObject
	{
		PhysicsObject()
		{
			myKECollider.myPhysicsObject = this;
		}


		bool disableColliderMovementPrecedence = true;
		int ID = -1;
		Collider myKECollider;
		physx::PxRigidActor* myActor = nullptr;
	};

	class GameObjectCollisionInterface
	{
	public:
		inline static void OnCollision(GameObject* aGameObject, const PhysXCollisionData& aData)
		{
			aGameObject->OnPhysXCollision(aData);
		}
	};
}