#pragma once

#if !defined(PX_PHYSICS_FOUNDATION_VERSION)
#define PX_PHYSICS_FOUNDATION_VERSION 0x05030100
#endif

#pragma warning(push, 0)
#include <External/Include/physx/PxPhysicsAPI.h>
#include <External/Include/physx/cooking/PxCooking.h>
#pragma warning(pop)
#include "PhysicsCharacterController.h"
#include "Engine/Source/Math/Ray.h"
#include "Engine/Source/Math/Vector3.h"
#include "Engine/Source/Collision/Shape.h"
#include "Engine/Source/Collision/Layers.h"
#include "Engine/Source/Collision/PhysicsObject.h"
#include "Engine/Source/Graphics/ModelData.h"

//https://nvidia-omniverse.github.io/PhysX/physx/5.3.1/_api_build/class_px_simulation_event_callback.html#_CPPv425PxSimulationEventCallback

// https://nvidia-omniverse.github.io/PhysX/physx/5.3.1/docs/Simulation.html#callback-sequence

// https://nvidia-omniverse.github.io/PhysX/physx/5.3.1/_api_build/class_px_box_geometry.html#_CPPv413PxBoxGeometry

// https://nvidia-omniverse.github.io/PhysX/physx/5.3.1/docs/Geometry.html

// https://winter.dev/articles/physics-engine

//namespace CommonUtilities
//{
//	class Rayf;
//	class Vector3f;
//}



namespace KE
{
	enum class CollisionMaskTest : int
	{
		eDefault = 1 << 0,
		ePlayer = 1 << 1,
		eEnemy = 1 << 2,
		eProjectile = 1 << 3,
		eTrigger = 1 << 4,
		eNavmesh = 1 << 5,
		eAll = INT_MAX
	};

	class Navmesh;

	class Collider;

	class DebugRenderer;


	class CollisionHandler : physx::PxSimulationEventCallback
	{
		friend class Scene;
	public:
		CollisionHandler();
		~CollisionHandler();
		void Init();
		void Unload();

		void RegisterToBlackboard();

		void AddNavmesh(Navmesh& aNavmesh);
		void NavmeshRayCast(Rayf& aRay, Vector3f& aOutPoint, Vector3f aUserPosition);

		void Update();

		// Old Collision
		//Collider& AddCollider();
		//void CheckCollision();


		/*
			Returns the first collider hit
		*/
		KE::Collider* RayCast(
			const Rayf& aRay,
			Vector3f& anOutPoint,
			const float aDistance = 100.0f,
			const int aLayerMask = 0,
			Vector3f* aOutNormal = nullptr
		);

		//KE::Collider* RayCast(Rayf& aRay, Vector3f& anOutPoint, const int aLayer);
		//KE::Collider* RayCast(Rayf& aRay, const bool isTrigger = false);
		//bool PlayerRayCast(Rayf& aRay, Vector3f& aOutPoint, Vector3f aPlayerPosition);

		//std::vector<KE::Collider*> ColliderCast(const KE::Shape& aShape, const int aLayerMask);
		std::vector<KE::Collider*> BoxCast(const KE::Box& aBox, int aLayerMask = 1);
		//std::vector<Collider*> SphereCast(const KE::Sphere& aSphere, const int aLayerMask = 1);


		// PhysX
		KE::Collider* CreateBox(const Transform& aTransform, const Vector3f& anOffset, const Vector3f& aSize, const Collision::Layers aLayer = Collision::Layers::Default, const bool aIsStatic = true, const bool aIsKinematic = true);
		KE::Collider* CreateSphere(const Transform& aTransform, const Vector3f& anOffset, const float aRadius, const Collision::Layers aLayer = Collision::Layers::Default, const bool aIsStatic = true, const bool aIsKinematic = true);
		KE::Collider* CreateCapsule(const Transform& aTransform, const Vector3f& anOffset, const float aRadius, const float aHalfHeight, const Collision::Layers aLayer = Collision::Layers::Default, const bool aIsKinematic = true, const bool aIsStatic = true);

		KE::Collider* CreateMeshCollider(const Transform& aTransform, const std::vector<Vertex>& someVertices, const std::vector<unsigned int>& someIndices, const Collision::Layers aLayer = Collision::Layers::Default, const bool aIsStatic = true, const bool aIsKinematic = true);
		
		KE::PhysicsCharacterController* CreateController(const PhysxControllerData& aPhysxData, const CharacterControllerUserData& aControllerUserData);
		void RegisterPhysicsObject(const std::pair<int, PhysicsObject*>& anObject);

		physx::PxPhysics* GetPhysics() const { return myPhysics; }
		physx::PxControllerManager* GetControllerManager() const { return myControllerManager; }

	private:
		//void UpdateCollisionData(KE::Collider* aCollider, const bool aHitThisFrame = false);

		void InitPhysX();

		void UpdateGameObjects();

		void DebugDraw(KE::DebugRenderer& aDrawer);

		int GenerateID();

		std::vector<PhysicsObject> myPhysicsObjects;

		std::map<int, PhysicsObject*> myPhysObjects;

		//std::vector<Collider*> myColliders;
		Navmesh* myNavmesh = nullptr;

		//Inherited via PxSimulationEventCallback
		void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override;
		void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override;

		void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override { __noop; };
		void onWake(physx::PxActor** actors, physx::PxU32 count) override { __noop; };;
		void onSleep(physx::PxActor** actors, physx::PxU32 count) override { __noop; };;
		void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override { __noop; };


		static physx::PxFilterFlags CustomFilterShader
		(	physx::PxFilterObjectAttributes attribute0, 
			physx::PxFilterData filterData0, 
			physx::PxFilterObjectAttributes attribute1, 
			physx::PxFilterData filterData1,
			physx::PxPairFlags& pairFlags, 
			const void* constantBlock, 
			physx::PxU32 constantBlockSize);

		// PhysX Vars
		physx::PxDefaultAllocator      myDefaultAllocatorCallback;
		physx::PxDefaultErrorCallback  myDefaultErrorCallback;
		physx::PxTolerancesScale       myToleranceScale;
		physx::PxDefaultCpuDispatcher* myDispatcher = NULL;
		physx::PxFoundation* myFoundation = NULL;
		physx::PxPhysics* myPhysics = NULL;
		physx::PxScene* myScene = NULL;
		physx::PxMaterial* myMaterial = NULL;
		
		physx::PxPvd*                  myPvd = NULL;
		physx::PxOmniPvd* myOmniPvd = NULL;
		KE::DebugRenderer* myDbg = NULL;
		

		// Controller test
		physx::PxControllerManager* myControllerManager = NULL;

		//PhysicsCharacterController myPlayerController;
		int myPlayersID = -1;
	};
}
