#pragma once
#include "Kinematic.h"

#define PI 3.14159265359f

inline float MapToRange(float aRotation)
{
	while (aRotation <= PI) {
		aRotation += 2 * PI;
	}

	while (aRotation > PI) {
		aRotation -= 2 * PI;
	}

	return aRotation;
}

class Align
{
	friend class SteeringController;

public:
	Align(Kinematic& aCharacter) : character(aCharacter)
	{
		// [TODO] Set targetOrientation based on Gameobject's starting Transform
	}
	~Align() {}
	inline SteeringOutput GetSteering(float aTimeDelta)
	{
		SteeringOutput result;

		// # Get the naive direction to the target.
		float rotation = targetOrientation - character.orientation;

		// # Map the result to the(-pi, pi) interval.
		rotation = MapToRange(rotation);
		float rotationSize = abs(rotation);

		// # Check if we are there, return no steering.
		if (rotationSize < targetRadius) {
			character.rotation = 0.0f;
			return result;
		}

		// # If we are outside the slowRadius, then use maximum rotation.
		float targetRotation = rotationSize > slowRadius ? character.maxRotation : character.maxRotation * (rotationSize / slowRadius);

		// # The final target rotation combines speed (already in the variable) and direction.
		targetRotation *= rotation / rotationSize;

		// # Acceleration tries to get to the target rotation.
		result.angular = targetRotation - character.rotation;
		result.angular /= timeToTarget * aTimeDelta;

		// # Check if the acceleration is too great.
		float angularAcceleration = abs(result.angular);

		if (angularAcceleration > character.maxAngularForce)
		{
			result.angular /= angularAcceleration;
			result.angular *= character.maxAngularForce;
		}

		result.linear = { 0,0,0 };

		return result;
	}



protected:
	Kinematic& character;
	float targetOrientation = 0;

	// Moved to Kinematic
	//float maxAngularAcceleration = 25.0f; 
	//float maxRotation = 5.0f;

	float slowRadius = 0.8f;
	float targetRadius = 0.01745f; // equals 1 degree
	float timeToTarget = 0.1f;
};
#undef PI
