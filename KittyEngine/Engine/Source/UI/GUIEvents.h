#pragma once
#include "Utility/Event.h"

namespace KE
{
	struct GUIResolutionEvent : ES::Event
	{
		GUIResolutionEvent() = default;
		~GUIResolutionEvent() override = default;

		int myWidth = 0;
		int myHeight = 0;
		bool myFullscreen = false;
	};
}
