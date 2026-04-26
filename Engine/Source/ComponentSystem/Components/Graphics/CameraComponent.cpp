#include "stdafx.h"
#include "CameraComponent.h"
#include "Engine/Source/Graphics/Camera.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/Math/Transform.h"

KE::CameraComponent::CameraComponent(GameObject& aGameObject) : Component(aGameObject)
{}

KE::CameraComponent::~CameraComponent()
{
}

void KE::CameraComponent::Update()
{
	if (myMode == CameraComponentMode::Lead)
	{
		myCamera->transform.SetMatrix(myGameObject.myTransform.GetMatrix());
	}
	else if (myMode == CameraComponentMode::Follow)
	{
		myGameObject.myTransform.SetMatrix(myCamera->transform.GetMatrix());
	}
}

void KE::CameraComponent::SetData(void* aDataObject)
{
	CameraComponentData& data = *(CameraComponentData*)aDataObject;
	myCamera = data.camera;
	
	//myGameObject.myTransform.scale = Vector3f(1.0f, 1.0f, 1.0f);
	//myGameObject.myTransform.rotation = data.rotation;
}

KE::Camera* KE::CameraComponent::GetCamera() 
{
	return myCamera;
}

const DirectX::XMMATRIX KE::CameraComponent::GetViewMatrix() const
{
	return myCamera->GetViewMatrix();
}

const DirectX::XMMATRIX& KE::CameraComponent::GetProjectionMatrix() const
{
	return myCamera->GetProjectionMatrix();
}
