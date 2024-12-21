#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Utility/EventSystem.h"

namespace P8
{
	class Player;
	class BoomerangComponent;

	class BallManager : public KE::Component, public ES::IObserver
	{
	private:
		std::vector<BoomerangComponent*> myBalls;
		std::vector<Player*> myPlayers;

	public:
		BallManager(KE::GameObject& aParentGameObject);
		~BallManager() override;

		BoomerangComponent* GetUnusedBall();

		void RegisterPlayer(Player* aPlayer);

		std::vector<Player*>& GetPlayers() { return myPlayers; }
		std::vector<BoomerangComponent*>& GetBalls() { return myBalls; }

	protected:
		void AddBall(BoomerangComponent* aBall);

		void LateUpdate() override {};
		void EarlyUpdate() override {};
		void Update() override {};

		void Reset();
		void OnSceneChange() override;

		// Inherited via IObserver
		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		void OnDestroy() override;
	};


}
