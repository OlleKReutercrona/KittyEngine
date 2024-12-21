#pragma once
#include "Engine/Source/Math/Vector3.h"
#include "Engine/Source/Collision/PhysicsObject.h"
#pragma warning(push, 0)
#include <External/Include/physx/PxPhysicsAPI.h>

#include "Layers.h"

#pragma warning(pop)


namespace P8
{
	enum class PlayerState;
}

struct PhysxControllerData
{
	float stepOffset = 0.0f;
	float skinWidth = 0.0001f;
	float halfHeight = 1.0f;
	float radius = 0.3f;
	Vector3f physicsMaterial = Vector3f(0.5f, 0.5f, 0.5f);
};

namespace KE
{
	class DebugRenderer;


	// Acceleration, Max forces, Decay
	struct MovementData
	{
		float maxSpeed = 5.0f;
		float maxRotation = 20.5f;
		float linearAcceleration = 100.0f;
		float angularAcceleration = 155.0f;
		float decayMagnitude = 20.0f;
		float angularDecayMagnitude = 10.0f;
	};

	struct CharacterControllerUserData
	{
		int myID; //just in case there is an evil cast somewhere :(

		enum class Type
		{
			Player,
			Boomerang,

			Count
		} objType = Type::Count;
		
		unsigned int team = 0;
		GameObject* gameObject = nullptr;

	};

	class CharacterControllerFilter : public physx::PxControllerFilterCallback, public physx::PxQueryFilterCallback
	{
	public:
		bool filter(const physx::PxController& a, const physx::PxController& b) override;
		physx::PxQueryHitType::Enum preFilter(const physx::PxFilterData& filterData, const physx::PxShape* shape,
			const physx::PxRigidActor* actor, physx::PxHitFlags& queryFlags) override;
		physx::PxQueryHitType::Enum postFilter(const physx::PxFilterData& filterData, const physx::PxQueryHit& hit,
			const physx::PxShape* shape, const physx::PxRigidActor* actor) override;
	};

	struct AttackUserData
	{
		std::unordered_map<int,bool> myHitIDs;
	};

	class PhysicsCharacterController : physx::PxUserControllerHitReport
	{
		friend class CollisionHandler;
	public:
		PhysicsCharacterController();
		~PhysicsCharacterController();

		std::pair<int, PhysicsObject*> Create(
			physx::PxPhysics* aPhysics,
			physx::PxControllerManager* aControllerManager,
			const PhysxControllerData& aData,
			const CharacterControllerUserData& aUserData
		);
		bool CheckAgainstMovingObjects();
		void DashUpdate();
		void AttackUpdate();

		void Update();
		void Move(const Vector3f& aDirection, float aSpeedScale);
		void Rotate(const Vector3f& aDirection);
		void Dash(const Vector3f& aDirection, float aDistance);

		inline bool IsDashing() const { return myIsDashing; }
		inline bool IsGrounded() const { return myIsGrounded; }
		inline Vector3f& GetVelocityRef() { return myVelocity; }
		void ApplyVelocity(const Vector3f& aVelocity);

		inline void SetPosition(const Vector3f& aPosition) { myController->setFootPosition({ aPosition.x, aPosition.y, aPosition.z }); }

		void onShapeHit(const physx::PxControllerShapeHit& aHit) override;
		void onControllerHit(const physx::PxControllersHit& aHit) override;
		void onObstacleHit(const physx::PxControllerObstacleHit& aHit) override;

		PhysicsObject& GetPhysicsObject() { return myPhysicsObject; }

		inline MovementData& GetMovementData() { return myMovementData; }
		void SetCollisionWithLayer(Collision::Layers aLayer, bool aEnable);
		inline void SetDashThroughWalls(bool aDashThroughWalls) { myDashThroughWalls = aDashThroughWalls; }

		inline const Vector3f& GetVelocity() const { return myVelocity; }
		inline CharacterControllerUserData* GetUserData() { return &myUserData; }
		inline void ResetAttack() { myAttackUserData.myHitIDs.clear(); }
		Vector3f GetFootPositioin();
		Vector3f GetRegularPositioin();

		float GetDistanceToGround(Vector3f* aPosition = nullptr);
		bool IsSquished() const { return myIsSquished; }

		inline void SetOrientation(float aOrientation) { myOrientation = aOrientation; }

	private:

		// Physx stuff
		physx::PxController* myController = NULL;
		physx::PxMaterial* myMaterial = NULL;
		PhysicsObject myPhysicsObject;
		physx::PxShape* myAttackShape = NULL;

		//filters!
		physx::PxControllerFilters myFilterData;
		physx::PxFilterData filterData;
		KE::CharacterControllerFilter myFilterCallback;
		//
		Transform* myTransform = nullptr;
		float myOrientation = 0.0f;
		bool myIsGrounded = false;

		float myCoyoteTimer = 0.0f; //how long have we been in the air
		float myCoyoteTime = 0.25f; //how long can we be in the air before we start falling

		bool myIsDashing = false;
		bool myIsSquished = false;

		Vector3f myDashTarget = {};
		Vector3f myDashOrigin = {};
		float myTimeToTarget = 0.1f;
		float myDashTimer = 0.2f;

		bool myDashThroughWalls = false;

		// Current forces
		Vector3f myVelocity;
		float myRotation = 0.0f;

		MovementData myMovementData;


		bool myLinearDecayActive = false;
		bool mAngularDecayActive = false;
		bool myShouldRotate = false;

		// Targets
		float targetRadius = 0.01745f; // Threashold for when rotation is good enough.
		float slowRadius = 0.8f; // Threashold for when to start slowing down our rotation.

		AttackUserData myAttackUserData;
		CharacterControllerUserData myUserData;
	};
}
