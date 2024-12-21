#pragma once
#include "Kinematic.h"

class Seek 
{
	friend class SteeringController;

public:
	Seek(Kinematic& aCharacter) : character(aCharacter)
	{

	}
	~Seek(){}

	inline SteeringOutput GetSteering()
	{
		SteeringOutput result;

		result.linear.x = seekTarget.x - character.transform.GetPosition().x;
		result.linear.z = seekTarget.z - character.transform.GetPosition().z;

		result.linear.Normalize();

		result.linear *= character.maxAcceleration;
		result.angular = 0;

		return result;
	}

protected:
	Kinematic& character;
	Vector3f seekTarget = {};
};