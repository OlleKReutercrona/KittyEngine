#include "stdafx.h"
#include "RaycastHandler.h"
#include <Engine/Source/Graphics/Camera.h>
#include <Engine/Source/Collision/CollisionHandler.h>
#include <Engine/Source/Collision/Layers.h>
#include <Engine/Source/Collision/Collider.h>

namespace KE
{
	void RaycastHandler::Init(Camera& aSceneCamera, CollisionHandler& aCollisionHandler)
	{
		mySceneCamera = &aSceneCamera;
		myCollisionHandler = &aCollisionHandler;
	}

	Vector3f RaycastHandler::Raycast(Vector2f aMousePos, Vector3f aPlayerPosition)
	{
		Vector3f position = { 0,0,0 };
		//float distance = FLT_MAX;
		//Rayf ray = mySceneCamera->GetRay(aMousePos);
		//
		//myCollisionHandler->NavmeshRayCast(ray, position, aPlayerPosition);

		//if (position.LengthSqr() > 0) {
		//	return position;
		//}

		//myCollisionHandler->PlayerRayCast(ray, position, aPlayerPosition);

		return position;
	}

	bool RaycastHandler::RayCastMissed(Vector2f aMousePos, Vector3f aPlayerPosition)
	{
		Vector3f position = { 0,0,0 };
		float distance = FLT_MAX;
		Rayf ray = mySceneCamera->GetRay(aMousePos);

		myCollisionHandler->NavmeshRayCast(ray, position, aPlayerPosition);

		if (position.LengthSqr() > 0) 
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	Vector3f RaycastHandler::RaycastPlayerAction(Vector2f aMousePos, Vector3f aPlayerPosition)
	{
		Vector3f position = {};
		//Rayf ray = mySceneCamera->GetRay(aMousePos);
		//myCollisionHandler->PlayerRayCast(ray, position, aPlayerPosition);

		return position;
	}

	Vector3f RaycastHandler::RaycastEnemy(Vector2f aMousePos)
	{
		Vector3f position = {};
		//Rayf ray = mySceneCamera->GetRay(aMousePos);
		//KE::Collider* temp = myCollisionHandler->RayCast(ray, position, static_cast<int>(KE::Collision::Layers::Enemy));
		return position;
	}

	bool RaycastHandler::RayCastEnemyHit(Vector2f aMousePos)
	{
		Vector3f position = {};
		//Rayf ray = mySceneCamera->GetRay(aMousePos);
		//KE::Collider* temp = myCollisionHandler->RayCast(ray, position, static_cast<int>(KE::Collision::Layers::Enemy));
		//if (temp != nullptr)
		//{
		//	return true;
		//}

		return false;
	}


	void RaycastHandler::AssignCamera(Camera& aCamera)
	{
		mySceneCamera = &aCamera;
	}
}