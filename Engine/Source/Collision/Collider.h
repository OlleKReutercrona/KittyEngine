#pragma once
//#include "Shapes/Shape.h"
#include <Engine/Source/Math/Vector3.h>


#include "CollisionData.h"
#include <vector>

class EnemyComponent;

namespace KE
{
	

	class Component;
	struct PhysicsObject;

	struct PhysxShapeUserData
	{
		int myID;
		GameObject* gameObject = nullptr;
	};

	class Collider
	{
		friend class CombatSystem;

	public:
		Collider() 
		{
			myOffset = { 0,0,0 };
			myPosition = { 0,0,0 };
			myBaseSize = { 1,1,1 };
			isActive = true;
		};
		~Collider() {};

		Component* myComponent = nullptr;
		PhysicsObject* myPhysicsObject = nullptr;
		PhysxShapeUserData myPhysxUserData;

		void UpdatePosition(const Vector3f& aPosition);
		void UpdateOffset(const Vector3f& anOffset);
		void UpdateSize(const Vector3f& aSize);
		void SetActive(const bool aValue);

		void AddForce(const Vector3f& theForce);
		void SetPhysxUserData(void* aData = nullptr);



		/*
		* Set to true if no physics should be applied.
		* Set to false to enable physics.
		*/
		void SetKinematic(const bool aValue);
		bool GetKinematic() const { return isKinematic; }
		void DisableGravity(const bool aValue);
		void DisableSimulation(const bool aValue);
		void SyncPhysXPosition();

		void SetLayer(int aLayer);

		PhysxShapeUserData myUserData;
	private:
		KE_EDITOR_FRIEND


		///// collision handler /////
		friend class CollisionHandler;

		///// Friended components /////
		friend class BoxColliderComponent;
		friend class SphereColliderComponent;
		friend class CapsuleColliderComponent;
		
		friend class EnemyComponent;

		

		std::vector<CollisionData> myCollisionData;

		// <<-+| DATA SET EXTERNALY |+->> 

		/*
		* Set This Variable With UpdateOffset Instead!
		*/
		Vector3f myOffset;
		/*
		* Set This Variable With UpdatePosition Instead!
		*/
		Vector3f myPosition;

		Vector3f myBaseSize;

		int myLayermask = 0;
		bool isActive = false;
		bool isStatic = false;
		bool isTrigger = false;
		bool isKinematic = false;
	};
}