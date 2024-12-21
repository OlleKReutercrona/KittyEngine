#include "stdafx.h"
#include "GUIHandler.h"

#include <format>
#include <ranges>

#include "GUIEvents.h"
#include "Engine/Source/UI/GUIElement.h"
#include "Engine/Source/UI/GUIScene.h"
#include "Engine/Source/Graphics/Graphics.h"
#include "Engine/Source/Windows/Window.h"
#include "Engine/Source/Utility/Global.h"
#include "Engine/Source/Input/Input.h"
#include "Engine/Source/Input/InputEvents.h"
#include "Engine/Source/Files/GUIFile.h"
#include "Project/Source/GameEvents/GameEvents.h"
#include "Engine/Source/Audio/GlobalAudio.h"
#include <External/Include/imgui/imgui.h>

#pragma message ("							  The Story of the GUI Cat")
#pragma message ("							  ------------------------")
#pragma message (" _._     _,-'\"\"`-._		 Once upon a time there was")
#pragma message ("(,-.`._,'(       |\\`-/|	 a GUI cat. It was a stran-")
#pragma message ("    `-.-' \\ )-`( , o o)	 ""ge cat that did not like")
#pragma message ("          `-    \\`_`\"'-	 to be petted. It was a ve-")
#pragma message ("							 ry angry cat. It had a be-")
#pragma message ("							 ef with CUI cats. The End.")
#pragma message ("							 ")
#pragma message ("Time for beautiful GUI :33")

namespace KE
{
	GUIHandler::GUIHandler()
	{
		isGUIEnabled = true;
		drawGUIGrid = false;
		myGridSizeX = 24;
		myGridSizeY = 14;
		gridSquareCells = true;

		myCurrentResolutionIndex = 1;
		myMaxResolutions = 3;
		isFullscreen = true;

		myMaxVolume = 10;
		myMinVolume = 0;
		myVolumeIncrement = 1;

		isCountdownActive = false;
		shouldCountDown = false;
		myCountdownTimer = 0.0f;
		myCountdownMaxTime = 3.2f;

		myBlockInputTimer = 0.0f;
		myBlockInputTime = 1.0f;

		myScoreAnimationTime = 0.2f;

		myMaxScore = 10;

		myCharacterColours[0] = { 0.0f, 0.825f, 0.894f, 1.0f }; // Blue 29dded
		myCharacterColours[1] = { 0.397f, 0.724f, 0.318f, 1.0f }; // Green 4b8852
		myCharacterColours[2] = { 0.94f, 0.36f, 0.58f, 1.0f }; // Pink f05b94
		myCharacterColours[3] = { 0.603f, 0.312f, 0.854f, 1.0f }; // Purple 6c46ab

		myPortraitPaths[4] = { "Data/Assets/UI/05 Score board/UI_scorboard_God.dds" };

		myCharacterPointPaths[0] = "Data/Assets/UI/05 Score board/UI_scorboard_dot_yellow.dds";
		myCharacterPointPaths[1] = "Data/Assets/UI/05 Score board/UI_scorboard_dot_green.dds";
		myCharacterPointPaths[2] = "Data/Assets/UI/05 Score board/UI_scorboard_dot_pink.dds";
		myCharacterPointPaths[3] = "Data/Assets/UI/05 Score board/UI_scorboard_dot_purple.dds";
		myCharacterPointPaths[4] = "Data/Assets/UI/05 Score board/UI_scorboard_dot_tga.dds";
		myCharacterDefaultPointPath = "Data/Assets/UI/05 Score board/UI_scorboard_whiteDot.dds";
		myCharacterScoreboardPaths[0] = "Data/Assets/UI/05 Score board/UI_scorboard_containerYellow.dds";
		myCharacterScoreboardPaths[1] = "Data/Assets/UI/05 Score board/UI_scorboard_containerGreen.dds";
		myCharacterScoreboardPaths[2] = "Data/Assets/UI/05 Score board/UI_scorboard_containerPink.dds";
		myCharacterScoreboardPaths[3] = "Data/Assets/UI/05 Score board/UI_scorboard_containerPurple.dds";
		myCharacterScoreboardPaths[4] = "Data/Assets/UI/05 Score board/UI_scorboard_containerGod.dds";
	}

	GUIHandler::~GUIHandler()
	{
		GUIHandler::OnDestroy();
	}

	void GUIHandler::Init(Graphics* aGraphics)
	{
		myGraphics = aGraphics;

		KE_GLOBAL::blackboard.Register("GUIHandler", this);

		char fontPath[64] = "motley";
		myFont = FontLoader::LoadFont(std::format("Data/InternalAssets/{}.ktf", fontPath).c_str());

		LoadGUIFromFile();

		OnInit();

		myInactiveGUIScenes.reserve(10);
	}

	void GUIHandler::LoadGUIFromFile()
	{
		KE::GUIFile guiFile;
		guiFile.Load("Data/GUI/GUI.prrmao");

		const Vector2i resolution = myGraphics->GetRenderSize();

		myEventMap.clear();

		for (auto& scene : guiFile.myScenesData)
		{
			if (myGUISceneMap.contains(scene.myName))
			{
				PopGUIScene(scene.myName);
				RemoveGUIScene(scene.myName);
			}

			CreateGUIScene(scene.myName);
			GUIScene* guiScene = GetGUIScene(scene.myName);

			for (auto& element : scene.myElements)
			{
				GUIElement& newElement = guiScene->CreateGUIElement(
					element.myName,
					element.myTexturePath,
					element.myEventName,
					(KE::eAlignType)element.myAlignType,
					(KE::eProgressionDirection)element.myProgressionDirection,
					element.myOffsetResolutionFactor,
					element.mySizeResolutionFactor,
					{ myGraphics->GetRenderSize() },
					element.isButton
				);

				if (element.myEventName != "")
				{
					myEventMap[element.myEventName].push_back(&newElement);
				}
				newElement.shouldHideAtStart = element.hideAtStart;
				newElement.shouldDraw = !element.hideAtStart;

				AssignSpriteToGUIElement(
					newElement,
					myGraphics,
					{
						newElement.myBox.myWidth,
						newElement.myBox.myHeight
					},
					element.myTexturePath);

				if (element.mySecondaryTexturePath != "")
				{
					newElement.mySecondaryTexture = myGraphics->GetTextureLoader().GetTextureFromPath(element.mySecondaryTexturePath);
				}

				if (element.hasText)
				{
					//newElement.UpdateSpriteScaleAndPosition({ (float)resolution.x, (float)resolution.y }, false);

					newElement.hasText = true;
					newElement.myTextColour = element.myTextColour;
					SetGUIText(&newElement, element.myText);
				}
			}

			guiScene->UpdateGUISize(resolution.x, resolution.y);
		}

		SetVolumeText();
		SetResolutionText();

		{
			GUIScene* scene = GetGUIScene("Options");

			if (!scene)
			{
				return;
			}

			for (GUIElement& element : scene->GetGUIElements())
			{
				if (element.myName == "FullscreenButton")
				{
					if (isFullscreen)
					{
						//SetGUIText(&element, "Yah");
						element.Reset();
					}
					else
					{
						//SetGUIText(&element, "Nah");
						element.Highlight();
					}
				}
				else if (element.myName == "FullscreenContainer")
				{
					myFullscreenContainer = &element;
				}
				else if (element.myName == "ResolutionContainer")
				{
					myResolutionContainer = &element;
				}
				else if (element.myName == "VolumeContainer")
				{
					myVolumeContainer = &element;
				}
			}
		}

		{ // Countdown setup
			GUIScene* scene = GetGUIScene("Countdown");

			if (scene)
			{
				for (auto& element : scene->GetGUIElements())
				{
					if (element.myName == "Ready")
					{
						myReadyFadeElement.myElement = &element;
						myReadyFadeElement.myFadeState = eFadeState::None;
						myReadyFadeElement.myFadeInTime = 0.5f;
						myReadyFadeElement.myFadeOutTime = 0.9f;
						myReadyFadeElement.myShowTime = 1.6f;
					}
					else if (element.myName == "Go")
					{
						myGoFadeElement.myElement = &element;
						myGoFadeElement.myFadeState = eFadeState::None;
						myGoFadeElement.myFadeInTime = 0.1f;
						myGoFadeElement.myFadeOutTime = 0.9f;
						myGoFadeElement.myShowTime = 1.0f;
					}
				}
			}
		}

		{
			GUIScene* scene = GetGUIScene("Score");

			if (scene)
			{
				int maxScore = myMaxScore;

				for (auto& element : scene->GetGUIElements())
				{
					if (element.myName == "P1ScoreMin")
					{
						myP1Score.myScoreMin = &element;
					}
					else if (element.myName == "P1ScoreMax")
					{
						myP1Score.myScoreMax = &element;
					}
					else if (element.myName == "P2ScoreMin")
					{
						myP2Score.myScoreMin = &element;
					}
					else if (element.myName == "P2ScoreMax")
					{
						myP2Score.myScoreMax = &element;
					}
					else if (element.myName == "P3ScoreMin")
					{
						myP3Score.myScoreMin = &element;
					}
					else if (element.myName == "P3ScoreMax")
					{
						myP3Score.myScoreMax = &element;
					}
					else if (element.myName == "P4ScoreMin")
					{
						myP4Score.myScoreMin = &element;
					}
					else if (element.myName == "P4ScoreMax")
					{
						myP4Score.myScoreMax = &element;
					}
					else if (element.myName == "Portrait1")
					{
						myPortraitPaths[0] = element.myTexturePath;
						myP1Score.myPortrait = &element;
					}
					else if (element.myName == "Portrait2")
					{
						myPortraitPaths[1] = element.myTexturePath;
						myP2Score.myPortrait = &element;
					}
					else if (element.myName == "Portrait3")
					{
						myPortraitPaths[2] = element.myTexturePath;
						myP3Score.myPortrait = &element;
					}
					else if (element.myName == "Portrait4")
					{
						myPortraitPaths[3] = element.myTexturePath;
						myP4Score.myPortrait = &element;
					}
					else if (element.myName == "P1Text")
					{
						myP1Score.myPlayerText = &element;
					}
					else if (element.myName == "P2Text")
					{
						myP2Score.myPlayerText = &element;
					}
					else if (element.myName == "P3Text")
					{
						myP3Score.myPlayerText = &element;
					}
					else if (element.myName == "P4Text")
					{
						myP4Score.myPlayerText = &element;
					}
					else if (element.myName == "P1Bg")
					{
						myP1Score.myPlayerTextBg = &element;
					}
					else if (element.myName == "P2Bg")
					{
						myP2Score.myPlayerTextBg = &element;
					}
					else if (element.myName == "P3Bg")
					{
						myP3Score.myPlayerTextBg = &element;
					}
					else if (element.myName == "P4Bg")
					{
						myP4Score.myPlayerTextBg = &element;
					}
					//else if (element.myName == "ScoreContainer")
					//{
					//	myP1Score.myScoreContainer = &element;
					//	myP2Score.myScoreContainer = &element;
					//	myP3Score.myScoreContainer = &element;
					//	myP4Score.myScoreContainer = &element;
					//}

				}

				myP1Score.myScoreElements.push_back(myP1Score.myScoreMin);
				myP2Score.myScoreElements.push_back(myP2Score.myScoreMin);
				myP3Score.myScoreElements.push_back(myP3Score.myScoreMin);
				myP4Score.myScoreElements.push_back(myP4Score.myScoreMin);

				for (GUIElement& element : scene->GetGUIElements())
				{
					if (element.myName.find("P1Score") != std::string::npos)
					{
						if (element.myName == "P1ScoreMin" || element.myName == "P1ScoreMax")
						{
							continue;
						}
						myP1Score.myScoreElements.push_back(&element);
					}
					else if (element.myName.find("P2Score") != std::string::npos)
					{
						if (element.myName == "P2ScoreMin" || element.myName == "P2ScoreMax")
						{
							continue;
						}
						myP2Score.myScoreElements.push_back(&element);
					}
					else if (element.myName.find("P3Score") != std::string::npos)
					{
						if (element.myName == "P3ScoreMin" || element.myName == "P3ScoreMax")
						{
							continue;
						}
						myP3Score.myScoreElements.push_back(&element);
					}
					else if (element.myName.find("P4Score") != std::string::npos)
					{
						if (element.myName == "P4ScoreMin" || element.myName == "P4ScoreMax")
						{
							continue;
						}
						myP4Score.myScoreElements.push_back(&element);
					}
				}

				myP1Score.myScoreElements.push_back(myP1Score.myScoreMax);
				myP2Score.myScoreElements.push_back(myP2Score.myScoreMax);
				myP3Score.myScoreElements.push_back(myP3Score.myScoreMax);
				myP4Score.myScoreElements.push_back(myP4Score.myScoreMax);
			}
		}

		{
			GUIScene* scene = GetGUIScene("Levels");

			if (scene)
			{
				for (auto& element : scene->GetGUIElements())
				{
					if (element.myName == "LeftButton")
					{
						myLevelSelectLeft = &element;
					}

					if (element.myName == "RightButton")
					{
						myLevelSelectRight = &element;
					}

					if (element.myName == "LevelImage")
					{
						myLevelSelectImage = &element;
					}

					if (element.myName == "LevelName")
					{
						myLevelSelectText = &element;
					}
				}
			}
		}

		//SetupScore(resolution);

		//// Setup cursor
		//SpriteData spriteData;
		//spriteData.myTexture = myGraphics->GetTextureLoader().GetTextureFromPath("Data/Assets/UI/01_UI_General/ui_cursor.dds");
		//spriteData.myMode = SpriteBatchMode::Screen;
		//
		//myCursorSpriteBatch.myData = spriteData;
		//myCursorSprite = &myCursorSpriteBatch.myInstances.emplace_back();
		//myCursorSprite->myAttributes.myTransform.SetScale({ 25.0f, 25.0f, 1.0f });
		//
		//myCursorSprite->myAttributes.myTransform.SetPosition({ 0.0f, 0.0f, 0.0f });
		//myCursorSprite->myAttributes.myColor = { 1, 1, 1, 1 };
	}

	void GUIHandler::SetupScore(const Vector2i& resolution)
	{
		{ // Score setup
			GUIScene* scene = GetGUIScene("Score");

			if (!scene)
			{
				return;
			}

			// TODO Get from somewhere else
			int maxScore = myMaxScore;

			for (auto& element : scene->GetGUIElements())
			{
				if (element.myName == "P1ScoreMin")
				{
					myP1Score.myScoreMin = &element;
				}
				else if (element.myName == "P1ScoreMax")
				{
					myP1Score.myScoreMax = &element;
				}
				else if (element.myName == "P2ScoreMin")
				{
					myP2Score.myScoreMin = &element;
				}
				else if (element.myName == "P2ScoreMax")
				{
					myP2Score.myScoreMax = &element;
				}
				else if (element.myName == "P3ScoreMin")
				{
					myP3Score.myScoreMin = &element;
				}
				else if (element.myName == "P3ScoreMax")
				{
					myP3Score.myScoreMax = &element;
				}
				else if (element.myName == "P4ScoreMin")
				{
					myP4Score.myScoreMin = &element;
				}
				else if (element.myName == "P4ScoreMax")
				{
					myP4Score.myScoreMax = &element;
				}
			}

			Vector2f fResolution = { (float)resolution.x, (float)resolution.y };

			float xMin = myP1Score.myScoreMin->myBox.myOffset.x;
			float xMax = myP1Score.myScoreMax->myBox.myOffset.x;
			//float width = xMax - xMin + myP1Score.myScoreMin->myBox.myWidth;
			//float offset = width / (float)maxScore + 1;

			float offset = (xMax - xMin) / (maxScore - 1);

			myP1Score.myScoreElements.reserve(16);
			myP2Score.myScoreElements.reserve(16);
			myP3Score.myScoreElements.reserve(16);
			myP4Score.myScoreElements.reserve(16);

			myP1Score.myScoreElements.push_back(myP1Score.myScoreMin);
			myP2Score.myScoreElements.push_back(myP2Score.myScoreMin);
			myP3Score.myScoreElements.push_back(myP3Score.myScoreMin);
			myP4Score.myScoreElements.push_back(myP4Score.myScoreMin);

			for (int i = 1; i < maxScore - 1; ++i)
			{
				GUIElement* newP1Element = &scene->DuplicateGUIElement(*myP1Score.myScoreMin, fResolution);
				AssignSpriteToGUIElement(
					*newP1Element,
					myGraphics,
					{ myP1Score.myScoreMin->myBox.myWidth, myP1Score.myScoreMin->myBox.myHeight },
					myP1Score.myScoreMin->myDisplayTexture->myMetadata.myFilePath);
				std::string p1Name = "P1Score" + std::to_string(i);
				newP1Element->myName = p1Name;
				newP1Element->myBox.myOffset.x = xMin + offset * i;
				newP1Element->mySecondaryTexture = myGraphics->GetTextureLoader().GetTextureFromPath(myP1Score.myScoreMin->mySecondaryTexture->myMetadata.myFilePath);
				newP1Element->UpdateSpriteScaleAndPosition(fResolution, false);
				//scene.MoveElement(scene.GetGUIElements().size() - 1, 0);

				GUIElement* newP2Element = &scene->DuplicateGUIElement(*myP2Score.myScoreMin, fResolution);
				AssignSpriteToGUIElement(
					*newP2Element,
					myGraphics,
					{ myP2Score.myScoreMin->myBox.myWidth, myP2Score.myScoreMin->myBox.myHeight },
					myP2Score.myScoreMin->myDisplayTexture->myMetadata.myFilePath);
				std::string p2Name = "P2Score" + std::to_string(i);
				newP2Element->myName = p2Name;
				newP2Element->myBox.myOffset.x = xMin + offset * i;
				newP2Element->mySecondaryTexture = myGraphics->GetTextureLoader().GetTextureFromPath(myP2Score.myScoreMin->mySecondaryTexture->myMetadata.myFilePath);
				newP2Element->UpdateSpriteScaleAndPosition(fResolution, false);
				//scene.MoveElement(scene.GetGUIElements().size() - 1, 0);

				GUIElement* newP3Element = &scene->DuplicateGUIElement(*myP3Score.myScoreMin, fResolution);
				AssignSpriteToGUIElement(
					*newP3Element,
					myGraphics,
					{ myP3Score.myScoreMin->myBox.myWidth, myP3Score.myScoreMin->myBox.myHeight },
					myP3Score.myScoreMin->myDisplayTexture->myMetadata.myFilePath);
				std::string p3Name = "P3Score" + std::to_string(i);
				newP3Element->myName = p3Name;
				newP3Element->myBox.myOffset.x = xMin + offset * i;
				newP3Element->mySecondaryTexture = myGraphics->GetTextureLoader().GetTextureFromPath(myP3Score.myScoreMin->mySecondaryTexture->myMetadata.myFilePath);
				newP3Element->UpdateSpriteScaleAndPosition(fResolution, false);
				//scene.MoveElement(scene.GetGUIElements().size() - 1, 0);

				GUIElement* newP4Element = &scene->DuplicateGUIElement(*myP4Score.myScoreMin, fResolution);
				AssignSpriteToGUIElement(
					*newP4Element,
					myGraphics,
					{ myP4Score.myScoreMin->myBox.myWidth, myP4Score.myScoreMin->myBox.myHeight },
					myP4Score.myScoreMin->myDisplayTexture->myMetadata.myFilePath);
				std::string p4Name = "P4Score" + std::to_string(i);
				newP4Element->myName = p4Name;
				newP4Element->myBox.myOffset.x = xMin + offset * i;
				newP4Element->mySecondaryTexture = myGraphics->GetTextureLoader().GetTextureFromPath(myP4Score.myScoreMin->mySecondaryTexture->myMetadata.myFilePath);
				newP4Element->UpdateSpriteScaleAndPosition(fResolution, false);
				//scene.MoveElement(scene.GetGUIElements().size() - 1, 0);


				myP1Score.myScoreElements.push_back(newP1Element);
				myP2Score.myScoreElements.push_back(newP2Element);
				myP3Score.myScoreElements.push_back(newP3Element);
				myP4Score.myScoreElements.push_back(newP4Element);
			}

			myP1Score.myScoreElements.push_back(myP1Score.myScoreMax);
			myP2Score.myScoreElements.push_back(myP2Score.myScoreMax);
			myP3Score.myScoreElements.push_back(myP3Score.myScoreMax);
			myP4Score.myScoreElements.push_back(myP4Score.myScoreMax);

			//bool moved1 = false;
			//bool moved2 = false;
			//bool moved3 = false;
			//bool moved4 = false;
			//while (!moved1 || !moved2 || !moved3 || !moved4)
			////{
			//	for (int i = 0; i < (int)scene.GetGUIElements().size(); ++i)
			//	{
			//		auto& elements = scene.GetGUIElements();
			//		if (elements[i].myName == "P1Bg")
			//		{
			//			GUIElement* newBg = &scene.DuplicateGUIElement(elements[i], fResolution);
			//			AssignSpriteToGUIElement(
			//				*newBg,
			//				myGraphics,
			//				{ elements[i].myBox.myWidth, elements[i].myBox.myHeight },
			//				elements[i].myDisplayTexture->myMetadata.myFilePath);
			//			newBg->myName = "P1Bg";
			//			newBg->mySecondaryTexture = myGraphics->GetTextureLoader().GetTextureFromPath(elements[i].mySecondaryTexture->myMetadata.myFilePath);
			//			scene.RemoveElement(i);
			//		}
			//		else if (elements[i].myName == "P2Bg")
			//		{
			//			//scene.MoveElement(i, scene.GetGUIElements().size());
			//			//moved2 = true;
			//			//std::cout << "\nMoved 2";
			//			GUIElement* newBg = &scene.DuplicateGUIElement(elements[i], fResolution);
			//			AssignSpriteToGUIElement(
			//				*newBg,
			//				myGraphics,
			//				{ elements[i].myBox.myWidth, elements[i].myBox.myHeight },
			//				elements[i].myDisplayTexture->myMetadata.myFilePath);
			//			newBg->myName = "P2Bg";
			//			newBg->mySecondaryTexture = myGraphics->GetTextureLoader().GetTextureFromPath(elements[i].mySecondaryTexture->myMetadata.myFilePath);
			//			scene.RemoveElement(i);
			//		}
			//		else if (elements[i].myName == "P3Bg")
			//		{
			//			//scene.MoveElement(i, scene.GetGUIElements().size());
			//			//moved3 = true;
			//			//std::cout << "\nMoved 3";
			//			GUIElement* newBg = &scene.DuplicateGUIElement(elements[i], fResolution);
			//			AssignSpriteToGUIElement(
			//				*newBg,
			//				myGraphics,
			//				{ elements[i].myBox.myWidth, elements[i].myBox.myHeight },
			//				elements[i].myDisplayTexture->myMetadata.myFilePath);
			//			newBg->myName = "P3Bg";
			//			newBg->mySecondaryTexture = myGraphics->GetTextureLoader().GetTextureFromPath(elements[i].mySecondaryTexture->myMetadata.myFilePath);
			//			scene.RemoveElement(i);
			//		}
			//		else if (elements[i].myName == "P4Bg")
			//		{
			//			//scene.MoveElement(i, scene.GetGUIElements().size());
			//			//moved4 = true;
			//			//std::cout << "\nMoved 4";
			//			GUIElement* newBg = &scene.DuplicateGUIElement(elements[i], fResolution);
			//			AssignSpriteToGUIElement(
			//				*newBg,
			//				myGraphics,
			//				{ elements[i].myBox.myWidth, elements[i].myBox.myHeight },
			//				elements[i].myDisplayTexture->myMetadata.myFilePath);
			//			newBg->myName = "P4Bg";
			//			newBg->mySecondaryTexture = myGraphics->GetTextureLoader().GetTextureFromPath(elements[i].mySecondaryTexture->myMetadata.myFilePath);
			//			scene.RemoveElement(i);
			//		}
			//	}
			//}

			scene->UpdateGUISize(resolution.x, resolution.y);
		}
	}

	void GUIHandler::AssignSpriteToGUIElement(GUIElement& aElement,
		Graphics* aGraphics,
		const Vector2f& aElementSize,
		const std::string& aSpritePath)
	{
		SpriteData spriteData;
		spriteData.myTexture = aGraphics->GetTextureLoader().GetTextureFromPath(aSpritePath);
		spriteData.myMode = SpriteBatchMode::Screen;
		spriteData.myEffectType = ProgressionDirectionToEffectType(aElement.myProgressionDirection);

		aElement.myDisplayTexture = spriteData.myTexture;
		if (aElement.mySecondaryTexture == nullptr)
		{
			aElement.mySecondaryTexture = spriteData.myTexture;
		}

		aElement.mySpriteBatch.myData = spriteData;
		aElement.mySpriteBatch.myInstances.emplace_back();
		aElement.mySpriteBatch.myInstances[0].myAttributes.myTransform.SetScale({ aElementSize.x, aElementSize.y, 1.0f });

		Vector2f pos = aElement.myBox.myScreenPosition;
		aElement.mySpriteBatch.myInstances[0].myAttributes.myTransform.SetPosition({ pos.x, pos.y - aElementSize.y, 0.0f });
		aElement.mySpriteBatch.myInstances[0].myAttributes.myColor = { 1, 1, 1, 1 };
	}

	void GUIHandler::AssignSpriteToTooltip(GUITooltip& aTooltip,
		const Window* aWindow,
		const Vector2f& aTooltipSize,
		const std::string& aSpritePath)
	{
		SpriteData spriteData;
		spriteData.myTexture = aWindow->GetGraphics().GetTextureLoader().GetTextureFromPath(aSpritePath);
		spriteData.myMode = SpriteBatchMode::Screen;

		aTooltip.mySpriteBatch.myData = spriteData;
		aTooltip.mySprite = &aTooltip.mySpriteBatch.myInstances.emplace_back();
		aTooltip.mySprite->myAttributes.myTransform.SetScale({ aTooltipSize.x, aTooltipSize.y, 1.0f });

		Vector2f pos = aTooltip.myBox.myScreenPosition;
		aTooltip.mySprite->myAttributes.myTransform.SetPosition({ pos.x, pos.y - aTooltipSize.y, 0.0f });
		aTooltip.mySprite->myAttributes.myColor = { 1, 1, 1, 1 };
	}

	void GUIHandler::CreateGUIScene(const std::string& aSceneName, const size_t aNumberOfElementsToReserve)
	{
		myGUISceneMap[aSceneName] = GUIScene();
		myGUISceneMap[aSceneName].Init(aSceneName, aNumberOfElementsToReserve);
	}

	void GUIHandler::CreateGUIScene(const std::string& aSceneName)
	{
		myGUISceneMap[aSceneName] = GUIScene();
		myGUISceneMap[aSceneName].Init(aSceneName);
	}

	void GUIHandler::RemoveGUIScene(const std::string& aSceneName)
	{
		myGUISceneMap.erase(aSceneName);
	}

	void GUIHandler::PushGUIScene(const std::string& aSceneName)
	{
		for (GUIScene* scene : myActiveGUIScenes)
		{
			if (scene->GetSceneName() == aSceneName)
			{
				return;
			}
		}

		myActiveGUIScenes.push_front(&myGUISceneMap[aSceneName]);
		myGUISceneMap[aSceneName].SetActive(true);

		mySelectedElement = nullptr;

		SelectFirstMenuElement();
	}

	void GUIHandler::PopGUIScene(const std::string& aSceneName)
	{
		myGUISceneMap[aSceneName].SetActive(false);
		std::erase(myActiveGUIScenes, &myGUISceneMap[aSceneName]);

		mySelectedElement = nullptr;
	}

	void GUIHandler::PopGUIScene()
	{
		myActiveGUIScenes.pop_front();

		mySelectedElement = nullptr;
	}

	GUIScene* GUIHandler::GetGUIScene(const std::string& aSceneName)
	{
		if (!myGUISceneMap.contains(aSceneName))
		{
			return nullptr;
		}
		return &myGUISceneMap[aSceneName];
	}

	bool GUIHandler::IsMouseOverGUI(const Vector2f& aMousePosition)
	{
		bool isMouseOverGUI = false;

		// No active scene
		if (myActiveGUIScenes.empty())
		{
			return false;
		}

		// Check if mouse is over any clickable GUI element
		for (GUIElement& element : myActiveGUIScenes.front()->GetGUIElements())
		{
			if (element.myBox.IsInside(aMousePosition))
			{
				if (element.isButton)
				{
					isMouseOverGUI = true;
				}
			}
		}

		// Stopped hovering over this element
		if (!isMouseOverGUI && myHoveredElement != nullptr)
		{
			myHoveredElement->Reset();
			myHoveredElement = nullptr;
		}

		return isMouseOverGUI;
	}

	GUIElement* GUIHandler::GetGUIElementFromMousePosition(const Vector2f& aMousePosition)
	{
		// If no active scene, return nullptr
		if (myActiveGUIScenes.empty())
		{
			if (myHoveredElement != nullptr)
			{
				myHoveredElement->Reset();
				myHoveredElement = nullptr;
			}
			return nullptr;
		}

		// Check if mouse is over any GUI element
		for (GUIElement& element : myActiveGUIScenes.front()->GetGUIElements())
		{
			if (element.myBox.IsInside(aMousePosition))
			{
				// Don't return non-clickable elements, they are not interactable
				if (element.isButton)
				{
					// If hovering over a new element that is not the same as the previous one
					if (myHoveredElement != nullptr && myHoveredElement != &element)
					{
						myHoveredElement->Reset();
					}
					myHoveredElement = &element;
					return &element;
				}
			}
		}

		// Stopped hovering over this element
		if (myHoveredElement != nullptr)
		{
			myHoveredElement->Reset();
			myHoveredElement = nullptr;
		}
		return nullptr;
	}

	GUIElement* GUIHandler::GetAnyGUIElementFromMousePosition(const Vector2f& aMousePosition)
	{
		// If no active scene, return nullptr
		if (myActiveGUIScenes.empty())
		{
			if (myHoveredElement != nullptr)
			{
				myHoveredElement->Reset();
				myHoveredElement = nullptr;
			}
			return nullptr;
		}

		// Check if mouse is over any GUI element
		for (std::pair<const std::string, GUIScene>& scene : myGUISceneMap)
		{
			for (GUIElement& element : scene.second.GetGUIElements())
			{
				if (element.myBox.IsInside(aMousePosition))
				{
					return &element;
				}
			}
		}
		return nullptr;
	}

	void GUIHandler::UpdateGUI()
	{
		// Imgui colour wheel
		//if (ImGui::Begin("Colour Wheels"))
		//{
		//ImGui::ColorPicker4("Player1", (float*)&myCharacterColours[0], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		//ImGui::ColorPicker4("Player2", (float*)&myCharacterColours[1], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		//ImGui::ColorPicker4("Player3", (float*)&myCharacterColours[2], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		//ImGui::ColorPicker4("Player4", (float*)&myCharacterColours[3], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		//}
		//ImGui::End();

		//if (GetAsyncKeyState('C') == SHORT_MIN + 1)
		//{
		//	myCursorSpriteBatch.myInstances[0].myAttributes.myColor.a = myCursorSpriteBatch.myInstances[0].myAttributes.myColor.a < 1.0f ? 1.0f : 0.0f;
		//}

		//if (GetAsyncKeyState('V') == SHORT_MIN + 1)
		//{
		//	if (myScore[0] < 0)
		//	{
		//		myScore[0] = 1;
		//	}
		//	else
		//	{
		//		myScore[0]++;
		//	}
		//	SetScore(myScore);
		//}

		//if (GetAsyncKeyState('B') == SHORT_MIN + 1)
		//{
		//	if (myScore[1] < 0)
		//	{
		//		myScore[1] = 1;
		//	}
		//	else
		//	{
		//		myScore[1]++;
		//	}
		//	SetScore(myScore);
		//}

		//if (GetAsyncKeyState('N') == SHORT_MIN + 1)
		//{
		//	if (myScore[2] < 0)
		//	{
		//		myScore[2] = 1;
		//	}
		//	else
		//	{
		//		myScore[2]++;
		//	}
		//	SetScore(myScore);
		//}

		//if (GetAsyncKeyState('M') == SHORT_MIN + 1)
		//{
		//	if (myScore[3] < 0)
		//	{
		//		myScore[3] = 1;
		//	}
		//	else
		//	{
		//		myScore[3]++;
		//	}
		//	SetScore(myScore);
		//}

		//if (GetAsyncKeyState('L') == SHORT_MIN + 1)
		//{
		//	ResetScores();
		//}

		const float deltaTime = KE_GLOBAL::deltaTime;
		/// -----------------------------------------------------------------------------------------------
		/// Active scene
		if (myActiveGUIScenes.empty())
		{
			return;
		}

		HandleEventTimers();

		for (GUIScene* scene : myActiveGUIScenes)
		{
			for (GUIElement& element : scene->GetGUIElements())
			{
				element.Update(deltaTime);
			}
		}
	}

	void GUIHandler::RenderGUI(Graphics* aGraphics)
	{
		Vector2i resolution = aGraphics->GetRenderSize();

		if (myActiveGUIScenes.empty())
		{
			return;
		}

		if (!isGUIEnabled)
		{
			return;
		}

		DrawGUIGrid(aGraphics, resolution);

		/// -----------------------------------------------------------------------------------------------
		/// Active scene
		for (int i = (int)myActiveGUIScenes.size() - 1; i >= 0; --i)
		{
			for (int j = (int)myActiveGUIScenes[i]->GetGUIElements().size() - 1; j >= 0; --j)
			{
				GUIElement& element = myActiveGUIScenes[i]->GetGUIElements()[j];
				if (!element.shouldDraw)
				{
					continue;
				}

				// Draw element sprites
				if (element.mySpriteBatch.myData.myTexture)
				{
					aGraphics->GetSpriteManager().QueueSpriteBatch(&element.mySpriteBatch);
				}

				Vector2f linePoints[4];
				linePoints[0] = { element.myBox.myScreenPosition.x, element.myBox.myScreenPosition.y };
				linePoints[1] = { element.myBox.myScreenPosition.x + element.myBox.myWidth, element.myBox.myScreenPosition.y };
				linePoints[2] = { element.myBox.myScreenPosition.x + element.myBox.myWidth, element.myBox.myScreenPosition.y - element.myBox.myHeight };
				linePoints[3] = { element.myBox.myScreenPosition.x, element.myBox.myScreenPosition.y - element.myBox.myHeight };

				// Draw debug lines
				aGraphics->AddScreenSpaceLine(linePoints[0], linePoints[1], element.myColour);
				aGraphics->AddScreenSpaceLine(linePoints[1], linePoints[2], element.myColour);
				aGraphics->AddScreenSpaceLine(linePoints[2], linePoints[3], element.myColour);
				aGraphics->AddScreenSpaceLine(linePoints[3], linePoints[0], element.myColour);

				// Draw tooltip sprites
				if (element.myTooltip.isActive && element.myTooltip.isInitialized)
				{
					element.myTooltip.DrawTooltip(aGraphics);
					aGraphics->GetSpriteManager().QueueSpriteBatch(&element.myTooltip.mySpriteBatch);
				}
			}
		}

		if (myCursorSpriteBatch.myData.myTexture)
		{
			aGraphics->GetSpriteManager().QueueSpriteBatch(&myCursorSpriteBatch);
		}
	}

	void GUIHandler::DrawGUIGrid(Graphics* aGraphics, const Vector2i& aResolution)
	{
		if (!drawGUIGrid)
		{
			return;
		}


		const float width = (float)aResolution.x;
		const float height = (float)aResolution.y;

		float gridSizeX;
		float gridSizeY;

		if (gridSquareCells)
		{
			const float aspectRatio = (float)aResolution.x / (float)aResolution.y;
			gridSizeX = aResolution.x / (float)myGridSizeX;
			gridSizeY = aResolution.y / (float)myGridSizeX * aspectRatio;
		}
		else
		{
			gridSizeX = aResolution.x / (float)myGridSizeX;
			gridSizeY = aResolution.y / (float)myGridSizeY;
		}

		// Vertical lines
		for (float x = 0; x < width; x += gridSizeX)
		{
			aGraphics->AddScreenSpaceLine({ x, 0.0f }, { x, height }, { 1.0f, 1.0f, 1.0f, 0.75f });
		}

		// Horizontal lines
		for (float y = 0; y < height; y += gridSizeY)
		{
			aGraphics->AddScreenSpaceLine({ 0.0f, y }, { width, y }, { 1.0f, 1.0f, 1.0f, 0.75f });
		}
	}

	void GUIHandler::ToggleDrawGUIGrid()
	{
		drawGUIGrid = !drawGUIGrid;
	}

	void GUIHandler::SetVolumeSliderPosition()
	{
		float minX = -30.0f / 1920.0f;
		float maxX = 320.0f / 1920.0f;

		float sliderPosition = lerp(minX, maxX, (float)myCurrentVolume * 0.1f);
		Vector2i currentResolution = myGraphics->GetRenderSize();
		myVolumeSlider->myBox.myOffset.x = sliderPosition * (float)currentResolution.x;
		myVolumeSlider->UpdateSpriteScaleAndPosition({ (float)currentResolution.x, (float)currentResolution.y }, false);
	}

	void GUIHandler::IncreaseVolume()
	{
		const int newVolume = myCurrentVolume + myVolumeIncrement;
		myCurrentVolume = std::clamp(newVolume, myMinVolume, myMaxVolume);

		GUIAudioVolumeEvent volumeEvent((float)myCurrentVolume * 0.1f);
		ES::EventSystem::GetInstance().SendEvent(volumeEvent);

		KE_GLOBAL::audioWrapper.SetMasteringVolume((float)myCurrentVolume * 0.1f);

		SetVolumeSliderPosition();
	}

	void GUIHandler::DecreaseVolume()
	{
		const int newVolume = myCurrentVolume - myVolumeIncrement;
		myCurrentVolume = std::clamp(newVolume, myMinVolume, myMaxVolume);

		GUIAudioVolumeEvent volumeEvent((float)myCurrentVolume * 0.1f);
		ES::EventSystem::GetInstance().SendEvent(volumeEvent);

		KE_GLOBAL::audioWrapper.SetMasteringVolume((float)myCurrentVolume * 0.1f);

		SetVolumeSliderPosition();
	}

	void GUIHandler::SetResolution() const
	{
		const Vector2i resolution = RESOLUTIONS[myCurrentResolutionIndex];

		GUIResolutionEvent resolutionEvent;
		resolutionEvent.myWidth = resolution.x;
		resolutionEvent.myHeight = resolution.y;
		resolutionEvent.myFullscreen = isFullscreen;

		ES::EventSystem::GetInstance().SendEvent(resolutionEvent);
	}

	void GUIHandler::SetVolumeText()
	{
		if (!myGUISceneMap.contains("Options"))
		{
			return;
		}

		GUIScene* scene = &myGUISceneMap["Options"];

		for (GUIElement& element : scene->GetGUIElements())
		{
			if (element.myName == "VolumeSlider")
			{
				myVolumeSlider = &element;
				SetVolumeSliderPosition();
			}
		}
	}

	void GUIHandler::SetResolutionText()
	{
		GUIScene* scene = GetGUIScene("Options");

		if (!scene)
		{
			return;
		}

		std::string resolutions[3] = {
					"1280x720",
					"1920x1080",
					"2560x1440"
		};

		for (GUIElement& element : scene->GetGUIElements())
		{
			if (element.myName == "Resolution")
			{
				SetGUIText(&element, resolutions[myCurrentResolutionIndex]);
			}
		}
	}

	void GUIHandler::SetScore(P8::PresentScoreDataEvent* aEvent)
	{
		GUIScene* scene = GetGUIScene("Score");

		if (!scene)
		{
			KE_LOG("Score scene not found");
			return;
		}

		for (int i = 0; i < 4; ++i)
		{
			int score = -1;
			int modelID = -1;
			if (aEvent == nullptr)
			{
				score = 0;
			}
			else
			{
				score = aEvent->scores[i];
				modelID = aEvent->modelID[i];
			}

			bool playerExists = true;

			if (score < 0 || score > myMaxScore)
			{
				if (aEvent != nullptr)
				{
					if (aEvent->modelID[i] == -1 && aEvent->playerID[i] == -1)
					{
						// Don't draw score for player that doesn't exist
						std::cout << "\nPlayer " << i << " doesn't exist";
						playerExists = false;
					}
				}

				//continue;
			}

			switch (i)
			{
			case 0:
			{
				if (playerExists)
				{
					SetScoreForPlayer(&myP1Score, score, modelID);
					SetPortraitForPlayer(&myP1Score, modelID);
					myScore[0] = score;
					//std::cout << "\nSetting score for player 1: " << score;
				}
				else
				{
					HideScoreForNonActivePlayer(0);
				}
				break;
			}
			case 1:
			{
				if (playerExists)
				{
					SetScoreForPlayer(&myP2Score, score, modelID);
					SetPortraitForPlayer(&myP2Score, modelID);
					myScore[1] = score;
				}
				else
				{
					HideScoreForNonActivePlayer(1);
				}
				//std::cout << "\nSetting score for player 2: " << score;
				break;
			}
			case 2:
			{
				if (playerExists)
				{
					SetScoreForPlayer(&myP3Score, score, modelID);
					SetPortraitForPlayer(&myP3Score, modelID);
					myScore[2] = score;
					//std::cout << "\nSetting score for player 3: " << score;
				}
				else
				{
					HideScoreForNonActivePlayer(2);
				}
				break;
			}
			case 3:
			{
				if (playerExists)
				{
					SetScoreForPlayer(&myP4Score, score, modelID);
					SetPortraitForPlayer(&myP4Score, modelID);
					myScore[3] = score;
				}
				else
				{
					HideScoreForNonActivePlayer(3);
				}
				//std::cout << "\nSetting score for player 4: " << score;
				break;
			}
			}
		}
	}

	void GUIHandler::SetScoreForPlayer(Score* aPScore, const int aScore, const int aModelID)
	{
		if (aPScore->myScore < 0 && aScore >= 0)
		{
			aPScore->myScore = 0;
			aPScore->myScorePreviousRound = 0;
		}

		aPScore->myModelID = aModelID;
		aPScore->myScore = aScore;
	}

	void GUIHandler::HideScoreForNonActivePlayer(const int aPlayerIndex)
	{
		GUIScene* scene = GetGUIScene("Score");

		if (!scene)
		{
			return;
		}

		switch (aPlayerIndex)
		{
		case 0:
		{
			// Player 1 will never NOT exist but just in case :^)
			for (auto& element : scene->GetGUIElements())
			{
				if (element.myName == "Portrait1")
				{
					element.shouldDraw = false;
				}

				if (element.myName == "P1Bg")
				{
					element.shouldDraw = false;
				}

				if (element.myName == "P1Text")
				{
					element.shouldDraw = false;
				}
			}

			for (auto& element : myP1Score.myScoreElements)
			{
				element->shouldDraw = false;
			}
			break;
		}
		case 1:
		{
			for (auto& element : scene->GetGUIElements())
			{
				if (element.myName == "Portrait2")
				{
					element.shouldDraw = false;
				}

				if (element.myName == "P2Bg")
				{
					element.shouldDraw = false;
				}

				if (element.myName == "P2Text")
				{
					element.shouldDraw = false;
				}
			}

			for (auto& element : myP2Score.myScoreElements)
			{
				element->shouldDraw = false;
			}
			break;
		}
		case 2:
		{
			for (auto& element : scene->GetGUIElements())
			{
				if (element.myName == "Portrait3")
				{
					element.shouldDraw = false;
				}

				if (element.myName == "P3Bg")
				{
					element.shouldDraw = false;
				}

				if (element.myName == "P3Text")
				{
					element.shouldDraw = false;
				}
			}

			for (auto& element : myP3Score.myScoreElements)
			{
				element->shouldDraw = false;
			}
			break;
		}
		case 3:
		{
			for (auto& element : scene->GetGUIElements())
			{
				if (element.myName == "Portrait4")
				{
					element.shouldDraw = false;
				}

				if (element.myName == "P4Bg")
				{
					element.shouldDraw = false;
				}

				if (element.myName == "P4Text")
				{
					element.shouldDraw = false;
				}
			}

			for (auto& element : myP4Score.myScoreElements)
			{
				element->shouldDraw = false;
			}
			break;
		}
		}
	}

	void GUIHandler::SetPortraitForPlayer(Score* aPSScore, const int aModelID)
	{
		if (aModelID < 1 || aModelID > 5)
		{
			return;
		}
		aPSScore->myPortrait->mySpriteBatch.myData.myTexture = myGraphics->GetTextureLoader().GetTextureFromPath(myPortraitPaths[aModelID - 1]);
		aPSScore->myPlayerTextBg->mySpriteBatch.myData.myTexture = myGraphics->GetTextureLoader().GetTextureFromPath(myCharacterScoreboardPaths[aModelID - 1]);
	}

	void GUIHandler::ResetScores()
	{
		myScore[0] = 0;
		myScore[1] = 0;
		myScore[2] = 0;
		myScore[3] = 0;

		myP1Score.myScore = 0;
		myP2Score.myScore = 0;
		myP3Score.myScore = 0;
		myP4Score.myScore = 0;

		SetScore(nullptr);

		GUIScene* scene = GetGUIScene("Score");

		if (scene)
		{
			for (auto& element : scene->GetGUIElements())
			{
				// P1
				if (element.myName == "Portrait1")
				{
					element.shouldDraw = true;
				}

				if (element.myName == "P1Bg")
				{
					element.shouldDraw = true;
				}

				if (element.myName == "P1Text")
				{
					element.shouldDraw = true;
				}

				// P2
				if (element.myName == "Portrait2")
				{
					element.shouldDraw = true;
				}

				if (element.myName == "P2Bg")
				{
					element.shouldDraw = true;
				}

				if (element.myName == "P2Text")
				{
					element.shouldDraw = true;
				}

				// P3
				if (element.myName == "Portrait3")
				{
					element.shouldDraw = true;
				}

				if (element.myName == "P3Bg")
				{
					element.shouldDraw = true;
				}

				if (element.myName == "P3Text")
				{
					element.shouldDraw = true;
				}

				// P4
				if (element.myName == "Portrait4")
				{
					element.shouldDraw = true;
				}

				if (element.myName == "P4Bg")
				{
					element.shouldDraw = true;
				}

				if (element.myName == "P4Text")
				{
					element.shouldDraw = true;
				}
			}

			for (auto& element : myP1Score.myScoreElements)
			{
				element->mySpriteBatch.myData.myTexture = element->myDisplayTexture;
				element->shouldDraw = true;
			}

			for (auto& element : myP2Score.myScoreElements)
			{
				element->mySpriteBatch.myData.myTexture = element->myDisplayTexture;
				element->shouldDraw = true;
			}

			for (auto& element : myP3Score.myScoreElements)
			{
				element->mySpriteBatch.myData.myTexture = element->myDisplayTexture;
				element->shouldDraw = true;
			}

			for (auto& element : myP4Score.myScoreElements)
			{
				element->mySpriteBatch.myData.myTexture = element->myDisplayTexture;
				element->shouldDraw = true;
			}

			myP1Score.myScore = 0;
			myP1Score.myScorePreviousRound = 0;

			myP2Score.myScore = 0;
			myP2Score.myScorePreviousRound = 0;

			myP3Score.myScore = 0;
			myP3Score.myScorePreviousRound = 0;

			myP4Score.myScore = 0;
			myP4Score.myScorePreviousRound = 0;
		}
	}

	void GUIHandler::SetGUIText(GUIElement* aElement, const std::string& aText)
	{
		if (aElement == nullptr)
		{
			return;
		}

		if (!aElement->hasText)
		{
			return;
		}

		aElement->myText = aText;
		size_t stringLength = aText.length();

		constexpr size_t charCount = 16;

		aElement->mySpriteBatch.myInstances.resize(charCount);
		aElement->mySpriteBatch.myData.myMode = SpriteBatchMode::ScreenText;
		aElement->mySpriteBatch.myData.myTexture = myFont.GetFontAtlas();
		auto& instances = aElement->mySpriteBatch.myInstances;
		instances[0].myAttributes.myColor.a = 0.0f;
		Vector4f& textColour = aElement->myTextColour;

		Sprite* textSprites[charCount];
		for (int i = 1; i < charCount; i++)
		{
			textSprites[i - 1] = &instances[i];
			instances[i].myAttributes.myColor = { textColour.x, textColour.y, textColour.z, textColour.w };
			instances[i].myAttributes.myColor.a = i > stringLength ? 0.0f : textColour.w;
		}

		aElement->mySpriteBatch.myTextStyling.text.horizontalAlign = KE::TextAlign::Center;

		myFont.PrepareSprites(
			textSprites,
			aText,
			aElement->mySpriteBatch.myTextStyling,
			instances[0].myAttributes.myTransform
		);

		aElement->mySpriteBatch.myCustomPS = myGraphics->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH "SDFRendering_PS.cso");
	}

	void GUIHandler::MoveElementBetweenScenes(const std::string& aSourceSceneName, const std::string& aDestinationSceneName, const size_t aElementIndex)
	{
		if (!myGUISceneMap.contains(aSourceSceneName) || !myGUISceneMap.contains(aDestinationSceneName))
		{
			return;
		}

		GUIScene* sourceScene = &myGUISceneMap[aSourceSceneName];
		GUIScene* destinationScene = &myGUISceneMap[aDestinationSceneName];

		if (aElementIndex >= sourceScene->GetGUIElements().size())
		{
			return;
		}

		GUIElement copy = sourceScene->GetGUIElements()[aElementIndex];
		sourceScene->RemoveElement(aElementIndex);
		destinationScene->AddElement(copy);
	}

	std::string GUIHandler::GetCurrentSceneName() const
	{
		if (myActiveGUIScenes.empty())
		{
			return "";
		}

		return myActiveGUIScenes.front()->GetSceneName();
	}

	void GUIHandler::RegisterAssignedEvent(const std::string& aEventName, GUIElement* aElement)
	{
		if (aElement->myEvent.myEventName == aEventName)
		{
			return;
		}
		myEventMap[aEventName].push_back(aElement);
		aElement->myEvent.myEventName = aEventName;
	}

	void GUIHandler::UnregisterAssignedEvent(GUIElement* aElement)
	{
		std::erase(myEventMap[aElement->myEvent.myEventName], aElement);

		if (myEventMap[aElement->myEvent.myEventName].empty())
		{
			myEventMap.erase(aElement->myEvent.myEventName);
		}
		aElement->myEvent.myEventName = "";
	}

	void GUIHandler::HandleEventTimers()
	{
		HandleBlockInput();
		HandleScoreAnimation();
		HandleCountdown();
	}

	void GUIHandler::HandleScoreAnimation()
	{
		if (isScoreAnimating)
		{
			myScoreAnimationTimer += KE_GLOBAL::deltaTime;

			if (myScoreAnimationTimer >= myScoreAnimationTime)
			{
				myScoreAnimationTimer = 0.0f;

				switch (myScoreAnimIndex)
				{
				case 0:
				{
					AnimateScore(&myP1Score);
					break;
				}
				case 1:
				{
					AnimateScore(&myP2Score);
					break;
				}
				case 2:
				{
					AnimateScore(&myP3Score);
					break;
				}
				case 3:
				{
					AnimateScore(&myP4Score);
					break;
				}
				}
			}
		}
	}

	void GUIHandler::HandleBlockInput()
	{
		if (isBlockingInputTimed)
		{
			myBlockInputTimer += KE_GLOBAL::deltaTime;
			if (myBlockInputTimer >= myBlockInputTime)
			{
				myBlockInputTimer = 0.0f;
				isBlockingInputTimed = false;
				isBlockingInput = false;

				GUIScene* scene = myActiveGUIScenes.front();

				if (scene)
				{
					// Get the continue text and button elements
					for (GUIElement& element : scene->GetGUIElements())
					{
						if (element.myName == "ContinueText")
						{
							element.shouldDraw = true;
						}
						if (element.myName == "ContinueA")
						{
							element.shouldDraw = true;
						}
					}
				}
			}
		}
	}

	void GUIHandler::HandleCountdown()
	{
		if (isCountdownActive)
		{
			if (myFadeInElement == &myReadyFadeElement)
			{
				if (FadeElement(myFadeInElement) == eFadeState::Done)
				{
					myFadeInElement->myFadeState = eFadeState::None;
					myFadeInElement = &myGoFadeElement;
					myFadeInElement->myFadeState = eFadeState::FadeIn;

					KE::GlobalAudio::PlaySFX(sound::SFX::AnnouncerGo);
				}
			}
			else if (myFadeInElement == &myGoFadeElement)
			{
				if (FadeElement(myFadeInElement) == eFadeState::Done)
				{
					myFadeInElement->myFadeState = eFadeState::None;
					myFadeInElement = nullptr;
					isCountdownActive = false;
					PopGUIScene("Countdown");
					//myActiveGUIScenes.clear();
				}
			}

			if (shouldCountDown)
			{
				myCountdownTimer += KE_GLOBAL::deltaTime;
			}

			if (myCountdownTimer >= myCountdownMaxTime)
			{
				myCountdownTimer = 0.0f;
				shouldCountDown = false;
				P8::ChangeGameState changeGameStateEvent(P8::eGameStates::ePlayLiveGame);
				ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);
			}
		}
	}

	void GUIHandler::HandleMenuElement(const bool aIsLeft)
	{
		if (mySelectedElement == nullptr)
		{
			return;
		}

		if (mySelectedElement->myName == "VolumeContainer")
		{
			HandleVolumeSlider(aIsLeft);
		}
		else if (mySelectedElement->myName == "LevelChange")
		{
			HandleLevelSelect(aIsLeft);
		}
		else if (mySelectedElement->myName == "ResolutionContainer")
		{
			HandleResolutionChange(aIsLeft);
		}
		else if (mySelectedElement->myName == "FullscreenContainer")
		{
			HandleFullscreenChange();
		}
	}

	void GUIHandler::HandleVolumeSlider(const bool aIsLeft)
	{
		if (myVolumeSlider == nullptr)
		{
			return;
		}

		if (aIsLeft)
		{
			DecreaseVolume();
		}
		else
		{
			IncreaseVolume();
		}
	}

	void GUIHandler::HandleLevelSelect(const bool aIsLeft)
	{
		if (aIsLeft)
		{
			if (!myLevelSelectLeft)
			{
				return;
			}
			myLevelSelectLeft->Highlight();
			myLevelSelectRight->Reset();

			myCurrentLevelSelectIndex--;
			if (myCurrentLevelSelectIndex < 0)
			{
				myCurrentLevelSelectIndex = (int)myLevelSelectData.size() - 1;
			}
		}
		else
		{
			if (!myLevelSelectRight)
			{
				return;
			}
			myLevelSelectRight->Highlight();
			myLevelSelectLeft->Reset();

			myCurrentLevelSelectIndex++;
			if (myCurrentLevelSelectIndex >= (int)myLevelSelectData.size())
			{
				myCurrentLevelSelectIndex = 0;
			}
		}

		myCurrentLevelImagePath = myLevelImageBasePath + myLevelSelectData[myCurrentLevelSelectIndex].myLevelName + ".dds";
		myLevelIndex = myLevelSelectData[myCurrentLevelSelectIndex].myLevelIndex;
		myLevelSelectImage->mySpriteBatch.myData.myTexture = myGraphics->GetTextureLoader().GetTextureFromPath(myCurrentLevelImagePath);

		std::string uppercaseLevelName = myLevelSelectData[myCurrentLevelSelectIndex].myLevelName;
		std::transform(uppercaseLevelName.begin(), uppercaseLevelName.end(), uppercaseLevelName.begin(), ::toupper);
		SetGUIText(myLevelSelectText, uppercaseLevelName);
	}

	void GUIHandler::HandleResolutionChange(const bool aIsLeft)
	{
		if (aIsLeft)
		{
			myCurrentResolutionIndex--;
			if (myCurrentResolutionIndex < 0)
			{
				myCurrentResolutionIndex = 2;
			}
		}
		else
		{
			myCurrentResolutionIndex++;
			if (myCurrentResolutionIndex >= 3)
			{
				myCurrentResolutionIndex = 0;
			}
		}

		SetResolution();
		SetResolutionText();
	}

	void GUIHandler::HandleFullscreenChange()
	{
		GUIScene* scene = GetGUIScene("Options");

		if (!scene)
		{
			return;
		}

		isFullscreen = !isFullscreen;

		for (GUIElement& element : scene->GetGUIElements())
		{
			if (element.myName == "FullscreenButton")
			{
				if (isFullscreen)
				{
					element.Reset();
				}
				else
				{
					element.Highlight();
				}
			}
		}

		SetResolution();
	}

	void GUIHandler::AnimateScore(Score* aScore)
	{
		if (myScore[myScoreAnimIndex] > 0)
		{
			if (aScore->myScorePreviousRound < 0)
			{
				aScore->myScorePreviousRound = 0;
			}

			if (aScore->myScorePreviousRound == myMaxScore)
			{
				IncreaseScoreIndex();
				return;
			}

			GUIElement* element = aScore->myScoreElements[aScore->myScorePreviousRound];

			if (aScore->myScorePreviousRound < aScore->myScore && aScore->myScorePreviousRound < myMaxScore)
			{
				element->mySpriteBatch.myData.myTexture = myGraphics->GetTextureLoader().GetTextureFromPath(myCharacterPointPaths[aScore->myModelID - 1]);
				aScore->myScorePreviousRound++;
				KE::GlobalAudio::PlaySFX(sound::SFX::MenuToggle);
			}
			else
			{
				IncreaseScoreIndex();
				element->mySpriteBatch.myData.myTexture = myGraphics->GetTextureLoader().GetTextureFromPath(myCharacterDefaultPointPath);
			}
		}
		else
		{
			IncreaseScoreIndex();
		}
	}

	void GUIHandler::IncreaseScoreIndex()
	{
		if (myScoreAnimIndex < 3)
		{
			myScoreAnimIndex++;
		}
		else
		{
			myScoreAnimIndex = 0;
			isScoreAnimating = false;
		}
	}

	void GUIHandler::OnReceiveEvent(ES::Event& aEvent)
	{
		/// -----------------------------------------------------------------------------------------------
		///	OVERRIDES GUI EVENTS
		if (InputEvent* event = dynamic_cast<InputEvent*>(&aEvent))
		{
			if (isBlockingInputTimed || isBlockingInput)
			{
				return;
			}

			for (Interaction& interaction : event->interactions)
			{
				if (interaction.myInteractionType == eInteractionType::Pressed)
				{
					if (interaction.myInputType == eInputType::Esc ||
						interaction.myInputType == eInputType::XboxB ||
						interaction.myInputType == eInputType::XboxStart)
					{
						if (GetCurrentSceneName() == "Pause")
						{
							P8::ChangeGameState changeGameStateEvent(P8::eGameStates::ePlayLiveGame);
							ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);
							PopGUIScene();
						}
						else if (GetCurrentSceneName() == "Options" ||
							GetCurrentSceneName() == "Credits")
						{
							PopGUIScene();
							SelectFirstMenuElement();
						}
						else if (GetCurrentSceneName() == "QuitConfirm")
						{
							PopGUIScene();
							SelectLastMenuElement();
						}
						else if (GetCurrentSceneName() == "MenuConfirm")
						{
							PopGUIScene();
							SelectFirstMenuElement();
						}
						else if (GetCurrentSceneName() == "Levels")
						{
							if (interaction.myInputType == eInputType::XboxStart)
							{
								myActiveGUIScenes.clear();

								P8::StartingLevelEvent startingLevelEvent;
								startingLevelEvent.myLevelIndex = myLevelIndex;
								ES::EventSystem::GetInstance().SendEvent(startingLevelEvent);

								P8::ChangeGameState changeGameStateEvent(P8::eGameStates::eMenuLobby);
								ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);
							}
							else
							{
								P8::StartingLevelEvent startingLevelEvent;
								startingLevelEvent.myLevelIndex = -1;
								ES::EventSystem::GetInstance().SendEvent(startingLevelEvent);

								PopGUIScene();
								SelectFirstMenuElement();
							}
						}
						else if (myActiveGUIScenes.empty() && (interaction.myInputType == eInputType::Esc || interaction.myInputType == eInputType::XboxStart))
						{
							P8::ChangeGameState changeGameStateEvent(P8::eGameStates::eMenuPause);
							ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);
							PushGUIScene("Pause");
							SelectFirstMenuElement();
						}
					}
					else if (interaction.myInputType == eInputType::Up ||
						interaction.myInputType == eInputType::XboxLeftTriggerToggledUp ||
						interaction.myInputType == eInputType::XboxDPadUp)
					{
						SelectPreviousMenuElement();
					}
					else if (interaction.myInputType == eInputType::Down ||
						interaction.myInputType == eInputType::XboxLeftTriggerToggledDown ||
						interaction.myInputType == eInputType::XboxDPadDown)
					{
						SelectNextMenuElement();
					}
					else if (interaction.myInputType == eInputType::Enter ||
						interaction.myInputType == eInputType::XboxA)
					{
						if (GetCurrentSceneName() == "Levels")
						{
							if (interaction.myInputType == eInputType::Enter)
							{
								myActiveGUIScenes.clear();

								P8::StartingLevelEvent startingLevelEvent;
								startingLevelEvent.myLevelIndex = myLevelIndex;
								ES::EventSystem::GetInstance().SendEvent(startingLevelEvent);

								P8::ChangeGameState changeGameStateEvent(P8::eGameStates::eMenuLobby);
								ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);
							}
							else
							{
								P8::StartingLevelEvent startingLevelEvent;
								startingLevelEvent.myLevelIndex = -1;
								ES::EventSystem::GetInstance().SendEvent(startingLevelEvent);

								PopGUIScene();
								SelectFirstMenuElement();
							}
						}
						else if (GetCurrentSceneName() == "QuitConfirm")
						{
							PostQuitMessage(0);
							return;
						}
						else if (GetCurrentSceneName() == "MenuConfirm")
						{
							PopGUIScene();
							P8::ChangeGameState changeGameStateEvent(P8::eGameStates::eMenuMain);
							ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);
							return;
						}

						bool clickSelected = true;
						if (!myActiveGUIScenes.empty())
						{
							if (GetCurrentSceneName() == "Score")
							{
								KE::GlobalAudio::PlaySFX(sound::SFX::MenuSelect);

								bool hasSomeoneWon = false;
								for (int i = 0; i < 4; ++i)
								{
									if (myScore[i] >= myMaxScore)
									{
										hasSomeoneWon = true;
										break;
									}
								}
								if (hasSomeoneWon)
								{
									clickSelected = false;
									PopGUIScene();
									P8::ChangeGameState changeGameStateEvent(P8::eGameStates::eWinScreen);
									ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);
								}
								else
								{
									clickSelected = false;
									PopGUIScene();
									P8::ChangeGameState changeGameStateEvent(P8::eGameStates::eLoading);
									ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);
								}
							}
							else if (GetCurrentSceneName() == "Tutorial")
							{
								KE::GlobalAudio::PlaySFX(sound::SFX::MenuSelect);

								if (myInactiveGUIScenes.empty())
								{
									clickSelected = false;
									PopGUIScene();
									P8::ChangeGameState changeGameStateEvent(P8::eGameStates::ePlayCountdown);
									ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);
								}
								else
								{
									myInactiveGUIScenes.clear();

									PopGUIScene();
									PushGUIScene("Pause");
									return;
								}
							}
							else if (GetCurrentSceneName() == "WinScreen")
							{
								KE::GlobalAudio::PlaySFX(sound::SFX::MenuSelect);

								clickSelected = false;
								PopGUIScene();
								P8::ChangeGameState changeGameStateEvent(P8::eGameStates::eMenuMain);
								ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);
							}

						}

						if (mySelectedElement && clickSelected)
						{
							mySelectedElement->Click();
						}
					}
					else if (interaction.myInputType == eInputType::Left ||
						interaction.myInputType == eInputType::XboxLeftTriggerToggledLeft ||
						interaction.myInputType == eInputType::XboxDPadLeft)
					{
						// Do the left thing on this element
						if (mySelectedElement)
						{
							HandleMenuElement(true);
						}
					}
					else if (interaction.myInputType == eInputType::Right ||
						interaction.myInputType == eInputType::XboxLeftTriggerToggledRight ||
						interaction.myInputType == eInputType::XboxDPadRight)
					{
						// Do the right thing on this element
						if (mySelectedElement)
						{
							HandleMenuElement(false);
						}
					}
				}
			}

		}

		/// -----------------------------------------------------------------------------------------------
		/// GAME STATE EVENTS
		if (P8::GameStateHasChanged* event = dynamic_cast<P8::GameStateHasChanged*>(&aEvent))
		{
			if (event->newGameState == P8::eGameStates::eMenuMain)
			{
				myActiveGUIScenes.clear();
				PushGUIScene("MainMenu");
				isBlockingInput = false;
			}
			else if (event->newGameState == P8::eGameStates::eMenuLobby)
			{
				PushGUIScene("Lobby");
				isBlockingInputTimed = true;
			}
			else if (event->newGameState == P8::eGameStates::ePlayCountdown)
			{
				PushGUIScene("Countdown");

				GUIScene& scene = myGUISceneMap["Countdown"];
				for (GUIElement& element : scene.GetGUIElements())
				{
					if (element.myName == "Ready")
					{
						KE::GlobalAudio::PlaySFX(sound::SFX::AnnouncerReady);

						myFadeInElement = &myReadyFadeElement;
						SetTextAlpha(myFadeInElement->myElement, 0.0f);
						myFadeInElement->myFadeState = eFadeState::FadeIn;
						isCountdownActive = true;
						shouldCountDown = true;
					}
				}
			}
			else if (event->newGameState == P8::eGameStates::ePlayLiveGame)
			{
				//myActiveGUIScenes.clear();
			}
			else if (event->newGameState == P8::eGameStates::eDebugTesting)
			{
				myActiveGUIScenes.clear();
			}
			else if (event->newGameState == P8::eGameStates::ePlayPresentScores)
			{
				PushGUIScene("Score");
				isBlockingInputTimed = true;

				GUIScene* scene = myActiveGUIScenes.front();

				if (scene)
				{
					isScoreAnimating = true;

					// Get the continue text and button elements
					for (GUIElement& element : scene->GetGUIElements())
					{
						if (element.myName == "ContinueText")
						{
							element.shouldDraw = false;
						}
						if (element.myName == "ContinueA")
						{
							element.shouldDraw = false;
						}
					}
				}
			}
			else if (event->newGameState == P8::eGameStates::ePlayTutorial)
			{
				PushGUIScene("Tutorial");
				isBlockingInputTimed = true;
			}
			else if (event->newGameState == P8::eGameStates::eWinScreen)
			{
				PushGUIScene("WinScreen");
				isBlockingInputTimed = true;
			}
			else if (event->newGameState == P8::eGameStates::ePlayMatchEnd)
			{
				isBlockingInput = true;
			}

		}


		/// -----------------------------------------------------------------------------------------------
		/// LOBBY EVENTS
		if (P8::LobbyEvent* event = dynamic_cast<P8::LobbyEvent*>(&aEvent))
		{
			switch (event->myLobbyEvent)
			{
			case P8::eLobbyEvents::PlayerJoined:
			{
				std::string readyText = "P" + std::to_string(event->myPlayerIndex + 1) + "Ready";
				std::string joinText = "P" + std::to_string(event->myPlayerIndex + 1) + "Join";
				SetDrawElement(joinText, false);
				SetDrawElement(readyText, true);
				break;
			}
			case P8::eLobbyEvents::PlayerReady:
			{
				std::string readyText = "P" + std::to_string(event->myPlayerIndex + 1) + "Ready";
				SetDrawElement(readyText, false);
				break;
			}
			case P8::eLobbyEvents::PlayerNotReady:
			{
				std::string readyText = "P" + std::to_string(event->myPlayerIndex + 1) + "Ready";
				SetDrawElement(readyText, true);
				break;
			}
			case P8::eLobbyEvents::PlayerRemoved:
			{
				// Help here Anton!!!
				std::string readyText = "P" + std::to_string(event->myPlayerIndex + 1) + "Ready";
				std::string joinText = "P" + std::to_string(event->myPlayerIndex + 1) + "Join";
				SetDrawElement(joinText, true);
				SetDrawElement(readyText, false);

				P8::LobbyEventData lobbyData = *(P8::LobbyEventData*)event->myData;

				for (int i = 0; i < 4; ++i)
				{
					if (lobbyData.mySelectedCharactersPlayerIndices[i] == -1)
					{
						std::string characterText = "PickChar" + std::to_string(i + 1);
						SetDrawTextElement(characterText, "", false);
						characterText = "Character" + std::to_string(i + 1);
						SetColourOfElement(characterText, { 1.0f, 1.0f, 1.0f, 1.0f });
						SetDrawElement(characterText, false);

						std::string playerBgText = "Player" + std::to_string(i + 1) + "Bg";
						SetDrawElement(playerBgText, false);
						continue;
					}
					std::string characterText = "PickChar" + std::to_string(i + 1);
					std::string displayText = "P" + std::to_string(lobbyData.mySelectedCharactersPlayerIndices[i] + 1);
					SetDrawTextElement(characterText, displayText, true);

					characterText = "Character" + std::to_string(i + 1);
					SetColourOfElement(characterText, myCharacterColours[i]);
					SetDrawElement(characterText, true);

					std::string playerBgText = "Player" + std::to_string(i + 1) + "Bg";
					SetDrawElement(playerBgText, true);
				}

				break;
			}
			case P8::eLobbyEvents::CharacterSelection:
			{
				P8::LobbyEventData lobbyData = *(P8::LobbyEventData*)event->myData;

				for (int i = 0; i < 4; ++i)
				{
					if (lobbyData.mySelectedCharactersPlayerIndices[i] == -1)
					{
						std::string characterText = "PickChar" + std::to_string(i + 1);
						SetDrawTextElement(characterText, "", false);
						characterText = "Character" + std::to_string(i + 1);
						SetColourOfElement(characterText, { 1.0f, 1.0f, 1.0f, 1.0f });
						SetDrawElement(characterText, false);

						std::string playerBgText = "Player" + std::to_string(i + 1) + "Bg";
						SetDrawElement(playerBgText, false);
						continue;
					}
					std::string characterText = "PickChar" + std::to_string(i + 1);
					std::string displayText = "P" + std::to_string(lobbyData.mySelectedCharactersPlayerIndices[i] + 1);
					SetDrawTextElement(characterText, displayText, true);

					characterText = "Character" + std::to_string(i + 1);
					SetColourOfElement(characterText, myCharacterColours[i]);
					SetDrawElement(characterText, true);

					std::string playerBgText = "Player" + std::to_string(i + 1) + "Bg";
					SetDrawElement(playerBgText, true);
				}
				break;
			}
			case P8::eLobbyEvents::AllReady:
			{
				std::string beginText = "Begin";
				SetDrawTextElement(beginText, "", true);
				SetDrawElement("Start", true);
				//std::cout << "\nAll players are ready! -- GUIHandler";
				break;
			}
			case P8::eLobbyEvents::AllNotReady:
			{
				std::string beginText = "Begin";
				SetDrawTextElement(beginText, "", false);
				SetDrawElement("Start", false);
				break;
			}
			case P8::eLobbyEvents::GameStart:
			{
				for (int i = 0; i < 4; ++i)
				{
					// Reset join text and button, and begin text
					std::string joinText = "P" + std::to_string(i + 1) + "Join";
					SetDrawElement(joinText, true);
					std::string beginText = "Begin";
					SetDrawTextElement(beginText, "", false);
					SetDrawElement("Start", false);

					// Reset character portraits and text
					std::string characterText = "PickChar" + std::to_string(i + 1);
					SetDrawTextElement(characterText, "", false);
					characterText = "Character" + std::to_string(i + 1);
					SetColourOfElement(characterText, { 1.0f, 1.0f, 1.0f, 1.0f });
					SetDrawElement(characterText, false);

					std::string playerBgText = "Player" + std::to_string(i + 1) + "Bg";
					SetDrawElement(playerBgText, false);
				}

				PopGUIScene("Lobby");
				break;
			}
			}
		}

		/// -----------------------------------------------------------------------------------------------
		/// SCORE EVENTS
		if (P8::PresentScoreDataEvent* event = dynamic_cast<P8::PresentScoreDataEvent*>(&aEvent))
		{
			SetScore(event);
		}

		/// -----------------------------------------------------------------------------------------------
		/// LEVEL SELECT EVENTS
		//if (P8::LevelSelectDataEvent* event = dynamic_cast<P8::)
		if (P8::LevelSelectDataEvent* event = dynamic_cast<P8::LevelSelectDataEvent*>(&aEvent))
		{
			for (P8::LevelSelectData& data : event->myLevelData)
			{
				std::string levelName = data.myLevelName.substr(0, data.myLevelName.size() - 5);
				std::transform(levelName.begin(), levelName.end(), levelName.begin(), ::tolower);

				myLevelSelectData.push_back({ levelName, data.myLevelIndex });
			}

			if (!myLevelSelectData.empty())
			{
				myCurrentLevelSelectIndex = 0;
				myLevelIndex = myLevelSelectData[0].myLevelIndex;

				myCurrentLevelImagePath = myLevelImageBasePath + myLevelSelectData[myCurrentLevelSelectIndex].myLevelName + ".dds";
				myLevelIndex = myLevelSelectData[myCurrentLevelSelectIndex].myLevelIndex;
				myLevelSelectImage->mySpriteBatch.myData.myTexture = myGraphics->GetTextureLoader().GetTextureFromPath(myCurrentLevelImagePath);

				std::string uppercase = myLevelSelectData[0].myLevelName;
				std::transform(uppercase.begin(), uppercase.end(), uppercase.begin(), ::toupper);
				SetGUIText(myLevelSelectText, uppercase);
			}
		}

		/// -----------------------------------------------------------------------------------------------
		/// GUI ELEMENT EVENTS (GUI Buttons are clicked)		
		if (GUIElementEvent* event = dynamic_cast<GUIElementEvent*>(&aEvent))
		{
			if (event->myEventName == "Play")
			{
				PopGUIScene("MainMenu");

				// Reset scores when starting a new game
				ResetScores();

				P8::ChangeGameState changeGameStateEvent(P8::eGameStates::eMenuLobby);
				ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);
			}

			if (event->myEventName == "Quit")
			{
				if (GetCurrentSceneName() == "Pause")
				{
					PushGUIScene("QuitConfirm");
					isBlockingInputTimed = true;
				}
				else
				{
					PostQuitMessage(0);
				}
				return;
			}

			if (event->myEventName == "Options")
			{
				PushGUIScene("Options");
			}

			if (event->myEventName == "Credits")
			{
				PushGUIScene("Credits");
			}

			if (event->myEventName == "Levels")
			{
				PushGUIScene("Levels");
			}

			if (event->myEventName == "Resume")
			{
				PopGUIScene();
				P8::ChangeGameState changeGameStateEvent(P8::eGameStates::ePlayLiveGame);
				ES::EventSystem::GetInstance().SendEvent(changeGameStateEvent);
			}

			if (event->myEventName == "Menu")
			{
				PushGUIScene("MenuConfirm");
				isBlockingInputTimed = true;
			}

			if (event->myEventName == "Tutorial")
			{
				myInactiveGUIScenes.push_back(myActiveGUIScenes.front());
				PopGUIScene();
				PushGUIScene("Tutorial");
			}
		}

		/// -----------------------------------------------------------------------------------------------
		///	RESOLUTION CHANGE
		if (ResolutionEvent* event = dynamic_cast<ResolutionEvent*>(&aEvent))
		{
			std::cout << "GuiHandler Resolution Event Received with: " <<
				event->myWidth <<
				", " <<
				event->myHeight <<
				", " <<
				(event->myFullscreen ? "true" : "false") <<
				", GUI isFullscreen = " <<
				(isFullscreen ? "true" : "false") <<
				std::endl;

			if (event->myWidth > 0 && event->myHeight > 0)
			{
				// Iterate through map and update all scenes
				for (GUIScene& value : myGUISceneMap | std::views::values)
				{
					value.UpdateGUISize(event->myWidth, event->myHeight);

					for (GUIElement& element : value.GetGUIElements())
					{
						if (element.hasText)
						{
							SetGUIText(&element, element.myText);
						}
					}
				}
				isFullscreen = event->myFullscreen;
			}
		}
	}

	void GUIHandler::OnInit()
	{
		ES::EventSystem::GetInstance().Attach<GUIEvent>(this);
		ES::EventSystem::GetInstance().Attach<ResolutionEvent>(this);
		ES::EventSystem::GetInstance().Attach<InputEvent>(this);
		ES::EventSystem::GetInstance().Attach<GUIElementEvent>(this);
		ES::EventSystem::GetInstance().Attach<MouseMoveEvent>(this);
		ES::EventSystem::GetInstance().Attach<P8::GameStateHasChanged>(this);
		ES::EventSystem::GetInstance().Attach<P8::LobbyEvent>(this);
		ES::EventSystem::GetInstance().Attach<P8::PresentScoreDataEvent>(this);
		ES::EventSystem::GetInstance().Attach<P8::LevelSelectDataEvent>(this);
	}

	void GUIHandler::OnDestroy()
	{
		ES::EventSystem::GetInstance().Detach<GUIEvent>(this);
		ES::EventSystem::GetInstance().Detach<ResolutionEvent>(this);
		ES::EventSystem::GetInstance().Detach<InputEvent>(this);
		ES::EventSystem::GetInstance().Detach<GUIElementEvent>(this);
		ES::EventSystem::GetInstance().Detach<MouseMoveEvent>(this);
		ES::EventSystem::GetInstance().Detach<P8::GameStateHasChanged>(this);
		ES::EventSystem::GetInstance().Detach<P8::LobbyEvent>(this);
		ES::EventSystem::GetInstance().Detach<P8::PresentScoreDataEvent>(this);
		ES::EventSystem::GetInstance().Detach<P8::LevelSelectDataEvent>(this);
	}

	void GUIHandler::SetTextColour(GUIElement* aElement, const Vector4f& aColour)
	{
		for (int i = 1; i < (int)aElement->mySpriteBatch.myInstances.size(); ++i)
		{
			aElement->mySpriteBatch.myInstances[i].myAttributes.myColor = { aColour.x, aColour.y, aColour.z, aColour.w };
		}
	}

	void GUIHandler::SetDrawElement(const std::string& aEventName, bool aShouldDraw)
	{
		for (GUIElement* element : myEventMap[aEventName])
		{
			element->shouldDraw = aShouldDraw;
		}
	}
	void GUIHandler::SetDrawTextElement(const std::string& aEventName, const std::string& aText, bool aShouldDraw)
	{
		for (GUIElement* element : myEventMap[aEventName])
		{
			element->shouldDraw = aShouldDraw;

			std::string text = aText;

			if (text == "")
			{
				text = element->myText;
			}
			SetGUIText(element, text);
		}
	}

	void GUIHandler::SetTextAlpha(GUIElement* aElement, const float aAlpha)
	{
		for (int i = 1; i < (int)aElement->mySpriteBatch.myInstances.size(); ++i)
		{
			aElement->mySpriteBatch.myInstances[i].myAttributes.myColor.a = aAlpha;
		}
	}

	void GUIHandler::SetColourOfElement(const std::string& aEventName, const Vector4f& aColour)
	{
		for (GUIElement* element : myEventMap[aEventName])
		{
			element->mySpriteBatch.myInstances[0].myAttributes.myColor = { aColour.x, aColour.y, aColour.z, aColour.w };
		}
	}

	void GUIHandler::SetColourOfElement(GUIElement* aElement, const Vector4f& aColour)
	{
		aElement->mySpriteBatch.myInstances[0].myAttributes.myColor = { aColour.x, aColour.y, aColour.z, aColour.w };
	}

	void GUIHandler::SetTextureOfElement(const std::string& aEventName, const std::string& aTexturePath)
	{
		for (GUIElement* element : myEventMap[aEventName])
		{
			element->mySpriteBatch.myData.myTexture = myGraphics->GetTextureLoader().GetTextureFromPath(aTexturePath);
		}
	}

	void GUIHandler::SelectFirstMenuElement()
	{
		if (myActiveGUIScenes.empty())
		{
			return;
		}

		GUIScene* scene = myActiveGUIScenes.front();

		for (GUIElement& element : scene->GetGUIElements())
		{
			if (element.isButton)
			{
				mySelectedElement = nullptr;
				element.Reset();
			}
		}

		for (GUIElement& element : scene->GetGUIElements())
		{
			if (element.isButton)
			{
				mySelectedElement = &element;
				mySelectedElement->Highlight();
				return;
			}
		}
	}

	void GUIHandler::SelectLastMenuElement()
	{
		GUIScene* scene = myActiveGUIScenes.front();
		auto& elements = scene->GetGUIElements();

		for (int i = static_cast<int>(elements.size()) - 1; i >= 0; --i)
		{
			if (elements[i].isButton)
			{
				mySelectedElement = &elements[i];
				mySelectedElement->Highlight();
				return;
			}
		}
	}

	void GUIHandler::SelectNextMenuElement()
	{
		if (myActiveGUIScenes.empty()) { return; }
		if (mySelectedElement == nullptr)
		{
			SelectFirstMenuElement();
			return;
		}

		auto& elements = myActiveGUIScenes.front()->GetGUIElements();
		int selectedIndex = -1;

		for (int i = 0; i < elements.size(); ++i)
		{
			if (&elements[i] == mySelectedElement)
			{
				selectedIndex = i;
				break;
			}
		}

		if (selectedIndex < 0)
		{
			SelectFirstMenuElement();
			return;
		}

		mySelectedElement->Reset();

		for (int i = selectedIndex + 1; i < elements.size(); ++i)
		{
			if (elements[i].isButton)
			{
				mySelectedElement = &elements[i];
				mySelectedElement->Highlight();
				return;
			}
		}

		SelectFirstMenuElement();
	}

	void GUIHandler::SelectPreviousMenuElement()
	{
		if (myActiveGUIScenes.empty()) { return; }

		if (mySelectedElement == nullptr)
		{
			SelectLastMenuElement();
			return;
		}

		auto& elements = myActiveGUIScenes.front()->GetGUIElements();
		int selectedIndex = -1;

		for (int i = 0; i < elements.size(); ++i)
		{
			if (&elements[i] == mySelectedElement)
			{
				selectedIndex = i;
				break;
			}
		}

		if (selectedIndex < 0)
		{
			SelectLastMenuElement();
			return;
		}

		mySelectedElement->Reset();

		for (int i = selectedIndex - 1; i >= 0; --i)
		{
			if (elements[i].isButton)
			{
				mySelectedElement = &elements[i];
				mySelectedElement->Highlight();
				return;
			}
		}

		SelectLastMenuElement();
	}

	KE::eFadeState GUIHandler::FadeElement(KE::FadeElement* aFadeElement)
	{
		if (aFadeElement->myElement)
		{
			aFadeElement->myTimer += KE_GLOBAL::deltaTime;
			switch (aFadeElement->myFadeState)
			{
			case eFadeState::FadeIn:
			{
				if (aFadeElement->myTimer < aFadeElement->myFadeInTime)
				{
					aFadeElement->myElement->shouldDraw = true;
					SetTextAlpha(aFadeElement->myElement, aFadeElement->myTimer / aFadeElement->myFadeInTime);
					return eFadeState::FadeIn;
				}
				else
				{
					SetTextAlpha(aFadeElement->myElement, 1.0f);
					aFadeElement->myFadeState = eFadeState::Show;
					aFadeElement->myTimer = 0.0f;
					return eFadeState::FadeIn;
				}
				break;
			}
			case eFadeState::Show:
			{
				if (aFadeElement->myTimer < aFadeElement->myShowTime)
				{
					return eFadeState::Show;
				}
				else
				{
					aFadeElement->myFadeState = eFadeState::FadeOut;
					aFadeElement->myTimer = 0.0f;
					return eFadeState::Show;
				}
				break;
			}
			case eFadeState::FadeOut:
			{
				if (aFadeElement->myTimer < aFadeElement->myFadeOutTime)
				{
					SetTextAlpha(aFadeElement->myElement, 1.0f - (aFadeElement->myTimer / aFadeElement->myFadeOutTime));
					return eFadeState::FadeOut;
				}
				else
				{
					SetTextAlpha(aFadeElement->myElement, 0.0f);
					aFadeElement->myElement->shouldDraw = false;
					aFadeElement->myFadeState = eFadeState::Done;
					aFadeElement->myTimer = 0.0f;
					return eFadeState::Done;
				}
				break;
			}
			}

		}
		return eFadeState::None;
	}


}
