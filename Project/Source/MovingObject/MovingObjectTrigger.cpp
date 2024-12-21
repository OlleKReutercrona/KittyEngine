#include "stdafx.h"
#include "MovingObjectTrigger.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/GameObjectManager.h"
#include "Project/Source/MovingObject/MovingObject.h"
#include "Engine/Source/Collision/CollisionData.h"
#include "Engine/Source/ComponentSystem/Components/Collider/BoxColliderComponent.h"

namespace P8
{
	MovingObjectTrigger::MovingObjectTrigger(KE::GameObject& aGo) : KE::Component(aGo) {}
	MovingObjectTrigger::~MovingObjectTrigger()
	{
	}

	void MovingObjectTrigger::Awake()
	{
		auto& manager = myGameObject.GetManager();

		// Collect all the moving objects we are going to trigger //
		for (auto id : myTriggerObjectIDs)
		{
			if (KE::GameObject* go = manager.GetGameObject(id))
			{
				myTriggerObjects.push_back(&go->GetComponent<P8::MovingObject>());
			}
		}

		if (auto* boxCollider = &myGameObject.GetComponent<KE::BoxColliderComponent>())
		{
			
		}
	}

	void MovingObjectTrigger::Update()
	{

	}

	void MovingObjectTrigger::SetData(void* aData)
	{
		if (MovingObjectTriggerData* data = static_cast<MovingObjectTriggerData*>(aData))
		{
			myTriggerObjectIDs = data->movingObjects;
		}
	}

	void MovingObjectTrigger::OnPhysXCollision(const KE::PhysXCollisionData& aPhysXCollisionData)
	{
		KE::Collision::Layers layer = aPhysXCollisionData.objHit->myLayer;

		if (layer == KE::Collision::Layers::Projectile)
		{
			for (auto* movingObject : myTriggerObjects)
			{
				movingObject->Trigger();
			}
		}
	}

}