#pragma once
#include "Editor/Source/EditorGraphics.h"
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Project/Source/Player/Player.h"

namespace P8
{
	struct BattleAreaData
	{
		float shrinkDuration = 15.0f;
		float shrinkDelay = 15.0f;
		bool shouldShrink = false;
	};

	class BattleArea : public KE::Component, public ES::IObserver
	{
		struct UserBoundsData
		{
			P8::Player* player = nullptr;
			float gamepadRumbleTimer = 0.0f;
			//bool isDead = false;
		};

	public:
		BattleArea(KE::GameObject& aGo);
		~BattleArea();

		void Awake() override;

		void SetData(void* aDataObject = nullptr) override;
		void Update() override;
		void DrawDebug(KE::DebugRenderer& aDbg) override;
		bool IsOutsideArea(const Vector3f& aPosition) const;
		void CollectPlayers();
		void ShrinkArea();

	protected:
		void OnEnable() override;
		void OnDisable() override;
		void OnDestroy() override;

		// Inherited via IObserver
		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		//void OnDestroy() override;

	private:

		float myWidth = 0.0f;
		float myDepth = 0.0f;
		float myCurrentWidth = 0.0f;
		float myCurrentDepth = 0.0f;

		float myShrinkTimer = 0.0f;
		float myShrinkDuration = 15.0f;
		float myStartShrinkTime = 15.0f;

		float myKillThreshold = 3.0f;
		bool myPlayersCollected = false;
		bool myShouldShrink = false;

		std::vector<UserBoundsData> myBattleUnits;

		KE::VFXPlayerInterface myEffectInterface;
	};
}