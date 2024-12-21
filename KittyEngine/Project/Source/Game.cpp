#include "stdafx.h"

#include "Game.h"
#include <Engine/Source/Graphics/ModelLoader.h>
#include <Engine/Source/Graphics/ShaderLoader.h>
#include <Engine/Source/Graphics/Texture/TextureLoader.h>

#include <Engine/Source/Graphics/CameraManager.h>
#include <Engine/Source/Math/KittyMath.h>
#include <Engine/Source/Math/Transform.h>

#include <Engine/Source/ComponentSystem/GameObjectManager.h>
#include <Engine/Source/ComponentSystem/GameObject.h>
#include <Engine/Source/ComponentSystem/Components/LightComponent.h>

#include <optional>
#include <string>
#include <iostream>

#include <Engine/Source/Utility/Global.h>
#include <Engine/Source/Graphics/SplashScreen.h>
#include <Engine/Source/Utility/DebugTimeLogger.h>

#include <Project/Source/GameRules/GameRules.h>

#include "Engine/Source/Audio/GlobalAudio.h"


#ifndef KITTYENGINE_SHIP
#define USE_GAME_NAME 0
#else
#define USE_GAME_NAME 1
#endif

namespace KE
{
	constexpr int WINDOW_WIDTH = 1600;
	constexpr int WINDOW_HEIGHT = 900;
	constexpr float FOV = 90.0f;

	Game::Game()
		:
		myWindow(WINDOW_WIDTH, WINDOW_HEIGHT, (USE_GAME_NAME) ? GAME_NAME : L"Kitty Engine: " _KITTYENGINE_BUILD)
	{
		Camera* debugCamera = myWindow.GetGraphics().GetCameraManager().GetCamera(KE_DEBUG_CAMERA_INDEX);
		debugCamera->SetPerspective(WINDOW_WIDTH, WINDOW_HEIGHT, FOV * KE::DegToRadImmediate, 0.01f, 1000.0f);

		//myPhysxTest.Init();
	}

	Game::~Game() { }

	int Game::Go()
	{
		myUserInput.Init();
		myScriptManager.Init();
		GlobalAudio::Init();
		P8::GameRules::CreateInstance();
		P8::GameRules::GetInstance()->Init();

		Camera* mainCamera = myWindow.GetGraphics().GetCameraManager().GetCamera(KE_MAIN_CAMERA_INDEX);
		mainCamera->SetPerspective(WINDOW_WIDTH, WINDOW_HEIGHT, FOV * KE::DegToRadImmediate, 0.01f, 5000.0f);
		mySceneManager.Init(myWindow);


		IF_EDITOR(
			ModelLoader & modelLoader = myWindow.GetGraphics().GetModelLoader();
			ShaderLoader & shaderFactory = myWindow.GetGraphics().GetShaderLoader();
			TextureLoader & textureLoader = myWindow.GetGraphics().GetTextureLoader();
			myEditor.Init(&myWindow, &myTimer, &shaderFactory, &textureLoader, &modelLoader, &mySceneManager);
			)


#ifdef KITTYENGINE_SHIP
			ShowCursor(false);
#endif	

		mySplashScreen.Init(&myWindow.GetGraphics());

		std::vector<std::string> vfxNames;

		for (const auto& path : std::filesystem::directory_iterator("Data/InternalAssets/VFXSequences"))
		{
			std::string filename = path.path().filename().string();
			//remove extension
			filename = filename.substr(0, filename.find_last_of('.'));
			vfxNames.push_back(filename);
		}

		while (true)
		{
			
			if (vfxNames.size() > 0)
			{
				myWindow.GetGraphics().GetVFXManager().GetVFXSequenceFromName(vfxNames.back());
				vfxNames.pop_back();
			}

			KE::DebugTimeLogger::BeginLogVar("Frame");
			// Process all messages pending
			if (const auto code = Window::ProcessMessages())
			{
				// If return optional has value, we're quitting
				return *code;
			}
			// If no value
			float dt = myTimer.UpdateDeltaTime();


			constexpr float DT_MAX = 0.1f;
			float clampedDT = dt > DT_MAX ? DT_MAX : dt;
			KE_GLOBAL::deltaTime = clampedDT;
			KE_GLOBAL::trueDeltaTime = clampedDT;
			KE_GLOBAL::totalTime += clampedDT;
			Update();
			HandleInput(clampedDT);
			KE::DebugTimeLogger::EndLogVar("Frame");
		}


	}

	void Game::Update()
	{
		// This is where all Queued Events are handled in the Event System
		ES::EventSystem::GetInstance().HandleQueuedEvents();

		KE::DebugTimeLogger::BeginLogVar("Begin Frame");
		// Main Update loop
		myWindow.GetGraphics().BeginFrame();

		IF_EDITOR(myEditor.BeginFrame());

		KE::DebugTimeLogger::EndLogVar("Begin Frame");

		if (mySplashScreen.Update())
		{
			mySplashScreen.Render(&myWindow.GetGraphics());
		}
		else
		{
			KE::DebugTimeLogger::BeginLogVar("Update Loop");

			mySceneManager.Update();
			GlobalAudio::Update();

			//myPhysxTest.Update();

			KE::DebugTimeLogger::EndLogVar("Update Loop");
		//
		//// TODO This should probably not be here MVH Anton
		//KE::DebugTimeLogger::BeginLogVar("GUI");
		//myGUIHandler.UpdateGUI();
		//myGUIHandler.RenderGUI(&myWindow.GetGraphics());
		//myGUIHandler.DrawGUIGrid(&myWindow.GetGraphics(), myWindow.GetWindowSize());
		//KE::DebugTimeLogger::EndLogVar("GUI");
		}

		//KE::DebugTimeLogger::BeginLogVar("AudioPlayer Update");
		//IF_EDITOR(KE_GLOBAL::audioPlayer.Update());
		//KE::DebugTimeLogger::EndLogVar("AudioPlayer Update");

		KE::DebugTimeLogger::BeginLogVar("Editor Update");
		IF_EDITOR(myEditor.Update());
		KE::DebugTimeLogger::EndLogVar("Editor Update");


		KE::DebugTimeLogger::BeginLogVar("Render");
		myWindow.GetGraphics().Render();
		KE::DebugTimeLogger::EndLogVar("Render");

		KE::DebugTimeLogger::BeginLogVar("End Frame");
		IF_EDITOR(myEditor.EndFrame());

		myWindow.GetGraphics().EndFrame();
		KE::DebugTimeLogger::EndLogVar("End Frame");
	}

	void Game::HandleInput(const float aDeltaTime)
	{
		// Update key states
		myWindow.GetInputWrapper().Update();

		// Determine if mouse is over GUI or over game
		// TODO Ugly fix for now, should be handled in a better way
		// TODO Don't want to fetch the GUIHandler every frame
		GUIHandler* guiHandler = KE_GLOBAL::blackboard.Get<GUIHandler>("GUIHandler");
		myUserInput.SetIsMouseOnGUI(guiHandler->IsMouseOverGUI(myWindow.GetInputWrapper().GetMousePosition()));

		UNREFERENCED_PARAMETER(aDeltaTime);

		if (myWindow.GetInputWrapper().IsKeyDown(VK_SHIFT) && myWindow.GetInputWrapper().IsKeyDown(VK_ESCAPE))
		{
			PostQuitMessage(0);
		}
	}
}
