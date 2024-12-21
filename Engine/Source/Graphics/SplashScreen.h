#pragma once
#include <Engine/Source/Graphics/Graphics.h>
#include <Engine/Source/Graphics/Texture/Texture.h>

#include <Engine/Source/Utility/Timer.h>

namespace KE
{
	class SplashScreen
	{
	private:
		Texture* myTGALogoScreen;
		Texture* myGroupLogoScreen;
		Texture* myEngineScreen;
		Timer myTimer;
#ifndef KITTYENGINE_NO_EDITOR
		//const float mySplashScreenElementDuration = 1.75f;
		const float mySplashScreenElementDuration = 0.0f;
#else
		const float mySplashScreenElementDuration = 1.75f;
#endif
		const int mySplashScreenElementCount = 3;
		

	public:
		void Init(KE::Graphics* aGraphics);
		bool Update();
		void Render(KE::Graphics* aGraphics);
		
	};
}