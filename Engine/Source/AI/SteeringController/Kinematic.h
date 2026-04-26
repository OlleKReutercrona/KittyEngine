#pragma once
#include "Engine/Source/Math/Transform.h"
#include "Engine/Source/Math/Vector2.h"
#include "Engine/Source/AI/SteeringController/SteeringOutput.h"
#include "SharedKinematic.h"

namespace KE_EDITOR
{
	class AIMovement;
}

class Kinematic
{
	friend class SteeringController;
	friend class Separation;
	friend class Seek;
	friend class Align;
	friend class Face;
	friend class Arrive;
	friend class KE_EDITOR::AIMovement;

public:
	Kinematic(Transform& aTransform) : transform(aTransform)
	{

	}

	inline void Update(SteeringOutput& aSteering, float aTimeDelta)
	{
		Vector3f& position = transform.GetPositionRef();

		// Apply Velocity and Rotation (X,Z only atm)
		position.x += velocity.x * aTimeDelta;
		position.z += velocity.z * aTimeDelta;
		orientation += rotation * aTimeDelta;

		transform.SetRotation(orientation);

		// Update Velocity and Rotation (X,Z only atm)
		velocity.x += aSteering.linear.x * aTimeDelta;
		velocity.z += aSteering.linear.z * aTimeDelta;
		rotation += aSteering.angular * aTimeDelta;

		// Limit to current MaxSpeed
		if (velocity.Length() > maxSpeed) // (NOTE) This was currentMaxSpeed before 
		{
			velocity.y = 0.0f; // To be safe

			velocity.Normalize();
			velocity *= maxSpeed;
		}

		// Decay used to slow down not needed?
		if (useDecay)
		{
			float decay = 1.0f - (10.0f * aTimeDelta);
			velocity *= decay;
		}
	}
	bool useDecay = false;

	Transform& transform;
	Vector3f velocity = {};

	float maxSpeed = 6.5f;
	//float currentMaxSpeed	= 25;
	float maxAcceleration = 55;
	float maxRotation = 7.5f;
	float maxAngularForce = 44;

	float rotation			= 0; 
	float orientation		= 0;
private:
	
};