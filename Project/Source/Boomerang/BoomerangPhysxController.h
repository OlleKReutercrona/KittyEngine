#pragma once
#include <characterkinematic/PxController.h>
#include "Engine/Source/Collision/PhysicsCharacterController.h"
#include "Engine/Source/Collision/PhysicsObject.h"
#include "characterkinematic/PxController.h"


namespace P8
{
	class BoomerangComponent;
	enum class BoomerangState;
	struct BallThrowParams;


	class BoomerangPhysicsController : physx::PxUserControllerHitReport
	{
		friend class CollisionHandler;
	public:
		BoomerangPhysicsController() = default;
		~BoomerangPhysicsController() override = default;

		std::pair<int, KE::PhysicsObject*> Create(physx::PxPhysics* aPhysics, physx::PxControllerManager* aControllerManager, const PhysxControllerData& aData, const KE::CharacterControllerUserData& aUserData);

		void Update(float aTimeDelta);
		void DebugRender();
		
		inline void SetPosition(const Vector3f& aPosition) const { myController->setFootPosition({ aPosition.x, aPosition.y, aPosition.z }); }
		inline void SetVelocity(const Vector3f& aVelocity) { myVelocity = aVelocity; }
		inline void AddVelocity(const Vector3f& aVelocity) { myVelocity += aVelocity; }
		inline void CapVelocity(float aMaxSpeed) { myVelocity = myVelocity.LengthSqr() > aMaxSpeed * aMaxSpeed ? myVelocity.GetNormalized() * aMaxSpeed : myVelocity; }

		void Recall();
		void Throw(const Vector3f& aVelocity, const Vector3f& aOrigin, BallThrowParams& aThrowParams);

		void Bounce(const Vector3f& aNormal);
		void Deflect(const Vector3f& aNormal);

		void onShapeHit(const physx::PxControllerShapeHit& aHit)		override;
		void onControllerHit(const physx::PxControllersHit& aHit)		override;
		void onObstacleHit(const physx::PxControllerObstacleHit& aHit)	override;

		Vector3f& GetVelocity() { return myVelocity; }
		KE::PhysicsObject& GetPhysicsObject() { return myPhysicsObject; }
		physx::PxController* GetController() const { return myController; }
		inline KE::CharacterControllerUserData* GetUserData() { return &myUserData; }
		inline void SetBoomerangComponent(BoomerangComponent* aBoomerangComponent) { myBoomerangComponent = aBoomerangComponent; }

		Vector3f GetPosition() const { return {
			static_cast<float>(myController->getPosition().x),
			static_cast<float>(myController->getPosition().y),
			static_cast<float>(myController->getPosition().z)
		}; }

		void SetRadius(float aRadius);

	private:
		void UpdatePosition(float aTimeDelta);
		void CalculateVelocity(float aTimeDelta);

		physx::PxController* myController = NULL;
		physx::PxMaterial* myMaterial = NULL;
		physx::PxControllerFilters myFilterData;
		physx::PxFilterData filterData;
		KE::CharacterControllerFilter myFilterCallback;
		KE::PhysicsObject myPhysicsObject;

		KE::CharacterControllerUserData myUserData;
		BallThrowParams* myThrowParams;
		BoomerangComponent* myBoomerangComponent = nullptr;
		Transform* myTransform = nullptr;
		Vector3f myVelocity = {};

		int myBounceCount = 0;
		float myFlightTime = 0.0f;
		float myBounceValue = 0.0f;
		float myMaxDecay = 5.0f;
		float myDefaultDecay = 1.0f;
		float myMinFlightBeforePickup = 0.2f;

		float myBounceSoundTimer = 0.0f;
		float myBounceSoundInterval = 0.04f;
		float myPickupAnimationTimer = 0.0f;
	};
}