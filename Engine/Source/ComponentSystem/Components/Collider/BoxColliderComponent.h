#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include <Engine/Source/Math/Vector3.h>
#include <map>
#include "Engine/Source/Graphics/DebugLine.h"
#include "Engine/Source/Reflection/Reflection.h"

namespace KE
{
	class Graphics;
	class Collider;
	struct CollisionData;

	struct BoxColliderComponentData
	{
		BoxColliderComponentData(Collider& aCollider) : collider(aCollider) {}

		Vector3f size;
		Vector3f offset;
		Collider& collider;
		Graphics* graphics = nullptr;
		bool isTrigger = false;
	};

	class BoxColliderComponent : public Component
	{
	public:
		BoxColliderComponent(GameObject& aGameObject);
		~BoxColliderComponent();

		void Awake() override;
		void Update() override;

		void SetActive(const bool aValue) override;

		void OnEnable() override;
		void OnDisable() override;
		void OnDestroy() override;

		void OnCollisionEnter(const CollisionData& aCollisionData) override;
		void OnCollisionStay(const CollisionData& aCollisionData) override;
		void OnCollisionExit(const CollisionData& aCollisionData) override;

		void DrawDebug(KE::DebugRenderer& aDbg) override;
		void SetData(void* aDataObject = nullptr) override;

		inline void DrawDebugLines(const bool aValue) { myDrawDebug = aValue; };
		inline KE::Collider* GetCollider() const { return myCollider; }

		/*BEGIN_REFLECTION(BoxColliderComponent)
		{
			REF(myCollider);
			REF(myDefaultColour);
			REF(myHitColour);
			REF(mySize);
		}
		END_REFLECTION;*/

		friend class KE_SER::Serializer;
		template<class SerializationIO>
		void Serialize(SerializationIO& aSerializer)
		{
			SET_REFLECTION(BoxColliderComponent)
			aSerializer & PARAM(myDefaultColour)
						& PARAM(myHitColour)
						& PARAM(mySize)
						& PARAM(myOffset)
			;	
		}

	private:
		KE::Collider* myCollider = nullptr;
		Vector4f myDefaultColour = { 1.0f, 0.0f, 0.0f, 1.0f };
		Vector4f myHitColour = { 0.0f, 1.0f, 0.0f, 1.0f };
		bool myIsHit = false;
		bool myDrawDebug = true;

		Vector3f mySize;
		Vector3f myMin;
		Vector3f myMax;
		Vector3f myOffset;

		Transform myCachedTransform;

		std::map<Collider*, CollisionData*>  myCollisions;

		// FOR DEBUG RENDERING
		Graphics* myGraphicsPTR = nullptr;
		LineVertex linePoints[8];
	};
}


//BEGIN_REFLECTION(BoxColliderComponent);
//REFLECT_MEMBER(myCollider);
//REFLECT_MEMBER(myDefaultColour);
//REFLECT_MEMBER(myHitColour);
//REFLECT_MEMBER(mySize);

//END_REFLECTION;
