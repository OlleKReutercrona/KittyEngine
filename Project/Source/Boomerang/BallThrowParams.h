#pragma once

class Transform;

namespace P8
{
	class Player;

	enum class BoomerangState
	{
		Die,
		Flying,
		Recalled,
		Telekinesis,
		PickedUp,
		PickupAnim
	};

	class BoomerangComponent;

	struct BallThrowParams
	{
		BoomerangState oldState = BoomerangState::PickedUp;
		BoomerangState state = BoomerangState::PickedUp;
		float returnDelay = 0.2f;
		float returnMultiplierRadius = 3.0f;
		float flightMaxDistance = 8.0f;
		float pickupRadius = 1.25f;
		float damageRadius = 0.25f;

		float maxSpeed = 33.0f;
		float slowRadius = 2.0f;
		float decayStrength = 5.0f;

		float dyingThreshold = 35.0f;

		float minBounceValue = 5.0f;
		float maxBounceValue = 35.0f;

		unsigned int team = 999;

		float flightTime = 0.0f;

		float size = 0.25f;
		float sizeMult = 1.0f;

		bool isCopy = false;
		bool hasSplit = false;
		bool hasExploded = false;

		BoomerangComponent* splitFrom = nullptr;


		Transform* throwerTransform;
		Player* throwerPlayer;
	};
}