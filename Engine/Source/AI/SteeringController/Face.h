#pragma once
#include "Kinematic.h"
#include "Align.h"

class Face : public Align
{
	friend class SteeringController;

public:
	Face(Kinematic& aCharacter) : Align(aCharacter){}
	~Face() {}

	inline void SetTarget(const Vector3f aTarget) { faceTarget = aTarget; }

	inline SteeringOutput GetSteering(float aTimeDelta)
	{
		SteeringOutput result;

		Vector3f direction = (faceTarget - character.transform.GetPosition());
		direction.y = 0.0f;
		direction.Normalize();

		if (direction.Length() == 0) {
			return result;
		}
		
		Align::targetOrientation = atan2f(direction.x, direction.z);

		return Align::GetSteering(aTimeDelta);
	}

protected:
	Vector3f faceTarget;
};