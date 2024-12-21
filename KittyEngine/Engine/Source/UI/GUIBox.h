#pragma once

namespace KE
{
	struct GUIBox
	{
		Vector2f myOffset;
		Vector2f myScreenPosition;
		float myWidth;
		float myHeight;
		Vector2f myOffsetResolutionFactor;
		Vector2f mySizeResolutionFactor;
		Vector2i myResolution;

		bool IsInside(const Vector2f aMousePosition) const
		{
			if (aMousePosition.x > myScreenPosition.x && aMousePosition.x < myScreenPosition.x + myWidth)
			{
				if (aMousePosition.y < myScreenPosition.y && aMousePosition.y > myScreenPosition.y - myHeight)
				{
					return true;
				}
			}
			return false;
		}
	};
}
