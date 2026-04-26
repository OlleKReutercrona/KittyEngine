#pragma once
#include <Engine/Source/Windows/Window.h>
#include <Engine/Source/Utility/Timer.h>
#include <Engine/Source/SceneManagement/SceneManager.h>
#include <Engine/Source/Graphics/Camera.h>
#include <Engine/Source/Graphics/SplashScreen.h>

#include <Engine/Source/Graphics/GraphicsConstants.h>

#include "Engine/Source/Input/UserInput.h"
#include "Engine/Source/Script/ScriptManager.h"


//editor stuff:
#ifndef KITTYENGINE_NO_EDITOR
#include "Editor/Source/Editor.h"
#endif
//

namespace KE
{
	class Game
	{
		KE_EDITOR_FRIEND
	public:
		Game();
		~Game();
		int Go();

	private:
		void Update();
		void HandleInput(const float aDeltaTime);

	private:
		Window myWindow;
		Timer myTimer;
		SceneManager mySceneManager;
		ScriptManager myScriptManager;
		UserInput myUserInput;

		// These shouldn't be here --Olle
		SplashScreen mySplashScreen;

		IF_EDITOR(KE_EDITOR::Editor myEditor;)

		float myCameraSpeed = 3.0f;
		float myCameraSpeedDefault = 3.0f;
		float myCameraSpeedBoost = 50.0f;

		//PhysxTest myPhysxTest;
	};
}
