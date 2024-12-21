#pragma once
#include "Powerup.h"

namespace P8
{
	class JuggernautPowerup : public Powerup
	{
	private:
	public:
		void Update(const PowerupInputData& aInputData) override;

		void OnPickup() override;
		void OnDrop() override;

		void OnEnable() override;
		void OnDisable() override;

		void SetVFXActive(bool aActive, int idx, bool loop = true);

		bool OnPlayerAction(PlayerActionType aAction, ActionState aState, PowerupInputData& input) override;
	};
}