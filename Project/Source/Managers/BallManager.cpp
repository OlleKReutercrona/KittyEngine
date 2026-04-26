#include "stdafx.h"
#include "BallManager.h"

#include "Boomerang/BoomerangComponent.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/LevelImporter/PrefabHandler.h"
#include "Engine/Source/SceneManagement/Scene.h"
#include <GameEvents\GameEvents.h>

P8::BallManager::BallManager(KE::GameObject& aParentGameObject) : KE::Component(aParentGameObject)
{
	OnInit();
}

P8::BallManager::~BallManager()
{
	OnDestroy();
}

void P8::BallManager::AddBall(BoomerangComponent* aBall)
{
	aBall->SetBallManager(this);
	myBalls.push_back(aBall);
}

void P8::BallManager::Reset()
{
	myPlayers.clear();
	myBalls.clear();
}

void P8::BallManager::OnSceneChange()
{
	Reset();
}

void P8::BallManager::OnReceiveEvent(ES::Event& aEvent)
{
	if (GameStateHasChanged* gameStateEvent = dynamic_cast<GameStateHasChanged*>(&aEvent))
	{
		if (gameStateEvent->newGameState == eGameStates::eMenuMain)
		{
			Reset();
		}
	}
}

void P8::BallManager::OnInit()
{
	ES::EventSystem::GetInstance().Attach<GameStateHasChanged>(this);
}

void P8::BallManager::OnDestroy()
{
	ES::EventSystem::GetInstance().Detach<GameStateHasChanged>(this);
}

P8::BoomerangComponent* P8::BallManager::GetUnusedBall()
{
	for (auto& ball : myBalls)
	{
		if (ball->IsOriginal()) { continue; }
		if (ball->IsFromPowerup()) { continue; }
		if (!ball->GetGameObject().IsActive())
		{
			return ball;
		}
	}

	auto* ball = KE::Scene::myPrefabHandler->Instantiate("Ball");
	if (ball)
	{
		auto& comp = ball->GetComponent<BoomerangComponent>();
		AddBall(&comp);
		comp.GetGameObject().SetActive(false);
		return &comp;
	}

	return nullptr;
}

void P8::BallManager::RegisterPlayer(Player* aPlayer)
{
	myPlayers.push_back(aPlayer);
}
