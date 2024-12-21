#include "stdafx.h"
#include "Camera.h"

#include "External/include/imgui/imgui.h"

#include "Engine/Source/Math/KittyMath.h"

#include <algorithm>

KE::Camera::Camera(int aIndex) : myIndex(aIndex)
{
	myProjectionMatrix = DirectX::XMMatrixIdentity();
	myProjectionData = ProjectionData();
}

void KE::Camera::SetFOV(const float aFOV)
{
	SetPerspective(myProjectionData.perspective.width, myProjectionData.perspective.height, aFOV, myProjectionData.perspective.nearPlane, myProjectionData.perspective.farPlane);
}

void KE::Camera::SetPerspective(float aWidth, float aHeight, float aFov, float aNearPlane, float aFarPlane)
{
	myProjectionData.perspective.width = aWidth;
	myProjectionData.perspective.height = aHeight;
	myProjectionData.perspective.fov = aFov;
	myProjectionData.perspective.nearPlane = aNearPlane;
	// TODO This should be set from one and only one place, not in Game/Cameramanager/Component etc...
	myProjectionData.perspective.farPlane = aFarPlane;
	myType = ProjectionType::Perspective;

	//our fov is horiontal, so we need to calculate the vertical fov to use in the perspective matrix
	const float aspectRatio = aHeight / aWidth;
	//float verticalFov = 2.0f * atan(tan(aFov / 2.0f) * (aHeight / aWidth));


	const float fovDeg = aFov * KE::RadToDegImmediate;

	const float verticalFov = 2.0f * atan(tan(aFov / 2.0f) * aspectRatio);

	const float verticalFovDeg = verticalFov * KE::RadToDegImmediate;

	myProjectionData.perspective.verticalFov = verticalFov;

	myProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(verticalFov, aWidth / aHeight, aNearPlane, aFarPlane);

}


void KE::Camera::SetOrthographic(float aWidth, float aHeight, float aNearPlane, float aFarPlane)
{
	myProjectionData.orthographic.width = aWidth;
	myProjectionData.orthographic.height = aHeight;
	myProjectionData.orthographic.nearPlane = aNearPlane;
	myProjectionData.orthographic.farPlane = aFarPlane;

	myType = ProjectionType::Orthographic;

	myProjectionMatrix = DirectX::XMMatrixOrthographicLH(aWidth, aHeight, aNearPlane, aFarPlane);
}

const DirectX::XMMATRIX KE::Camera::GetViewMatrix()
{
	return DirectX::XMMatrixInverse(nullptr, transform.GetMatrix());
}

const DirectX::XMMATRIX& KE::Camera::GetProjectionMatrix()
{
	return myProjectionMatrix;
}

Rayf KE::Camera::ScreenPointToRay(const Vector2f& aMousePosition)
{
	Vector2i res;
	res.x = (int)myProjectionData.perspective.width;
	res.y = (int)myProjectionData.perspective.height;
	Vector2f mousePos;

	Vector2f mousePosDiv = {
		((aMousePosition.x + 0.5f) / res.x) * 2.0f - 1.0f,
		(((aMousePosition.y + 0.5f) / res.y) * 2.0f - 1.0f) * -1.0f
	};

	float camHFovRad = myProjectionData.perspective.fov;
	float camVFovRad = myProjectionData.perspective.verticalFov;
	float tanHalfHFov = tan(camHFovRad * 0.5f);
	float tanHalfVFov = tan(camVFovRad * 0.5f);

	Vector3f forward = transform.GetForward();
	Vector3f right = transform.GetRight();
	Vector3f up = transform.GetUp();

	Vector3f rightScaled = right * tanHalfHFov;
	Vector3f upScaled = up * tanHalfVFov;

	Vector3f dir = forward + mousePosDiv.x * rightScaled + mousePosDiv.y * upScaled;
	dir.Normalize();

	Vector3f origin = transform.GetPosition();

	Rayf ray;
	ray.InitWithOriginAndDirection(origin, dir);

	return ray;
}

Rayf KE::Camera::GetRay(const Vector2f aMousePosition)
{
	return ScreenPointToRay(aMousePosition);
}

bool KE::Camera::WorldToScreenPoint(const Vector3f& aWorldPoint, Vector2f& aOutScreenPoint, Vector2f aScreenDimensions)
{
	Vector4f positonClip = { aWorldPoint.x, aWorldPoint.y, aWorldPoint.z, 1.0f };

	positonClip = positonClip * Matrix4x4f(GetViewMatrix() * GetProjectionMatrix());


	//if behind camera, skip
	if (positonClip.z < 0.0f)
	{
		return false;
	}

	positonClip /= positonClip.w;
	if (aScreenDimensions.x <= 0.0f || aScreenDimensions.y <= 0.0f)
	{
		aScreenDimensions = { myProjectionData.perspective.width, myProjectionData.perspective.height };
	}

	aOutScreenPoint.x = (positonClip.x + 1.0f) * 0.5f * aScreenDimensions.x;
	aOutScreenPoint.y = aScreenDimensions.y - (positonClip.y + 1.0f) * 0.5f * aScreenDimensions.y;
	
	if (aOutScreenPoint.x < 0.0f || aOutScreenPoint.x > aScreenDimensions.x || aOutScreenPoint.y < 0.0f || aOutScreenPoint.y > aScreenDimensions.y)
	{
		return false;
	}

	return true;
}

//bool KE::Camera::NDCtoWorldSpace(const Vector2f& aPoint, OUT Vector3f& aOutWP)
//{
//	DirectX::XMVECTOR wp = { aPoint.x, aPoint.y, 0.0f, 1.0f };
//
//	wp = DirectX::XMVector4Transform(wp, DirectX::XMMatrixInverse(nullptr, myProjectionMatrix));
//
//	// Dude I HATE DirectX math library. Who in their right mind thought it was a good idea to use this in our engine? Everything is hidded behind a shit func or some other bullshit that you cant see.
//	wp.m128_f32[0] /= wp.m128_f32[3];
//	wp.m128_f32[1] /= wp.m128_f32[3];
//	wp.m128_f32[2] /= wp.m128_f32[3];
//
//	wp = DirectX::XMVector4Transform(wp, GetViewMatrix());
//
//	aOutWP = { wp.m128_f32[0], wp.m128_f32[1], wp.m128_f32[2] };
//
//	return true;
//
//	// float4 clipSpacePos = float4(UVtoNDC(uv), depth, 1.0f);
//	// 
//	// float4 viewSpacePosition = mul(projMatrixInverse, clipSpacePos);
//	// 
//	// viewSpacePosition /= viewSpacePosition.w;
//	// 
//	// return mul(viewMatrixInverse, viewSpacePosition);
//}

void KE::Camera::Rotate2D(float aX, float aY)
{
	const float pitch = aY * 0.005f;
	const float yaw	  =	aX * 0.005f;


	transform.RotateLocal(Vector3f(pitch, 0.0f, 0.0f));
	transform.RotateWorld(Vector3f(0.0f, yaw, 0.0f));
}
