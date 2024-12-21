#include "stdafx.h"

#include "PowerupPickupComponent.h"

#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/GameObjectManager.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/VFXComponent.h"
#include "Engine/Source/Graphics/DebugRenderer.h"
#include "Player/Player.h"
#include "Engine/Source/Audio/GlobalAudio.h"

P8::PowerupPickupComponent::~PowerupPickupComponent()
{
	OnDestroy();
}

void P8::PowerupPickupComponent::SetData(void* aDataObject)
{
	
}

void P8::PowerupPickupComponent::Awake()
{
	for (const auto& playerObj : myGameObject.GetManager().GetGameObjectsWithComponent<Player>())
	{
		players.push_back(&playerObj->GetComponent<Player>());
	}

	auto& vfx = myGameObject.GetComponent<KE::VFXComponent>();
	vfx.TriggerVFX(0, true, false);

	OnInit();
}

void P8::PowerupPickupComponent::Update()
{
	for (auto& player : players)
	{
		if (player == nullptr)
		{
			continue;
		}
		Vector3f playerHorizontalPos = player->GetGameObject().myWorldSpaceTransform.GetPosition();
		playerHorizontalPos.y = 0.0f;

		Vector3f selfHorizontalPos = myGameObject.myWorldSpaceTransform.GetPosition();
		selfHorizontalPos.y = 0.0f;

		float dist = (playerHorizontalPos - selfHorizontalPos).LengthSqr();

		if (dist < 1.0f)
		{


			auto& powerupList = player->GetPowerups();
			int powerupMask = powerupList.GetPowerupMask();
			Powerup* freePowerup = powerupList.manager->GetRandomPowerup(powerupMask);
			if (freePowerup)
			{
				powerupList.AddPowerup(freePowerup);
			}

			auto& vfx = myGameObject.GetComponent<KE::VFXComponent>();
			vfx.StopAllVFX();

			KE::GlobalAudio::PlaySFX(sound::SFX::PowerupPickup);

			myGameObject.GetManager().DestroyGameObject(myGameObject.myID);
			break;
		}
	}
}

void P8::PowerupPickupComponent::OnEnable()
{

}

void P8::PowerupPickupComponent::OnDisable()
{

}

void P8::PowerupPickupComponent::DrawDebug(KE::DebugRenderer& aDbg)
{

}

void P8::PowerupPickupComponent::OnReceiveEvent(ES::Event& aEvent)
{
	if (auto* gameStateEvent = dynamic_cast<GameStateHasChanged*>(&aEvent))
	{
		if (gameStateEvent->newGameState == eGameStates::eMenuMain)
		{
			players.clear();
		}
	}
}

void P8::PowerupPickupComponent::OnInit()
{
	ES::EventSystem::GetInstance().Attach<GameStateHasChanged>(this);
}

void P8::PowerupPickupComponent::OnDestroy()
{
	ES::EventSystem::GetInstance().Detach<GameStateHasChanged>(this);
}
