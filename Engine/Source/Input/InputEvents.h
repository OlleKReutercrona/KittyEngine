#pragma once
#include "Input.h"
#include "Engine/Source/Utility/Event.h"

namespace KE
{
	enum class eInputType;

	struct PlayerEvent : ES::Event
	{
		PlayerEvent() = default;
		~PlayerEvent() override = default;

		std::vector<Interaction> interactions;

		eInputType myInputType = eInputType::None;
		eInteractionType myInteractionType = eInteractionType::None;
		std::vector<Key> myKeyBindings = {};
		POINT myMousePosition = {0, 0};
		std::string myTriggerKeyName = "Not set";
	};

	struct GUIEvent : ES::Event
	{
		GUIEvent() = default;
		~GUIEvent() override = default;

		std::vector<Interaction> interactions;

		eInputType myInputType = eInputType::None;
		eInteractionType myInteractionType = eInteractionType::None;
		std::vector<Key> myKeyBindings = {};
		POINT myMousePosition = {0, 0};
		std::string myTriggerKeyName = "Not set";
	};
}
