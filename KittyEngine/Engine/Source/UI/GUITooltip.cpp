#include "stdafx.h"
#include "GUITooltip.h"

#include "Engine/Source/Graphics/Graphics.h"

namespace KE
{
	void GUITooltip::DrawTooltip(Graphics* aGraphics) const
	{
		aGraphics->AddScreenSpaceLine(myLinePoints[0], myLinePoints[1], myColour);
		aGraphics->AddScreenSpaceLine(myLinePoints[1], myLinePoints[2], myColour);
		aGraphics->AddScreenSpaceLine(myLinePoints[2], myLinePoints[3], myColour);
		aGraphics->AddScreenSpaceLine(myLinePoints[3], myLinePoints[0], myColour);
	}

	void GUITooltip::Show()
	{
		if (isActive)
		{
			return;
		}
		isActive = true;
	}

	void GUITooltip::Hide()
	{
		isActive = false;
	}
}
