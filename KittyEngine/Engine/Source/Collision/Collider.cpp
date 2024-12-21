#include "stdafx.h"
#include "Collider.h"
#include "PhysicsObject.h"

#include <External/Include/physx/PxActor.h>
#include <External/Include/physx/PxRigidDynamic.h>
#include <External/Include/physx/PxRigidStatic.h>
#include <External/Include/physx/PxRigidActor.h>

#include "ComponentSystem/GameObject.h"

void KE::Collider::UpdatePosition(const Vector3f& aPosition)
{
	myPosition = aPosition;

	SyncPhysXPosition();
}

void KE::Collider::UpdateOffset(const Vector3f& anOffset)
{
	myOffset = anOffset;

	SyncPhysXPosition();
}

void KE::Collider::UpdateSize(const Vector3f& aSize)
{
}

void KE::Collider::SetActive(const bool aValue)
{
	isActive = aValue;
}

void KE::Collider::AddForce(const Vector3f& theForce)
{
	if (myPhysicsObject->myKECollider.isStatic) return;

	physx::PxRigidDynamic* actor = myPhysicsObject->myActor->is<physx::PxRigidDynamic>();

	if (actor)
	{
		physx::PxVec3 theFakeForce;
		memcpy(&theFakeForce, &theForce, sizeof(theForce));
		actor->addForce(theFakeForce);
	}
}

void KE::Collider::SetKinematic(const bool aValue)
{
	isKinematic = aValue;
	DisableGravity(aValue);
	DisableSimulation(aValue);
}

void KE::Collider::DisableGravity(const bool aValue)
{
	myPhysicsObject->myActor->setActorFlag(physx::PxActorFlag::eDISABLE_GRAVITY, aValue);
}

void KE::Collider::DisableSimulation(const bool aValue)
{
	myPhysicsObject->myActor->setActorFlag(physx::PxActorFlag::eDISABLE_SIMULATION, aValue);
}

void KE::Collider::SyncPhysXPosition()
{
	auto worldTransform = myComponent->GetGameObject().myWorldSpaceTransform;
	worldTransform.TranslateLocal(myOffset);

	physx::PxTransform tm = worldTransform;
	myPhysicsObject->myActor->setGlobalPose(tm);

	physx::PxShape* shape;
	myPhysicsObject->myActor->getShapes(&shape, 1, 0);

	physx::PxGeometryHolder geom(shape->getGeometry());
	
	Vector3f scaleMod = worldTransform.GetScale();
	physx::PxVec3 scale = {
		myBaseSize.x * scaleMod.x,
		myBaseSize.y * scaleMod.y,
		myBaseSize.z * scaleMod.z
	};

	geom.box().halfExtents.x = scale.x / 2;
	geom.box().halfExtents.y = scale.y / 2;
	geom.box().halfExtents.z = scale.z / 2;

	shape->setGeometry(geom.box());

}

void KE::Collider::SetLayer(int aLayer)
{
	physx::PxShape* shape;
	myPhysicsObject->myActor->getShapes(&shape, 1, 0);

	auto queryFilter = shape->getQueryFilterData();
	auto simulationFilter = shape->getSimulationFilterData();

	queryFilter.word0 = aLayer;
	simulationFilter.word0 = aLayer;

	shape->setQueryFilterData(queryFilter);
	shape->setSimulationFilterData(simulationFilter);

}

void KE::Collider::SetPhysxUserData(void* aData)
{
	if (PhysxShapeUserData* userData = static_cast<PhysxShapeUserData*>(aData)) {

		myPhysxUserData = *userData;
		physx::PxShape* shape;
		myPhysicsObject->myActor->getShapes(&shape, 1, 0);
		shape->userData = &myPhysxUserData;
	}
}
