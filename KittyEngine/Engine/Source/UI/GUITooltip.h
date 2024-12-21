#pragma once
#include "Engine/Source/UI/GUIBox.h"
//#include "Engine/Source/Graphics/Graphics.h"
#include "Engine/Source/Graphics/Sprite/Sprite.h"

namespace KE
{
	class Graphics;
	struct Sprite;

	struct GUITooltip
	{
		bool isInitialized = false;
		GUIBox myBox;
		bool isActive = false;
		SpriteBatch mySpriteBatch;
		Sprite* mySprite = nullptr;
		//Temp
		Vector2f myLinePoints[4];
		Vector4f myColour;

		void DrawTooltip(Graphics* aGraphics) const;
		void Show();
		void Hide();
	};
}
