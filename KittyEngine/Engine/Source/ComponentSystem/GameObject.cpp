#include "stdafx.h"
#include "GameObject.h"
#include "Collision/CollisionData.h"
#include "Engine\Source\ComponentSystem\GameObjectManager.h"

KE::GameObject::GameObject(const int anID, const bool aIsStatic, GameObjectManager& aManager) : myID(anID), myManager(&aManager), isStatic(aIsStatic)
{ 

}

void KE::GameObject::CalculateWorldTransform()
{
	myWorldSpaceTransform = myTransform * myParent->myWorldSpaceTransform;
	for (int i = 0; i < myChildren.size(); i++)
	{
		myChildren[i]->CalculateWorldTransform();
	}
}

KE::GameObjectManager& KE::GameObject::GetManager()
{
	return *myManager;
}

KE::GameObject::~GameObject()
{
	OnDestroy();

	for (int i = 0; i < myComponents.size(); i++)
	{
		delete myComponents[i];
	}
	myComponents.clear();
}

void KE::GameObject::Awake()
{
	if (!isActive) return;

	for (int i = 0; i < myComponents.size(); i++)
	{
		myComponents[i]->Awake();
	}
}

void KE::GameObject::EarlyUpdate()
{
	if (!isActive) return;

	for (int i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->isActive) continue;

		myComponents[i]->EarlyUpdate();
	}
}

void KE::GameObject::LateUpdate()
{
	if (!isActive) return;

	for (int i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->isActive) continue;

		myComponents[i]->LateUpdate();
	}
}

void KE::GameObject::Update()
{
	if (!isActive) return;

	//UpdateTransforms();


	for (int i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->isActive) continue;

		myComponents[i]->Update();
	}
}

void KE::GameObject::OnDisable()
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->isActive) continue;

		myComponents[i]->OnDisable();
	}
}

void KE::GameObject::OnEnable()
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->isActive) continue;

		myComponents[i]->OnEnable();
	}
}

void KE::GameObject::OnDestroy()
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		myComponents[i]->OnDestroy();
	}
}

void KE::GameObject::OnCollisionEnter(const CollisionData& aCollisionData)
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->isActive) continue;

		myComponents[i]->OnCollisionEnter(aCollisionData);
	}
}

void KE::GameObject::OnCollisionStay(const CollisionData& aCollisionData)
{	
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->isActive) continue;

		myComponents[i]->OnCollisionStay(aCollisionData);
	}
}

void KE::GameObject::OnCollisionExit(const CollisionData& aCollisionData)
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->isActive) continue;

		myComponents[i]->OnCollisionExit(aCollisionData);
	}
}

void KE::GameObject::OnTriggerEnter(const CollisionData& aCollisionData)
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->isActive) continue;

		myComponents[i]->OnTriggerEnter(aCollisionData);
	}
}

void KE::GameObject::OnTriggerStay(const CollisionData& aCollisionData)
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->isActive) continue;

		myComponents[i]->OnTriggerStay(aCollisionData);
	}
}

void KE::GameObject::OnTriggerExit(const CollisionData& aCollisionData)
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->isActive) continue;

		myComponents[i]->OnTriggerExit(aCollisionData);
	}
}

void KE::GameObject::OnPhysXCollision(const PhysXCollisionData& aPhysXCollisionData)
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->isActive) continue;

		myComponents[i]->OnPhysXCollision(aPhysXCollisionData);
	}
}

void KE::GameObject::OnSceneChange()
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		myComponents[i]->OnSceneChange();
	}
}

void KE::GameObject::DebugDraw(DebugRenderer& aDrawer)
{
	for (int i = 0; i < myComponents.size(); i++)
	{
		if (!myComponents[i]->isActive) continue;

		myComponents[i]->DrawDebug(aDrawer);
	}
}

void KE::GameObject::UpdateTransforms()
{
	if (myParent != nullptr) return;
	// if this gameobject is the root object, calculate child transform


	myWorldSpaceTransform = myTransform;
	for (auto& child : myChildren)
	{
		child->CalculateWorldTransform();
	}
}

void KE::GameObject::SetActive(const bool aValue)
{
	isActive = aValue;

	isActive ? OnEnable() : OnDisable();
}