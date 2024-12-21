#include "stdafx.h"
#include "TimeManager.h"
#include "Engine/Source/Math/KittyMath.h"
#include "Project/Source/GameEvents/GameEvents.h"

P8::TimeManager::TimeManager(KE::GameObject& aGameObject) : KE::Component(aGameObject)
{
	OnInit();
}

P8::TimeManager::~TimeManager()
{
	OnDestroy();
}

void P8::TimeManager::EarlyUpdate()
{
	if (myTimeModifier < 1.0f)
	{
		myTimeSinceEvent += KE_GLOBAL::deltaTime;

		if (myTimeSinceEvent >= mySettings.timeDelayUntilReset)
		{
			float diff = myTimeSinceEvent - mySettings.timeDelayUntilReset;

			float percentage = std::clamp(
				std::powf(diff, (float)mySettings.resetCurvePower),
				0.0f,
				1.0f);

			myTimeModifier = std::lerp(mySettings.timeModifier, mySettings.maxModifierValue, percentage);

			if (myTimeModifier >= mySettings.resetTime)
			{
				SetTimeModifier(1.0f);

				myTimeSinceEvent = 0.0f;
			}
		}
	}
	KE_GLOBAL::deltaTime *= myTimeModifier;
}

void P8::TimeManager::OnReceiveEvent(ES::Event& aEvent)
{
	// Recieve events that may change time
	
	if (P8::SlowMotionEvent* slowMotionMSG = dynamic_cast<P8::SlowMotionEvent*>(&aEvent))
	{
		mySettings = *(P8::TimeEventSettings*)&slowMotionMSG->data;

		myStartModifier = mySettings.timeModifier;

		myTimeSinceEvent = 0.0f;
		SetTimeModifier(myStartModifier);
		//mySettings.startTimeModifier = myTimeModifier;
	}
}

void P8::TimeManager::OnInit()
{
	ES::EventSystem::GetInstance().Attach<P8::SlowMotionEvent>(this);
}

void P8::TimeManager::OnDestroy()
{
	ES::EventSystem::GetInstance().Detach<P8::SlowMotionEvent>(this);
}

void P8::TimeManager::SetTimeModifier(const float aValue) 
{
	myTimeModifier = std::clamp(aValue, mySettings.minModifierValue, mySettings.maxModifierValue);
}
