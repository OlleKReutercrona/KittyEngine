#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Utility/EventSystem.h"

namespace P8
{
	class Player;

	class PowerupPickupComponent : public KE::Component, public ES::IObserver
	{
	public:
		PowerupPickupComponent(KE::GameObject& aParentGameObject) : Component(aParentGameObject)
		{
		}

		~PowerupPickupComponent() override;

		void SetData(void* aDataObject) override;

		std::vector<Player*> players;

	protected:
		void Awake() override;
		void Update() override;
		void OnEnable() override;
		void OnDisable() override;
		void DrawDebug(KE::DebugRenderer& aDbg) override;

		// Inherited via IObserver
		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		void OnDestroy() override;
	};
}
