#pragma once

class Separation
{
public:
	Separation(Kinematic& aCharacter) : character(aCharacter)
	{

	}
	~Separation() {}

	// [TODO] Needs to get hands on units to separate from.
	SteeringOutput GetSteering(/* WorldInterface? */)
	{
		SteeringOutput result;

		return result;
	}

protected:
	Kinematic& character;
};