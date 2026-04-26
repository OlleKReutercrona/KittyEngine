#include "stdafx.h"
#include "ComponentSystem/Components/Collider/SphereColliderComponent.h"
#include "Collision/Collider.h"
#include "Collision/CollisionData.h"
#include "ComponentSystem/GameObject.h"
//#include "Collision/Shape.h"
//#include "Engine\Source\ComponentSystem\GameObjectManager.h"
//#include "Engine\Source\SceneManagement\Scene.h"
#include "Engine\Source\Graphics\DebugRenderer.h"

#include <iostream>

KE::SphereColliderComponent::SphereColliderComponent(GameObject& aGameObject) : KE::Component(aGameObject)
{
}

KE::SphereColliderComponent::~SphereColliderComponent()
{
}

void KE::SphereColliderComponent::Awake()
{
}

void KE::SphereColliderComponent::Update()
{
	if (!myCollider->isStatic)
	{
		//myCollider->UpdatePosition(myGameObject.myWorldSpaceTransform.GetPositionRef());
	}
	//myCollider->myShape->myOffset = myOffset;
	//myCollider->myPosition = myGameObject.myWorldSpaceTransform.GetPositionRef();

	if (myCollisions.size() == 0 && myCollider->myCollisionData.size() == 0) return;

	for (auto it = myCollisions.begin(); it != myCollisions.end();)
	{

		int isColliding = false;
		for (int j = 0; j < myCollider->myCollisionData.size(); j++)
		{
			if (it->first == &myCollider->myCollisionData[j].hitCollider)
			{
				myHitLastFrame = true;

				isColliding = true;
				std::cout << "STAY KITTY" << std::endl;

				// on stay
				if (myCollider->myCollisionData[j].isTrigger)
				{
					myGameObject.OnTriggerStay(myCollider->myCollisionData[j]);
					break;
				}

				myGameObject.OnCollisionStay(myCollider->myCollisionData[j]);
				break;
			}
		}

		if (isColliding == false)
		{
			myHitLastFrame = false;
			std::cout << "EXIT KITTY" << std::endl;

			// on exit
			if (it->second->isTrigger)
			{
				myGameObject.OnTriggerExit(*it->second);
			}
			else
			{
				myGameObject.OnCollisionExit(*it->second);
			}

			myCollisions.erase(it++);
			continue;
		}
		++it;
	}

	for (int i = 0; i < myCollider->myCollisionData.size(); i++)
	{
		if (myCollisions.count(&myCollider->myCollisionData[i].hitCollider) == 0)
		{
			myHitLastFrame = true;
			std::cout << "ENTER KITTY" << std::endl;

			// On Enter
			myCollisions.insert({ &myCollider->myCollisionData[i].hitCollider , &myCollider->myCollisionData[i] });
			if (myCollider->myCollisionData[i].isTrigger)
			{
				myGameObject.OnTriggerEnter(myCollider->myCollisionData[i]);
				continue;
			}
			myGameObject.OnCollisionEnter(myCollider->myCollisionData[i]);
		}
	}

	myCollider->myCollisionData.clear();
}

void KE::SphereColliderComponent::SetActive(const bool aValue)
{
	isActive = aValue;

	isActive ? OnEnable() : OnDisable();

	myCollider->isActive = aValue;
}

void KE::SphereColliderComponent::OnEnable()
{
	myCollider->isActive = true;
}

void KE::SphereColliderComponent::OnDisable()
{
	myCollider->isActive = false;
}

void KE::SphereColliderComponent::OnDestroy()
{
}

void KE::SphereColliderComponent::OnCollisionEnter(const CollisionData& aCollisionData)
{
}

void KE::SphereColliderComponent::OnCollisionStay(const CollisionData& aCollisionData)
{
}

void KE::SphereColliderComponent::OnCollisionExit(const CollisionData& aCollisionData)
{
}

void KE::SphereColliderComponent::SetData(void* aDataObject)
{
	SphereColliderComponentData* data = (SphereColliderComponentData*)aDataObject;

	myRadius = data->radius;

	myOffset.x = data->offset.x * myGameObject.myTransform.GetScale().x;
	myOffset.y = data->offset.y * myGameObject.myTransform.GetScale().y;
	myOffset.z = data->offset.z * myGameObject.myTransform.GetScale().z;

	myCollider = &data->collider;
	myCollider->myOffset = myOffset; //data->offset;
	myCollider->myComponent = this;
	//myCollider->myShape = new Sphere(myRadius, myGameObject.myTransform.GetPositionRef());
	//myCollider->myShape->myShapeData = eShapes::eSphere;
	myCollider->isTrigger = data->isTrigger;

	myGraphicsPTR = data->graphics;
}

void KE::SphereColliderComponent::DrawDebug(KE::DebugRenderer& aDbg)
{
	Vector4f colour = myHitLastFrame ? myHitColour : myDefaultColour;

	aDbg.RenderSphere(myGameObject.myWorldSpaceTransform.GetPositionRef() + myOffset, myRadius, colour);
}

//void KE::SphereColliderComponent::SetRadius(const float aRadius)
//{
//	myRadius = Clamp(aRadius, 0.0f, 10000.0f);
//	Sphere* sphere = (Sphere*)myCollider->myShape;
//	sphere->myRadius = Clamp(aRadius, 0.0f, 10000.0f);
//}
