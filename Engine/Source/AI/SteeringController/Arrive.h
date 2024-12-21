#pragma once
#include "Kinematic.h"

class Arrive
{
	friend class SteeringController;

public:
	Arrive(Kinematic& aCharacter) : character(aCharacter)
	{

	}
	~Arrive() {}
	inline SteeringOutput GetSteering()
	{
		SteeringOutput result;

		Vector3f characterPos = character.transform.GetPosition();
		target.y = 0.0f;
		characterPos.y = 0.0f;

		// # Get the direction to the target.
		Vector3f direction = target - characterPos;
		//float distance = direction.Length();
		float distance = (characterPos - target).Length();

		//// # Check if we are there, return no steering.
		if (distance < targetRadius)
		{
			//character.velocity = { 0,0,0 };
			return result;
		}

		// # If we are outside the slowRadius, then move at max speed. Otherwise calculate a scaled speed.
		//float magnitude = slowActive ? character.maxSpeed * (distance / slowRadius) : character.maxSpeed;
		//float targetSpeed = distance > slowRadius ? character.maxSpeed * 2.0f : magnitude;

		float targetSpeed = character.maxAcceleration;

		// # The target velocity combines speed and direction
		Vector3f targetVelocity = direction.GetNormalized();
		targetVelocity *= targetSpeed;

		result.linear = targetVelocity /*- character.velocity*/;
		result.linear /= timeToTarget;

		if (result.linear.Length() > character.maxAcceleration) {
			result.linear.Normalize();
			result.linear *= character.maxAcceleration;
		}

		result.angular = 0;

		return result;
	}

	inline void SetSlowActive(bool aState) { slowActive = true; aState; }

	// Used to slow down before arriving at the final target location.
	inline void SetFinalTarget(Vector2f& aFinalTarget) 
	{ 
		finalTarget = { aFinalTarget.x, 0.0f, aFinalTarget.y };
	}

	// Used to slow down before arriving at the final target location.
	inline void SetTarget(Vector3f& aTarget)
	{
		target = aTarget;
	}

private:
	Kinematic& character;
	Vector3f target;
	Vector3f finalTarget;
	float targetRadius = 0.3f;
	float slowRadius = 0.5f;
	float timeToTarget = 0.15f;
	bool slowActive = false;
};