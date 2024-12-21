#pragma once
#include "Engine\Source\ComponentSystem\Components\Component.h"
#include <Engine/Source/Math/Vector3.h>
#include <map>

namespace KE
{
	class Graphics;
	class Collider;
	struct CollisionData;

	struct SphereColliderComponentData
	{
		SphereColliderComponentData(Collider& aCollider) : collider(aCollider) {}

		float radius = 0.0f;
		Vector3f offset;
		Collider& collider;
		Graphics* graphics = nullptr;
		bool isTrigger = false;
	};

	class SphereColliderComponent : public Component
	{
		KE_EDITOR_FRIEND

	public:
		SphereColliderComponent(GameObject& aGameObject);
		~SphereColliderComponent();

		void Awake() override;
		void Update() override;


		void SetActive(const bool aValue) override;

		void OnEnable() override;
		void OnDisable() override;
		void OnDestroy() override;

		void OnCollisionEnter(const CollisionData& aCollisionData) override;
		void OnCollisionStay(const CollisionData& aCollisionData) override;
		void OnCollisionExit(const CollisionData& aCollisionData) override;

		void SetData(void* aDataObject = nullptr) override;

		void DrawDebug(KE::DebugRenderer& aDbg) override;
		inline KE::Collider* GetCollider() const { return myCollider; }
	private:
		KE::Collider* myCollider = nullptr;
		Vector4f myDefaultColour = { 1.0f,0.0f, 0.0f, 1.0f };
		Vector4f myHitColour = { 0.0f,1.0f, 0.0f, 1.0f };
		bool myHitLastFrame = false;

		float myRadius;
		Vector3f myOffset;

		std::map<Collider*, CollisionData*>  myCollisions;

		// FOR DEBUG RENDERING
		Graphics* myGraphicsPTR = nullptr;
	};
}