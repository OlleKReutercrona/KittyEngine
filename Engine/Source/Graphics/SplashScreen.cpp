#include "stdafx.h"
#include "SplashScreen.h"

namespace KE
{
	
	void SplashScreen::Init(KE::Graphics* aGraphics)
	{
		myTimer.Reset();
		
		KE::TextureLoader& textureLoader = aGraphics->GetTextureLoader();
		myTGALogoScreen   = textureLoader.GetTextureFromPath("Data/InternalAssets/tgaLogo_top.dds");
		myGroupLogoScreen = textureLoader.GetTextureFromPath("Data/InternalAssets/groupLogo_top.dds");
		myEngineScreen = textureLoader.GetTextureFromPath("Data/InternalAssets/engineLogo_top.dds");
	}
	
	bool SplashScreen::Update()
	{
		myTimer.UpdateDeltaTime();
		return myTimer.GetTotalTime() < mySplashScreenElementDuration * mySplashScreenElementCount + 1.0f;
	}
	
	void SplashScreen::Render(KE::Graphics* aGraphics)
	{
		if (myTimer.GetTotalTime() < mySplashScreenElementDuration)
		{
			aGraphics->RenderFullscreen(myTGALogoScreen);
			return;
		}
		else if (myTimer.GetTotalTime() < mySplashScreenElementDuration * 2)
		{
			aGraphics->RenderFullscreen(myGroupLogoScreen);
			return;
		}
		else if (myTimer.GetTotalTime() < mySplashScreenElementDuration * 3 + 1.0f)
		{
			aGraphics->RenderFullscreen(myEngineScreen);
			return;
		}
	}

}