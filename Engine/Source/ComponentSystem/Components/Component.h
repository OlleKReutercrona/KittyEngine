#pragma once

namespace KE_EDITOR { class ComponentInspector; }

namespace KE
{
	struct PhysXCollisionData;
	class GameObject;
	struct CollisionData;
	class DebugRenderer;


	class Component
	{
		public:	
			virtual ~Component() {};

			virtual void SetData(void* aDataObject = nullptr) { aDataObject; };
			virtual void DefaultData() { __noop; }

			inline virtual void SetActive(const bool aValue)
			{
				isActive = aValue;

				isActive ? OnEnable() : OnDisable();
			};

			inline bool IsActive() const { return isActive; };

			inline GameObject& GetGameObject() { return myGameObject; };
			inline const GameObject& GetGameObject() const { return myGameObject; };
		protected:
			friend class GameObject;

			///// Update Loop Handling /////
			virtual void Awake() { __noop; };
			virtual void LateUpdate() { __noop; };
			virtual void EarlyUpdate() { __noop; };
			virtual void Update() { __noop; };

			///// Activity Handling /////
			virtual void OnEnable() { __noop; };
			virtual void OnDisable() { __noop; };
			virtual void OnDestroy() { __noop; };


			///// Collision Handling /////
			virtual void OnTriggerEnter(const CollisionData& aCollisionData) { __noop; aCollisionData; };
			virtual void OnTriggerStay(const CollisionData& aCollisionData) { __noop; aCollisionData; };
			virtual void OnTriggerExit(const CollisionData& aCollisionData) { __noop; aCollisionData; };

			virtual void OnCollisionEnter(const CollisionData& aCollisionData) { __noop; aCollisionData; };
			virtual void OnCollisionStay(const CollisionData& aCollisionData) { __noop; aCollisionData; };
			virtual void OnCollisionExit(const CollisionData& aCollisionData) { __noop; aCollisionData; };

			virtual void OnPhysXCollision(const PhysXCollisionData& aPhysXCollisionData) { __noop; aPhysXCollisionData; };


			/* Is called right when GameObjectManager destroys all other gameobjects. */
			virtual void OnSceneChange() { __noop; };

			///// Debug /////

			virtual void DrawDebug(KE::DebugRenderer& aDbg) { __noop; };
			virtual void Inspect(KE_EDITOR::ComponentInspector& aInspector) { __noop; };

			Component(GameObject& aParentGameObject) : myGameObject(aParentGameObject) {};

			GameObject& myGameObject;

			bool isActive = true;
	};
}