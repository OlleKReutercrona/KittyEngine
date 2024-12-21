#include "stdafx.h"
#include "Powerup.h"

#include "DashThroughWallsPowerup.h"
#include "DoubleBallPowerup.h"
#include "JuggernautPowerup.h"
#include "ShieldPowerup.h"
#include "SpeedPowerup.h"
#include "SplitBoomerangPowerup.h"
#include "TelekinesisPowerup.h"
#include "TeleportPowerup.h"
#include "ExplosionPowerup.h"
#include "PowerupAreaComponent.h"
#include "PowerupPickupComponent.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/CameraComponent.h"
#include "Engine/Source/Graphics/FX/VFX.h"
#include "Engine/Source/LevelImporter/PrefabHandler.h"
#include "Engine/Source/SceneManagement/Scene.h"
#include "Engine/Source/Utility/Randomizer.h"
#include "Managers/GameManager.h"
#include "Player/Player.h"

namespace P8
{
#pragma region Powerup

	Powerup::Powerup()
	{
		vfxInterface = new KE::VFXPlayerInterface();
		vfxInterface->manager = KE_GLOBAL::blackboard.Get<KE::VFXManager>();
	}

	Powerup::~Powerup()
	{
		delete vfxInterface;
	}

	void Powerup::SetVFX(std::vector<std::string> aVFXNames)
	{
		for (auto& vfxName : aVFXNames)
		{
			vfxInterface->AddVFX(vfxName);
		}
	}

	void Powerup::SetIdentity(PowerupIdentityBase* anIdentity)
	{
		identity = anIdentity;
	}

#pragma endregion

#pragma region PowerupManager

	PowerupManager::PowerupManager(KE::GameObject& aGameObject) : Component(aGameObject)
	{
		const std::string powerupIconPath = "Data/Assets/PowerupIcons/";

		//
		RegisterPowerup(PowerupIdentity<TeleportPowerup>(
			"Teleport",
			powerupIconPath+"teleport.dds",
			"Dashing teleports you to your thrown boomerang instead.",
			{
				"TeleportPowerupTrigger"
			}
		));

		//
		RegisterPowerup(PowerupIdentity<DashThroughWallsPowerup>(
			"Dash Through Walls",
			powerupIconPath+"dashThroughWalls.dds",
			"Ignore walls while dashing.",
			{}
		));

		//
		RegisterPowerup(PowerupIdentity<SplitBoomerangPowerup>(
			"Split Throw",
			powerupIconPath+"split.dds",
			"After flying for a moment, thrown boomerangs split.",
			{
				"SplitBoomerangPowerupTrigger"
			}
		));

		//
		RegisterPowerup(PowerupIdentity<ShieldPowerup>(
			"Shield",
			powerupIconPath+"shield.dds",
			"Blocks one hit of damage.",
			{
				"ShieldPowerup",
				"ShieldPowerupBreak"
			}
		));

		//
		RegisterPowerup(PowerupIdentity<DoubleBallPowerup>(
			"Double Trouble",
			powerupIconPath+"doubleTrouble.dds",
			"Gain an extra ball.",
			{}
		));

		//
		//RegisterPowerup(PowerupIdentity<JuggernautPowerup>(
		//	"Juggernaut",
		//	powerupIconPath+"powerup10icon.dds",
		//	"You can no longer throw. Gain increased attack speed.",
		//	{
		//		"JuggernautPowerup"
		//	}
		//));

		//
		RegisterPowerup(PowerupIdentity<SpeedPowerup>(
			"Hyped Up",
			powerupIconPath+"hypedUp.dds",
			"Gain increased move speed.",
			{
				"SpeedPowerup"
			}
		));

		//
		RegisterPowerup(PowerupIdentity<TelekinesisPowerup>(
			"Telekinesis",
			powerupIconPath+"telekinesis.dds",
			"While recalling, move to control thrown balls.",
			{
				"TelekinesisPowerup"
			}
		));

		//
		RegisterPowerup(PowerupIdentity<ExplosionBoomerangPowerup>(
			"Explosion",
			powerupIconPath + "explosion.dds",
			"TEMP",
			{
				"ExplosionPowerup"
			}
		));

		//

		powerupDisplayFont = KE::FontLoader::LoadFont("Data/InternalAssets/motley.ktf");
	}

	PowerupManager::~PowerupManager()
	{
		for (auto* powerupID : myPowerupIdentities)
		{
			delete powerupID;
		}
	}

	Vector3f PowerupManager::GetPowerupSpawnLocation()
	{
		if (myPowerupSpawnBounds.empty()) { return {0.0f, 0.0f, 0.0f}; }

		float totalSquareArea = 0.0f;
		for (auto& area : myPowerupSpawnBounds)
		{
			float xSize = area.max.x - area.min.x;
			float ySize = area.max.y - area.min.y;
			totalSquareArea += xSize * ySize;
		}

		std::vector<std::pair<float,float>> areaWeights;

		for (auto& area : myPowerupSpawnBounds)
		{
			float xSize = area.max.x - area.min.x;
			float ySize = area.max.y - area.min.y;
			float squareSize = xSize * ySize;

			float weight = squareSize / totalSquareArea;
			float lowerBounds = areaWeights.empty() ? 0.0f : areaWeights.back().second;

			areaWeights.push_back({lowerBounds, lowerBounds + weight });
		}

		float randomValue = KE::GetRandomUniformFloat(0.0f, 1.0f);

		for (int i = 0; i < areaWeights.size(); i++)
		{
			if (randomValue >= areaWeights[i].first && randomValue < areaWeights[i].second)
			{
				float x = KE::GetRandomUniformFloat(myPowerupSpawnBounds[i].min.x, myPowerupSpawnBounds[i].max.x);
				float y = 0.0f;
				float z = KE::GetRandomUniformFloat(myPowerupSpawnBounds[i].min.z, myPowerupSpawnBounds[i].max.z);

				return { x, y, z };
			}
		}

		return { 0.0f, 0.0f, 0.0f };
	}

	void PowerupManager::SpawnPowerup(const Vector3f& aSpawnPosition)
	{
		if (myPowerupIdentities.empty()) { return; }
		if (myPowerupSpawnBounds.empty()) { return; }

		Vector3f positionToUse = aSpawnPosition.y < 0 ? GetPowerupSpawnLocation() : aSpawnPosition;

		auto* obj = KE::Scene::myPrefabHandler->Instantiate("PowerupPickup");
		obj->myTransform.SetPosition(positionToUse);
	}

	Powerup* PowerupManager::GetRandomPowerup(int aPowerupMask)
	{
		std::vector<PowerupIdentityBase*> unmaskedPowerups = {};

		for (int i = 0; i < myPowerupIdentities.size(); i++)
		{
			if ((aPowerupMask & (1 << i)) == 0)
			{
				unmaskedPowerups.push_back(myPowerupIdentities[i]);
			}
		}

		if (unmaskedPowerups.size() == 0) { return nullptr; } //this should never happen, but just in case

		int randomIndex = KE::GetRandomUniformInt(0, static_cast<int>(unmaskedPowerups.size()) - 1);

		return unmaskedPowerups[randomIndex]->CreateInstance();
	}

	void PowerupManager::SetupDisplay(PowerupPickupDisplay& aDisplay)
	{
		aDisplay.powerupIconBatch.myInstances.resize(1);

		aDisplay.powerupTextBatch.myInstances.resize(24);
		aDisplay.powerupTextBatch.myData.myTexture = powerupDisplayFont.GetFontAtlas();
		aDisplay.powerupTextBatch.myData.myMode = KE::SpriteBatchMode::ScreenText;
		aDisplay.powerupTextBatch.myTextStyling.text.horizontalAlign = KE::TextAlign::LeftOrTop;
		aDisplay.powerupTextBatch.myTextStyling.text.colour = {1.0f, 1.0f, 1.0f, 1.0f};
		aDisplay.powerupTextBatch.myCustomPS = KE_GLOBAL::blackboard.Get<KE::Graphics>()->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH "SDFRendering_PS.cso");
		aDisplay.text = "";
		aDisplay.displayTimer = 0.0f;

		//
		aDisplay.powerupIconBatch.myData.myMode = KE::SpriteBatchMode::Screen;

	}

	PowerupPickupDisplay* PowerupManager::GetFreeDisplay()
	{
		//find an unused PowerupPickupDisplay
		PowerupPickupDisplay* display = nullptr;
		for (auto& pickupDisplay : myPowerupPickupDisplays)
		{
			if (pickupDisplay.displayTimer <= 0.0f)
			{
				display = &pickupDisplay;
				break;
			}
		}
		if (!display)
		{
			myPowerupPickupDisplays.push_back({});
			display = &myPowerupPickupDisplays.back();
			SetupDisplay(*display);
		}
		return display;
	}

	void PowerupManager::DisplayPickup(Powerup* aPowerup, Transform* aTransform)
	{
		PowerupPickupDisplay* display = GetFreeDisplay();
		display->displayTimer = 2.0f;
		display->text = std::format("{}", aPowerup->GetName());
		display->transform = aTransform;
		display->myType = PowerupPickupDisplay::Type::Pickup;


		display->textSpriteVec.resize(display->text.size());
		for (int i = 0; i < display->text.size(); i++)
		{
			display->textSpriteVec[i] = &display->powerupTextBatch.myInstances[i];
			display->textSpriteVec[i]->myAttributes.myColor.a = 1.0f;
		}

		static KE::Graphics* graphics = KE_GLOBAL::blackboard.Get<KE::Graphics>();
		static KE::TextureLoader* textureLoader = &graphics->GetTextureLoader();

		display->powerupIconBatch.myData.myTexture = textureLoader->GetTextureFromPath(aPowerup->GetIdentity()->icon);
	}

	void PowerupManager::DisplayDrop(Powerup* aPowerup, Transform* aTransform)
	{

	}

	void PowerupManager::DisplayOwnedPowerups(PowerupList* aPowerupList)
	{
		Transform* t = &aPowerupList->ownerPlayer->GetGameObject().myWorldSpaceTransform;
		KE::Camera* c = myGameObject.GetManager().GetGameObject(KE::ReservedGameObjects::eMainCamera)->GetComponent<KE::CameraComponent>().GetCamera();
		Vector2f screenPosition;
		c->WorldToScreenPoint(t->GetPosition(), screenPosition);

		int powerupCount = static_cast<int>(aPowerupList->powerups.size());

		static KE::Graphics* graphics = KE_GLOBAL::blackboard.Get<KE::Graphics>();
		static KE::TextureLoader* textureLoader = &graphics->GetTextureLoader();

		int i = 0;
		for (auto& powerup : aPowerupList->powerups)
		{
			PowerupPickupDisplay* display = GetFreeDisplay();
			display->displayTimer = 2.0f;

			display->myType = PowerupPickupDisplay::Type::Owned;

			display->powerupIconBatch.myData.myTexture = textureLoader->GetTextureFromPath(powerup->GetIdentity()->icon);
			display->ownedPowerupDisplayCount = powerupCount;
			display->ownedPowerupDisplayIndex = i;
			display->transform = t;
			i++;
		}
	}

	void PowerupManager::UpdateDisplays()
	{
		KE::Camera* c = myGameObject.GetManager().GetGameObject(KE::ReservedGameObjects::eMainCamera)->GetComponent<KE::CameraComponent>().GetCamera();
		std::unordered_map<Transform*, Vector3f> screenPositions;

		static KE::Graphics* graphics = KE_GLOBAL::blackboard.Get<KE::Graphics>();
		static KE::SpriteManager* spriteManager = &graphics->GetSpriteManager();

		for (auto& display : myPowerupPickupDisplays)
		{
			if (display.displayTimer <= 0.0f) { continue; }
			if (!display.transform) { continue; }

			display.displayTimer -= KE_GLOBAL::deltaTime;
			if (screenPositions.find(display.transform) == screenPositions.end())
			{
				Vector2f screenPos;
				c->WorldToScreenPoint(display.transform->GetPosition(), screenPos);
				screenPositions[display.transform] = { screenPos.x, screenPos.y, 0.0f };
			}

			switch (display.myType)
			{
			case PowerupPickupDisplay::Type::Pickup:
			{

				const float alpha = display.displayTimer >= 1.75f ? 
				(2.0f - display.displayTimer) / 0.25f :
				display.displayTimer <= 1.0f ? 
					display.displayTimer :
					1.0f;

				for (int i = 0; i < display.powerupTextBatch.myInstances.size(); i++)
				{

					display.powerupTextBatch.myInstances[i].myAttributes.myColor.a = i >= display.textSpriteVec.size() ? 0.0f : alpha;
				}

				for (int i = 0; i < display.powerupIconBatch.myInstances.size(); i++)
				{
					display.powerupIconBatch.myInstances[i].myAttributes.myColor.a = alpha;
				}

				float h = c->GetProjectionData().perspective.height;
		

				const float textScale = 30.0f;
				const float iconScale = 48.0f;
				const float textXOffset = 110.0f;
				const float iconXOffset = 48.0f;

				const float timerYOffset = display.displayTimer >= 1.75f ?
					std::pow((1.0f - ((2.0f - display.displayTimer) / 0.25f)), 2.0f) * 16.0f :
					display.displayTimer > 1.0f ? 
						0.0f :
						-64.0f * (1.0f - display.displayTimer);

				Transform t;
				Vector3f tPos = screenPositions[display.transform];
				tPos.x += textXOffset;
				tPos.y = h - (tPos.y + timerYOffset);

				t.SetPosition(tPos);
				t.SetScale({textScale, textScale, 1.0f});
				Transform iconT;


				iconT.SetPosition(screenPositions[display.transform] + Vector3f(iconXOffset, -(iconScale - textScale/3.0f) + timerYOffset, 0.0f));
				iconT.SetScale({iconScale, iconScale, 1.0f});

				display.powerupTextBatch.myTextStyling.text.colour.w = alpha;
				powerupDisplayFont.PrepareSprites(display.textSpriteVec.data(), display.text, display.powerupTextBatch.myTextStyling, t);

				spriteManager->QueueSpriteBatch(&display.powerupTextBatch);

				display.powerupIconBatch.myInstances[0].myAttributes.myTransform = iconT;
				spriteManager->QueueSpriteBatch(&display.powerupIconBatch);

				break;
			}
			case PowerupPickupDisplay::Type::Owned:
			{
				Vector3f screenPosition = screenPositions[display.transform];

				float iconSize = 48.0f;
				const float iconSpacing = 8.0f;

				Vector3f iconPosStart = {
					screenPosition.x - ((iconSize + iconSpacing) * display.ownedPowerupDisplayCount) / 2.0f,
					screenPosition.y,
					0.0f
				};
				iconPosStart.x += iconSpacing / 2.0f;

				Vector3f iconPos = iconPosStart + Vector3f(
					(iconSize + iconSpacing) * display.ownedPowerupDisplayIndex,
					16.0f, 
					0.0f
				);

				display.powerupIconBatch.myInstances[0].myAttributes.myTransform.SetPosition(iconPos);
				display.powerupIconBatch.myInstances[0].myAttributes.myTransform.SetScale({iconSize, iconSize, 1.0f});
				display.powerupIconBatch.myInstances[0].myAttributes.myColor.a = 1.0f;
				spriteManager->QueueSpriteBatch(&display.powerupIconBatch);


				break;
			}
			}
		}
	}

	void PowerupManager::Awake()
	{
		myPowerupSpawnBounds.clear();
		for (auto* object : myGameObject.GetManager().GetGameObjectsWithComponent<PowerupAreaComponent>())
		{
			auto& powerupArea = object->GetComponent<PowerupAreaComponent>();

			myPowerupSpawnBounds.push_back({ powerupArea.GetMin(), powerupArea.GetMax() });
		}
	}

	void PowerupManager::Update()
	{
		if (P8::GameManager::IsPaused())
		{
			return;
		}

		//if (GetAsyncKeyState('V') == SHRT_MIN + 1)
		//{
		//	powerupSpawnTimer = powerupSpawnInterval;
		//
		//}
			
		powerupSpawnTimer += KE_GLOBAL::deltaTime;
		if (powerupSpawnTimer >= powerupSpawnInterval)
		{
			powerupSpawnTimer = 0.0f;

			if(!myGameObject.GetManager().GetGameObjectWithComponent<PowerupPickupComponent>())
			{
				SpawnPowerup();
			}
		}
	}

	void PowerupManager::LateUpdate()
	{
		UpdateDisplays();
	}

#pragma endregion

#pragma region PowerupList

	//void PowerupList::UpdateBatches()
	//{
	//	for (int i = 0; i < 4; i++)
	//	{
	//		if (powerupTextBatchTimers[i] > 0.0f)
	//		{
	//			powerupTextBatchTimers[i] -= KE_GLOBAL::deltaTime;
	//
	//			Transform t = {};
	//			auto* camObj = ownerPlayer->GetGameObject().GetManager().GetGameObject(KE::ReservedGameObjects::eMainCamera);
	//			KE::Camera* c = camObj->GetComponent<KE::CameraComponent>().GetCamera();
	//
	//			Vector2f screenPos;
	//			c->WorldToScreenPoint(ownerPlayer->GetGameObject().myWorldSpaceTransform.GetPosition(), screenPos);
	//
	//			const float size = 24.0f;
	//			const float yOffset = 36.0f + size * 1.5f * ((float)i + (2.5f - powerupTextBatchTimers[i]));
	//			const float xOffset = 64.0f;
	//
	//			t.SetPosition({ screenPos.x + xOffset,  c->GetProjectionData().perspective.height - (screenPos.y - yOffset), 0.0f });
	//			t.SetScale({ size,size,size });
	//
	//			powerupFont.PrepareSprites(spriteVecs[i].data(), powerupTexts[i], {}, t);
	//
	//			//when .5 is reached, start fading out
	//			const float alpha = powerupTextBatchTimers[i] <= 0.5f ? powerupTextBatchTimers[i] / 0.5f : 1.0f;
	//
	//			powerupTextBatches[i].myTextStyling.text.colour.w = alpha;
	//			for (int j = 0; j < powerupTextBatches[i].myInstances.size(); j++)
	//			{
	//				powerupTextBatches[i].myInstances[j].myAttributes.myColor.a = j >= spriteVecs[i].size() ? 0.0f : alpha;
	//			}
	//
	//		}
	//	}
	//}
	//
	//void PowerupList::InitBatches()
	//{
	//	char fontPath[64] = "motley";
	//	powerupFont = KE::FontLoader::LoadFont(std::format("Data/InternalAssets/{}.ktf", fontPath).c_str());
	//	for (int i = 0; i < 4; i++)
	//	{
	//		powerupTextBatchTimers[i] = 0.0f;
	//		powerupTextBatches[i].myInstances.resize(32);
	//		powerupTextBatches[i].myData.myTexture = powerupFont.GetFontAtlas();
	//		powerupTextBatches[i].myData.myMode = KE::SpriteBatchMode::ScreenText;
	//
	//		powerupTextBatches[i].myTextStyling.text.horizontalAlign = KE::TextAlign::Center;
	//
	//		auto* textShader = KE_GLOBAL::blackboard.Get<KE::Graphics>()->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH "SDFRendering_PS.cso");
	//		powerupTextBatches[i].myCustomPS = textShader;
	//		powerupTexts[i] = "";
	//	}
	//}
	//
	//void PowerupList::PrintPowerupText(const std::string& aText)
	//{
	//	for (int i = 0; i < 4; i++)
	//		{
	//			if (powerupTextBatchTimers[i] <= 0.0f)
	//			{
	//				powerupTextBatchTimers[i] = 2.5f;
	//
	//				spriteVecs[i].clear();
	//				spriteVecs[i].resize(aText.size());
	//				for (int j = 0; j < aText.size(); j++)
	//				{
	//					spriteVecs[i][j] = &powerupTextBatches[i].myInstances[j];
	//
	//					if (aText[0] == '+')
	//					{
	//						//rgb(172, 211, 142)
	//						spriteVecs[i][j]->myAttributes.myColor.r = 172.0f / 255.0f;
	//						spriteVecs[i][j]->myAttributes.myColor.g = 211.0f / 255.0f;
	//						spriteVecs[i][j]->myAttributes.myColor.b = 142.0f / 255.0f;
	//
	//					}
	//					else if (aText[0] == '-')
	//					{
	//						//rgb(217, 88, 79)
	//						spriteVecs[i][j]->myAttributes.myColor.r = 217.0f / 255.0f;
	//						spriteVecs[i][j]->myAttributes.myColor.g = 88.0f / 255.0f;
	//						spriteVecs[i][j]->myAttributes.myColor.b = 79.0f / 255.0f;
	//					}
	//
	//					spriteVecs[i][j]->myAttributes.myColor.a = 1.0f;
	//				}
	//				powerupTexts[i] = aText;
	//
	//				break;
	//			}
	//		}
	//}

	void PowerupList::AddPowerup(Powerup* aPowerup, bool aIgnorePowerupLimit, bool aDisplayPopup)
	{
		if (powerups.size() >= maxPowerups && !aIgnorePowerupLimit)
		{
			int randomPowerupIndex = KE::GetRandomUniformInt(0, maxPowerups - 1);
			RemovePowerup(powerups[randomPowerupIndex]);
		}
		powerups.push_back(aPowerup);
		aPowerup->SetPlayer(ownerPlayer);
		aPowerup->OnPickup();

		if (aDisplayPopup)
		{
			manager->DisplayPickup(aPowerup, &ownerPlayer->GetGameObject().myWorldSpaceTransform);
		}

		//std::string powerupText = std::format("+ {}", aPowerup->GetName());
		//PrintPowerupText(powerupText);
	}

	void PowerupList::RemovePowerup(Powerup* aPowerup)
	{

		auto it = std::find(powerups.begin(), powerups.end(), aPowerup);
		if (it != powerups.end())
		{
			//std::string powerupText = std::format("- {}", (*it)->GetName());
			//PrintPowerupText(powerupText);

			manager->DisplayDrop(*it, &ownerPlayer->GetGameObject().myWorldSpaceTransform);

			(*it)->OnDrop();
			delete *it;
			powerups.erase(it);
		}
	}

	void PowerupList::ClearPowerups()
	{
		for (auto& powerup : powerups)
		{
			RemovePowerup(powerup);
		}
		powerups.clear();
	}

	bool PowerupList::OnPlayerAction(const PlayerActionType& anAction, const ActionState& aState,
		PowerupInputData& anInput)
	{
		bool success = false;
		for (auto& powerup : powerups)
		{
			if (powerup->OnPlayerAction(anAction, aState, anInput))
			{
				success = true;						
			}
		}
		return success;
	}

	bool PowerupList::OnBoomerangAction(const BoomerangAction& anAction, const ActionState& aState,
		PowerupInputData& anInput)
	{
		bool success = false;
		for (auto& powerup : powerups)
		{
			if (powerup->OnBoomerangAction(anAction, aState, anInput))
			{
				success = true;						
			}
		}
		return success;
	}

	void PowerupList::Update(PowerupInputData aInput)
	{
		for (auto& powerup : powerups)
		{
			powerup->Update(aInput);
		}
	}

	int PowerupList::GetPowerupMask()
	{
		int mask = 0;
		if (!manager)
		{
			KE_ERROR("PowerupManager not set! Returning default mask.");
			return mask;
		}

		const auto& identities = manager->GetPowerupIdentities();
		for (int i = 0; i < identities.size(); i++)
		{
			for (int j = 0; j < powerups.size(); j++)
			{
				if (identities[i]->IsType(powerups[j]))
				{
					mask |= (1 << i);
					break;
				}
			}
		}
		return mask;
	}

	void PowerupList::SetPowerupsFromMask(int mask, bool aDisplayOwned)
	{
		if (!manager)
		{
			KE_ERROR("PowerupManager not set! Aborting mask set.");
			return;
		}
		const auto& identities = manager->GetPowerupIdentities();

		ClearPowerups();

		for (int i = 0; i < identities.size(); i++)
		{
			if (mask & 1 << i)
			{
				AddPowerup(identities[i]->CreateInstance(), true, false);
			}
		}
		if (aDisplayOwned)
		{
			manager->DisplayOwnedPowerups(this);
		}
	}

#pragma endregion
}

