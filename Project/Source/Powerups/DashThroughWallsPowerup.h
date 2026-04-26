#pragma once
#include "Powerup.h"

namespace P8
{
	
	class DashThroughWallsPowerup : public Powerup
	{
	private:
		bool lastFrameDashing = false;
	public:
		void Update(const PowerupInputData& aInputData) override;

		void OnPickup() override;
		void OnDrop() override;

		void OnEnable() override;
		void OnDisable() override;

		bool OnPlayerAction(PlayerActionType aAction, ActionState aState, PowerupInputData& input) override;
		bool OnBoomerangAction(BoomerangAction aAction, ActionState aState, PowerupInputData& input) override;
	};
}
