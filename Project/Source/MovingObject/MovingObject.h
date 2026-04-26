#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"


namespace KE {

	class Collider;
}

namespace P8
{

	struct MovingObjectData
	{
		Vector3f endPosition;
		bool shouldLoop = false;
		float timeToTarget = 0.0f;

	};
	class MovingObject : public KE::Component
	{

	public:
		MovingObject(KE::GameObject& aGO);
		~MovingObject();
		void Awake() override;
		void SetData(void* aData = nullptr) override;
		void Update() override;
		void Trigger();
		const Vector3f GetTargetPosition(const float t, Vector3f* aOutVelocity = nullptr);

		void OnCollisionEnter(const KE::CollisionData& aCollisionData) override;
		void OnCollisionStay(const KE::CollisionData& aCollisionData) override;
		void OnCollisionExit(const KE::CollisionData& aCollisionData) override;

		const Vector3f& GetVelocity() const { return myVelocity; }
		bool IsMoving() const { return myIsMoving; }

	private:
		std::array<Transform, 2> myTargetTransform;
		Transform& myGoTransform;
		bool myIsMoving = false;
		float myTimeToTarget = 1.0f;
		float myTimer = 0.0f;
		const float myThreshold = 0.98f;
		bool myTarget = false;
		KE::Collider* myCollider = nullptr;
		Vector3f myMovePlayerToTarget = { 0.0f, 0.0f, 0.0f };

		Vector3f myVelocity = { 0.0f, 0.0f, 0.0f };
		float height = 0.0f;

		Vector3f lastFramePos;
	};
}