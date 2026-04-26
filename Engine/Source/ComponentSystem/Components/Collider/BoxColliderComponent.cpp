#include "stdafx.h"
#include "ComponentSystem/Components/Collider/BoxColliderComponent.h"
#include "Engine/Source/Math/Matrix3x3.h"
#include "Engine/Source/Math/Matrix4x4.h"
#include "Engine/Source/Collision/Collider.h"
#include "Engine/Source/Collision/CollisionData.h"
#include "ComponentSystem/GameObject.h"
#include "Engine\Source\Graphics\DebugRenderer.h"

KE::BoxColliderComponent::BoxColliderComponent(GameObject& aGameObject) : Component(aGameObject)
{
	linePoints[0].colour = { 0.0f, 1.0f, 0.0f , 1.0f };
	linePoints[1].colour = { 0.0f, 1.0f, 0.0f , 1.0f };
	linePoints[2].colour = { 0.0f, 1.0f, 0.0f , 1.0f };
	linePoints[3].colour = { 0.0f, 1.0f, 0.0f , 1.0f };
	linePoints[4].colour = { 0.0f, 1.0f, 0.0f , 1.0f };
	linePoints[5].colour = { 0.0f, 1.0f, 0.0f , 1.0f };
	linePoints[6].colour = { 0.0f, 1.0f, 0.0f , 1.0f };
	linePoints[7].colour = { 0.0f, 1.0f, 0.0f , 1.0f };

	
}

KE::BoxColliderComponent::~BoxColliderComponent()
{

}

void KE::BoxColliderComponent::Awake()
{
	//myCollider->isActive = true; 
	//myCollider->SyncPhysXPosition();

	//myUserData.gameObject = &myGameObject;
	//myUserData.myID = myGameObject.myID;
}

void KE::BoxColliderComponent::Update()
{
	//if (!myCollider->isStatic)
	{
		myCollider->UpdatePosition(myGameObject.myWorldSpaceTransform.GetPositionRef());

		Matrix4x4f temp = myGameObject.myWorldSpaceTransform.GetMatrix();
		Matrix3x3f rot = temp;
		Vector3f* rows = (Vector3f*)&rot;

		for (int i = 0; i < 3; i++)
		{
			rows[i].Normalize();
		}

		//Vector3f offset = rot * myOffset;
		//myCollider->UpdateOffset(offset);

	}

	for (auto& collision : myCollider->myCollisionData)
	{
		if (collision.state & 1 << 0)
		{
			myGameObject.OnCollisionEnter(collision);
		}
		if (collision.state & 1 << 1)
		{
			myGameObject.OnCollisionStay(collision);
		}
		if (collision.state & 1 << 2)
		{
			myGameObject.OnCollisionExit(collision);
		}
	}

	//if (myCollisions.size() == 0 && myCollider->myCollisionData.size() == 0) return;
	//
	//for (auto it = myCollisions.begin(); it != myCollisions.end();)
	//{
	//
	//	int isColliding = false;
	//	for (int j = 0; j < myCollider->myCollisionData.size(); j++)
	//	{
	//		if (it->first == &myCollider->myCollisionData[j].hitCollider)
	//		{
	//			myIsHit = true;
	//			isColliding = true;
	//			// on stay
	//			if (myCollider->myCollisionData[j].isTrigger)
	//			{
	//				myGameObject.OnTriggerStay(myCollider->myCollisionData[j]);
	//				break;
	//			}
	//
	//			myGameObject.OnCollisionStay(myCollider->myCollisionData[j]);
	//			break;
	//		}
	//	}
	//
	//	if (isColliding == false)
	//	{
	//		myIsHit = false;
	//
	//		// on exit
	//		if (it->second->isTrigger)
	//		{
	//			myGameObject.OnTriggerExit(*it->second);
	//		}
	//		else
	//		{
	//			myGameObject.OnCollisionExit(*it->second);
	//		}
	//
	//		myCollisions.erase(it++);
	//		continue;
	//	}
	//	++it;
	//}
	//
	//for (int i = 0; i < myCollider->myCollisionData.size(); i++)
	//{
	//	if (myCollisions.count(&myCollider->myCollisionData[i].hitCollider) == 0)
	//	{
	//		myIsHit = true;
	//
	//		// On Enter
	//		myCollisions.insert({ &myCollider->myCollisionData[i].hitCollider , &myCollider->myCollisionData[i] });
	//		if (myCollider->myCollisionData[i].isTrigger)
	//		{
	//			myGameObject.OnTriggerEnter(myCollider->myCollisionData[i]);
	//			continue;
	//		}
	//		myGameObject.OnCollisionEnter(myCollider->myCollisionData[i]);
	//	}
	//}

	myCollider->myCollisionData.clear();
}

void KE::BoxColliderComponent::SetActive(const bool aValue)
{
	isActive = aValue;

	isActive ? OnEnable() : OnDisable();

	myCollider->isActive = aValue;
}

void KE::BoxColliderComponent::OnEnable()
{
	myCollider->isActive = true;
}

void KE::BoxColliderComponent::OnDisable()
{
	myCollider->isActive = false;
}

void KE::BoxColliderComponent::OnDestroy()
{

}

void KE::BoxColliderComponent::OnCollisionEnter(const CollisionData& aCollisionData)
{
	aCollisionData;
	//std::cout << "Collision enter | ID " << myGameObject.myID << " collided with " << aCollisionData.hitGameObject.myID << "\n";
}

void KE::BoxColliderComponent::OnCollisionStay(const CollisionData& aCollisionData)
{
	aCollisionData;
	//std::cout << "Collision stay | ID " << myGameObject.myID << " collided with " << aCollisionData.hitGameObject.myID << "\n";
}

void KE::BoxColliderComponent::OnCollisionExit(const CollisionData& aCollisionData)
{
	aCollisionData;
	//std::cout << "Collision exit | ID " << myGameObject.myID << " collided with " << aCollisionData.hitGameObject.myID << "\n";
}

// Found out that this debug rendering does not match physx // DR 
void KE::BoxColliderComponent::DrawDebug(KE::DebugRenderer& aDbg)
{
	if (myCollider->isStatic) return;

	Transform temp = myGameObject.myWorldSpaceTransform;
	temp.TranslateLocal(myOffset);

	aDbg.RenderCube(
		temp,
		mySize,
		myIsHit ? myHitColour : myDefaultColour
	);
}

void KE::BoxColliderComponent::SetData(void* aDataObject)
{
	BoxColliderComponentData* data = (BoxColliderComponentData*)aDataObject;

	myMax.x = (data->size.x * myGameObject.myTransform.GetScale().x) / 2;
	myMax.y = (data->size.y * myGameObject.myTransform.GetScale().y) / 2;
	myMax.z = (data->size.z * myGameObject.myTransform.GetScale().z) / 2;

	myMin.x = (-data->size.x * myGameObject.myTransform.GetScale().x) / 2;
	myMin.y = (-data->size.y * myGameObject.myTransform.GetScale().y) / 2;
	myMin.z = (-data->size.z * myGameObject.myTransform.GetScale().z) / 2;

	mySize = data->size;
	myOffset.x = data->offset.x * myGameObject.myTransform.GetScale().x;
	myOffset.y = data->offset.y * myGameObject.myTransform.GetScale().y;
	myOffset.z = data->offset.z * myGameObject.myTransform.GetScale().z;

	PhysxShapeUserData userData;
	userData.gameObject = &myGameObject;
	userData.myID = myGameObject.myID;

	data->collider.SetPhysxUserData(&userData);


	//data->collider.myShape->myOffset

	myCollider = &data->collider;
	myCollider->myOffset = myOffset; //data->offset;
	myCollider->myComponent = this;
	//myCollider->myShape = new Box(myMin, myMax, myGameObject.myTransform.GetPositionRef());
	//myCollider->myShape->myShapeData = eShapes::eBox;
	myCollider->isTrigger = data->isTrigger;


	myGraphicsPTR = data->graphics;
}