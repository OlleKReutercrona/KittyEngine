#include "stdafx.h"
#include "CapsuleColliderComponent.h"
#include "Engine\Source\Graphics\DebugRenderer.h"
#include "Engine\Source\ComponentSystem\GameObject.h"
#include "Engine\Source\Collision\Collider.h"


KE::CapsuleColliderComponent::CapsuleColliderComponent(GameObject& aGameobject) : Component(aGameobject)
{
}

void KE::CapsuleColliderComponent::Update()
{
	if (myCollisions.size() == 0 && myCollider->myCollisionData.size() == 0) return;

	for (auto it = myCollisions.begin(); it != myCollisions.end();)
	{

		int isColliding = false;
		for (int j = 0; j < myCollider->myCollisionData.size(); j++)
		{
			if (it->first == &myCollider->myCollisionData[j].hitCollider)
			{
				myIsHit = true;
				isColliding = true;
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
			myIsHit = false;

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
			myIsHit = true;

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

void KE::CapsuleColliderComponent::SetActive(const bool aValue)
{
	isActive = aValue;

	isActive ? OnEnable() : OnDisable();

	myCollider->SetActive(aValue);
}

void KE::CapsuleColliderComponent::OnEnable()
{
	myCollider->SetActive(true);
}

void KE::CapsuleColliderComponent::OnDisable()
{
	myCollider->SetActive(false);
}


void KE::CapsuleColliderComponent::DrawDebug(KE::DebugRenderer& aDbg)
{
	Transform temp = myGameObject.myWorldSpaceTransform;
	temp.TranslateLocal(myOffset);

	aDbg.RenderCapsule(temp, myRadius / 2.0f, myLength / 2.0f, myIsHit ? myHitColour : myDefaultColour);
}

void KE::CapsuleColliderComponent::SetData(void* aDataObject)
{
	CapsuleColliderData* data = (CapsuleColliderData*)aDataObject;

	myRadius = data->radius;
	myLength = data->length;
	myIsTrigger = data->isTrigger;

	myCollider = &data->collider;

	myCollider->myOffset.x = data->offset.x * myGameObject.myTransform.GetScale().x;
	myCollider->myOffset.y = data->offset.y * myGameObject.myTransform.GetScale().y;
	myCollider->myOffset.z = data->offset.z * myGameObject.myTransform.GetScale().z;

	myCollider->myComponent = this;
	myCollider->isTrigger = myIsTrigger;

	PhysxShapeUserData userData;
	userData.gameObject = &myGameObject;
	userData.myID = myGameObject.myID;

	data->collider.SetPhysxUserData(&userData);
}
