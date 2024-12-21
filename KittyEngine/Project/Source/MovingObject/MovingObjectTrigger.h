#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"

namespace P8
{
	class MovingObject;

	struct MovingObjectTriggerData
	{
		std::vector<int> movingObjects;
	};

	class MovingObjectTrigger : public KE::Component
	{
	public:
		MovingObjectTrigger(KE::GameObject& aGo);
		~MovingObjectTrigger();

		void Awake() override;
		void Update() override;
		void SetData(void* aData = nullptr) override;

		void OnPhysXCollision(const KE::PhysXCollisionData& aPhysXCollisionData);

	private:
		std::vector<int> myTriggerObjectIDs;
		std::vector<MovingObject*> myTriggerObjects;
	};
}