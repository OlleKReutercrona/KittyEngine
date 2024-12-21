#pragma once
#include "Engine/Source/Utility/Tags.h"
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Math/Transform.h"


#include <vector>
#include <string>

#include <type_traits> // inkludar denna 
#include <typeinfo>

#include "Engine/Source/Collision/Layers.h"

#include <Editor/Source/EditorInterface.h>

namespace KE
{
	struct CollisionData;
	struct PhysXCollisionData;
	class GameObjectManager;
	class DebugRenderer;

	class GameObject
	{
		friend class CollisionHandler;
		friend class BoxColliderComponent;
		friend class SphereColliderComponent;
		friend class CapsuleColliderComponent;

	public:
		Transform myTransform;
		Transform myWorldSpaceTransform;
		const int myID;
		Tags myTag = Tags::Untagged;
		Collision::Layers myLayer = Collision::Layers::Default;

		operator int() { return myID; }
		operator std::string() { return name; }

		template<class ComponentType>
		ComponentType& GetComponent();

		template<class ComponentType>
		inline std::vector<ComponentType*> GetComponents();

		template<class ComponentType>
		bool TryGetComponent(OUT ComponentType*& anOutComponent);

		template<class Type>
		KE::Component* GetComponentOfType();

		template<class ComponentType>
		ComponentType* AddComponent();

		template<class ComponentType>
		ComponentType* AddComponent(void* someData);

		template<class ComponentType>
		bool RemoveComponent();

		template<class ComponentType>
		bool HasComponent();

		inline bool IsStatic() { return isStatic; }
		inline bool IsActive() { return isActive; };
		void SetActive(const bool aValue);
		inline void SetName(const std::string& aName)
		{
			name = aName;
		}
		inline const std::string& GetName() const
		{
			return name;
		}
		GameObjectManager& GetManager();

		inline const std::vector<Component*>& GetComponentsRaw() { return myComponents; }
		inline const std::vector<GameObject*>& GetChildren() { return myChildren; }
		inline GameObject* GetParent() const { return myParent; }

		~GameObject();

	private:
		friend class GameObjectFactory;
		friend class GameObjectManager;
		friend class GameObjectCollisionInterface;
		KE_EDITOR_FRIEND


		GameObject(const int anID, const bool aIsStatic, GameObjectManager& aManager);

		void CalculateWorldTransform();
		void UpdateTransforms();

		void Awake();
		void EarlyUpdate();
		void LateUpdate();
		void Update();

		void OnDisable();
		void OnEnable();
		void OnDestroy();

		void OnCollisionEnter(const CollisionData& aCollisionData);
		void OnCollisionStay(const CollisionData& aCollisionData);
		void OnCollisionExit(const CollisionData& aCollisionData);
		
		void OnTriggerEnter(const CollisionData& aCollisionData);
		void OnTriggerStay(const CollisionData& aCollisionData);
		void OnTriggerExit(const CollisionData& aCollisionData);

		void OnPhysXCollision(const PhysXCollisionData& aPhysXCollisionData);

		void OnSceneChange();

		void DebugDraw(DebugRenderer& aDrawer);

		std::vector<Component*> myComponents;

		const bool isStatic = false;

		bool isActive = true;
		std::string name;

		GameObjectManager* myManager;

		GameObject* myParent = nullptr;
		std::vector<GameObject*> myChildren;
	};

	template<class ComponentType>
	inline ComponentType& GameObject::GetComponent()
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "Type must inherit from Component");

		ComponentType* t = nullptr;

		for (size_t i = 0; i < myComponents.size(); i++)
		{
			t = dynamic_cast<ComponentType*>(myComponents[i]);
			if (t != nullptr)
			{
				break;
			}
		}
		return *t;
	}

	template<class ComponentType>
	inline std::vector<ComponentType*> GameObject::GetComponents()
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "Type must inherit from Component");

		std::vector<ComponentType*> myList;

		for (size_t i = 0; i < myComponents.size(); i++)
		{
			ComponentType* t = dynamic_cast<ComponentType*>(myComponents[i]);
			if (t != nullptr)
			{
				myList.push_back(t);
			}
		}
		return myList;
	}

	template<class ComponentType>
	inline bool GameObject::TryGetComponent(OUT ComponentType*& anOutComponent)
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "Type must inherit from Component");

		ComponentType* t = nullptr;

		for (size_t i = 0; i < myComponents.size(); i++)
		{
			t = dynamic_cast<ComponentType*>(myComponents[i]);
			if (t != nullptr)
			{
				anOutComponent = (ComponentType*)&*myComponents[i];
				return true;
			}
		}

		return false;
	}

	template<class Type>
	inline KE::Component* GameObject::GetComponentOfType()
	{
		for (int i = 0; i < myComponents.size(); i++)
		{
			Type* t = (Type*)myComponents[i];
			if (t != nullptr)
			{
				return myComponents[i];
			}
		}

		return nullptr;
	}

	template<class ComponentType>
	inline ComponentType* GameObject::AddComponent()
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "Type must inherit from Component");

		myComponents.emplace_back(new ComponentType(*this));

		return static_cast<ComponentType*>(myComponents.back());
	}


	template<class ComponentType>
	inline ComponentType* GameObject::AddComponent(void* someData)
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "Type must inherit from Component");

		auto& component = myComponents.emplace_back(new ComponentType(*this));

		myComponents.back()->SetData(someData);

		return static_cast<ComponentType*>(myComponents.back());
	}


	template<class ComponentType>
	inline bool GameObject::RemoveComponent()
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "Type must inherit from Component");

		ComponentType* t = nullptr;

		for (size_t i = 0; i < myComponents.size(); i++)
		{
			t = dynamic_cast<ComponentType*>(myComponents[i]);
			if (t != nullptr)
			{
				myComponents[i]->OnDestroy();

				delete myComponents[i];

				myComponents.erase(myComponents.begin() + i);

				return true;
			}
		}

		return false;
	}

	template<class ComponentType>
	inline bool GameObject::HasComponent()
	{
		static_assert(std::is_base_of<Component, ComponentType>::value, "Type must inherit from Component");

		const std::type_info& type = typeid(ComponentType);
		for (size_t i = 0; i < myComponents.size(); i++)
		{
			if (type == typeid(*myComponents[i]))
			{
				return true;
			}
		}

		return false;
	}
}	 
