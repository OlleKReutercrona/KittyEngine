#pragma once
#include "Engine\Source\ComponentSystem\Components\Component.h"
#include "Engine/Source/Utility/EventSystem.h"

namespace P8
{
	struct TimeEventSettings
	{
		int resetCurvePower = 2;
		float resetTime = 1.0f;
		float timeDelayUntilReset = 0.75f;
		float minModifierValue = 0.01f;
		float maxModifierValue = 1.0f;
		float timeModifier = 0.05f;
	};

	class TimeManager : public KE::Component, public ES::IObserver
	{
	public:
		TimeManager(KE::GameObject& aGameObject);
		~TimeManager();

		void EarlyUpdate() override;

		// Inherited via IObserver
		void OnReceiveEvent(ES::Event& aEvent) override;
	private:
		void OnInit() override;
		void OnDestroy() override;

		void SetTimeModifier(const float aValue);
	private:
		float myTimeModifier = 1.0f;
		float myStartModifier = 0.1f;

		float myTimeSinceEvent = 0.0f;

		// constants
		TimeEventSettings mySettings;
	};
}