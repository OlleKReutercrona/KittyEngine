#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include <map>


namespace KE
{
	class Collider;
	struct CollisionData;

	struct CapsuleColliderData
	{
		CapsuleColliderData(KE::Collider& aCollider) : collider(aCollider) {}

		Vector3f offset;
		float radius;
		float length;
		bool isTrigger = false;
		KE::Collider& collider;
	};

	class CapsuleColliderComponent : public Component
	{
	public:
		CapsuleColliderComponent(GameObject& aGameobject);
		~CapsuleColliderComponent() = default;

		void Update() override;

		void SetActive(const bool aValue) override;

		void OnEnable() override;
		void OnDisable() override;

		void DrawDebug(KE::DebugRenderer& aDbg) override;

		void SetData(void* aDataObject = nullptr) override;

		inline void DrawDebugLines(const bool aValue) { myDrawDebug = aValue; };

		inline KE::Collider* GetCollider() const { return myCollider; }
	private:
		KE::Collider* myCollider = nullptr;

		std::map<Collider*, CollisionData*>  myCollisions;

		Vector4f myDefaultColour = { 1.0f, 0.0f, 0.0f, 1.0f };
		Vector4f myHitColour = { 0.0f, 1.0f, 0.0f, 1.0f };

		Vector3f myOffset;
		float myRadius = 0.0f;
		float myLength = 0.0f;

		bool myIsHit = false;
		bool myDrawDebug = true;
		bool myIsTrigger = false;
	};
}

