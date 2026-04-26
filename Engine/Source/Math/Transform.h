#pragma once
#include "Matrix4x4.h"
#include "Vector3.h"

#include <DirectXMath.h>
#include <External/Include/physx/foundation/PxTransform.h>

class Transform
{

public:
	Transform(
		Vector3f aPosition = Vector3f(0.0f,0.0f,0.0f), 
		Vector3f aRotation = Vector3f(0.0f, 0.0f, 0.0f),
		Vector3f aScale = Vector3f(1.0f, 1.0f, 1.0f)
	)
	{
		myMatrix = DirectX::XMMatrixIdentity();
		SetPosition(aPosition);
		SetRotation(aRotation);
		SetScale(aScale);
	}

	Transform(const DirectX::XMMATRIX& aMatrix)
	{
		myMatrix = aMatrix;
	}

	Transform(const physx::PxTransform& apxTransform)
	{
		myMatrix = DirectX::XMMatrixRotationQuaternion({ apxTransform.q.x,apxTransform.q.y ,apxTransform.q.z,apxTransform.q.w });
		SetPosition({ apxTransform.p.x,apxTransform.p.y, apxTransform.p.z });
	}

	void operator=(const physx::PxTransform& apxTransform)
	{
		myMatrix = DirectX::XMMatrixRotationQuaternion({ apxTransform.q.x,apxTransform.q.y ,apxTransform.q.z,apxTransform.q.w });
		SetPosition({ apxTransform.p.x,apxTransform.p.y, apxTransform.p.z });
	}

	operator physx::PxTransform() const
	{
		Transform temp = *this;
		temp.SetScale({1.0f, 1.0f, 1.0f});
		temp.SetPosition({0.0f, 0.0f, 0.0f});

		auto quat = DirectX::XMQuaternionRotationMatrix(temp.GetMatrix());
		auto pos = GetPosition();

		return physx::PxTransform(pos.x, pos.y, pos.z, { quat.m128_f32[0],quat.m128_f32[1], quat.m128_f32[2], quat.m128_f32[3] });
	}

	inline void SetMatrix(const DirectX::XMMATRIX& aMatrix) { myMatrix = aMatrix; };

	inline DirectX::XMMATRIX& GetMatrix() { return myMatrix; };
	inline const DirectX::XMMATRIX& GetMatrix() const { return myMatrix; };

	inline Matrix4x4f GetCUMatrix() const { return Matrix4x4f(myMatrix); };

	//
	Transform operator*(const Transform aTransform)
	{
		Transform transform;
		transform.myMatrix = myMatrix * aTransform.myMatrix;
		return transform;
	}

	//*= operator
	Transform& operator*=(const Transform aTransform)
	{
		myMatrix = myMatrix * aTransform.myMatrix;
		return *this;
	}

	bool operator!=(const Transform& aTransform)
	{
		return  myMatrix.r[0].m128_f32[0] != aTransform.myMatrix.r[0].m128_f32[0] ||
				myMatrix.r[0].m128_f32[1] != aTransform.myMatrix.r[0].m128_f32[1] ||
				myMatrix.r[0].m128_f32[2] != aTransform.myMatrix.r[0].m128_f32[2] ||
				myMatrix.r[0].m128_f32[3] != aTransform.myMatrix.r[0].m128_f32[3] ||
				myMatrix.r[1].m128_f32[0] != aTransform.myMatrix.r[1].m128_f32[0] ||
				myMatrix.r[1].m128_f32[1] != aTransform.myMatrix.r[1].m128_f32[1] ||
				myMatrix.r[1].m128_f32[2] != aTransform.myMatrix.r[1].m128_f32[2] ||
				myMatrix.r[1].m128_f32[3] != aTransform.myMatrix.r[1].m128_f32[3] ||
				myMatrix.r[2].m128_f32[0] != aTransform.myMatrix.r[2].m128_f32[0] ||
				myMatrix.r[2].m128_f32[1] != aTransform.myMatrix.r[2].m128_f32[1] ||
				myMatrix.r[2].m128_f32[2] != aTransform.myMatrix.r[2].m128_f32[2] ||
				myMatrix.r[2].m128_f32[3] != aTransform.myMatrix.r[2].m128_f32[3] ||
				myMatrix.r[3].m128_f32[0] != aTransform.myMatrix.r[3].m128_f32[0] ||
				myMatrix.r[3].m128_f32[1] != aTransform.myMatrix.r[3].m128_f32[1] ||
				myMatrix.r[3].m128_f32[2] != aTransform.myMatrix.r[3].m128_f32[2] ||
				myMatrix.r[3].m128_f32[3] != aTransform.myMatrix.r[3].m128_f32[3];
	}

	bool operator==(const Transform& aTransform)
	{
		return myMatrix.r[0].m128_f32[0] == aTransform.myMatrix.r[0].m128_f32[0] &&
				myMatrix.r[0].m128_f32[1] == aTransform.myMatrix.r[0].m128_f32[1] &&
				myMatrix.r[0].m128_f32[2] == aTransform.myMatrix.r[0].m128_f32[2] &&
				myMatrix.r[0].m128_f32[3] == aTransform.myMatrix.r[0].m128_f32[3] &&
				myMatrix.r[1].m128_f32[0] == aTransform.myMatrix.r[1].m128_f32[0] &&
				myMatrix.r[1].m128_f32[1] == aTransform.myMatrix.r[1].m128_f32[1] &&
				myMatrix.r[1].m128_f32[2] == aTransform.myMatrix.r[1].m128_f32[2] &&
				myMatrix.r[1].m128_f32[3] == aTransform.myMatrix.r[1].m128_f32[3] &&
				myMatrix.r[2].m128_f32[0] == aTransform.myMatrix.r[2].m128_f32[0] &&
				myMatrix.r[2].m128_f32[1] == aTransform.myMatrix.r[2].m128_f32[1] &&
				myMatrix.r[2].m128_f32[2] == aTransform.myMatrix.r[2].m128_f32[2] &&
				myMatrix.r[2].m128_f32[3] == aTransform.myMatrix.r[2].m128_f32[3] &&
				myMatrix.r[3].m128_f32[0] == aTransform.myMatrix.r[3].m128_f32[0] &&
				myMatrix.r[3].m128_f32[1] == aTransform.myMatrix.r[3].m128_f32[1] &&
				myMatrix.r[3].m128_f32[2] == aTransform.myMatrix.r[3].m128_f32[2] &&
				myMatrix.r[3].m128_f32[3] == aTransform.myMatrix.r[3].m128_f32[3];
	}

	Transform operator+(const Vector3f aPosition)
	{
		Transform temp(myMatrix);

		Vector3f pos = temp.GetPosition();
		pos += aPosition;

		temp.SetPosition(pos);
		return temp;
	}

	inline Vector3f GetForward() const
	{
		return Vector3f(myMatrix.r[2]).GetNormalized();
	}
	inline Vector3f GetRight() const
	{
		return Vector3f(myMatrix.r[0]).GetNormalized();
	}
	inline Vector3f GetUp() const
	{
		return Vector3f(myMatrix.r[1]).GetNormalized();
	}

	inline Vector3f GetForwardUnnormalized() const
	{
		return Vector3f(myMatrix.r[2]);
	}

	inline Vector3f GetRightUnnormalized() const
	{
		return Vector3f(myMatrix.r[0]);
	}

	inline Vector3f GetUpUnnormalized() const
	{
		return Vector3f(myMatrix.r[1]);
	}
	//

	inline const Vector3f GetPosition() const
	{
		return myMatrix.r[3];
	}
	
	inline Vector3f& GetPositionRef()
	{
		return *(Vector3f*)&myMatrix.r[3];
	}

	inline void SetPosition(const Vector3f& aPosition)
	{
		myMatrix.r[3].m128_f32[0] = aPosition.x;
		myMatrix.r[3].m128_f32[1] = aPosition.y;
		myMatrix.r[3].m128_f32[2] = aPosition.z;
	}

	inline void FucKShitUp(const float spriteWidth, const float spriteHeight, const Vector3f& aRotation)
	{
		DirectX::XMMATRIX pivotTranslation = DirectX::XMMatrixTranslation(-spriteWidth, -spriteHeight, 0.0f);
		DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(aRotation.x, aRotation.y, aRotation.z);
		myMatrix = pivotTranslation * rotation * DirectX::XMMatrixTranslation(spriteWidth, spriteHeight, 0.0f) * myMatrix;
	}
	//

	inline void LookAt(const Vector3f& aPosition)
	{
		Vector3f forward = (aPosition - GetPosition()).GetNormalized();
		Vector3f right = Vector3f(0.0f, 1.0f, 0.0f).Cross(forward).GetNormalized();
		Vector3f up = forward.Cross(right).GetNormalized();

		myMatrix.r[0].m128_f32[0] = right.x;
		myMatrix.r[0].m128_f32[1] = right.y;
		myMatrix.r[0].m128_f32[2] = right.z;

		myMatrix.r[1].m128_f32[0] = up.x;
		myMatrix.r[1].m128_f32[1] = up.y;
		myMatrix.r[1].m128_f32[2] = up.z;

		myMatrix.r[2].m128_f32[0] = forward.x;
		myMatrix.r[2].m128_f32[1] = forward.y;
		myMatrix.r[2].m128_f32[2] = forward.z;
	}

	inline void SetDirection(const Vector3f& aDirection)
	{
		LookAt(GetPosition() + aDirection);
	}

	inline void SetHorizontalDirection(const Vector3f& aDirection)
	{
		LookAt(GetPosition() + Vector3f(aDirection.x, 0.0f, aDirection.z));
	}

	//I don't really think you should use this one. It probably won't work as you expect
	inline void SetRotation(const Vector3f& aRotation)
	{
		Vector3f pos = GetPosition();
		Vector3f scale = GetScale();

		myMatrix = DirectX::XMMatrixRotationRollPitchYaw(aRotation.x, aRotation.y, aRotation.z);

		SetPosition(pos);
		SetScale(scale);
	}
	
	//This is sort of an operation that shouldn't be done, but it's here for convenience
	inline Vector3f GetRotation() const
	{
		Vector3f rotation;

		Vector3f forwardRotation = GetForward();

		rotation.x = atan2f(forwardRotation.y, forwardRotation.z);
		rotation.y = atan2f(-forwardRotation.x, sqrtf(forwardRotation.y * forwardRotation.y + forwardRotation.z * forwardRotation.z));
		rotation.z = atan2f(GetUp().x, GetRight().x);

		return rotation;
	}


	inline Vector3f GetScale() const
	{
		Vector3f scale;

		scale.x = GetRightUnnormalized().Length();
		scale.y = GetUpUnnormalized().Length();
		scale.z = GetForwardUnnormalized().Length();

		return scale;
	}

	inline void SetScale(const Vector3f& aScale)
	{
		const float minScale = 0.001f;

		Vector3f right = GetRight();
		Vector3f up = GetUp();
		Vector3f forward = GetForward();

		right = right * (aScale.x < minScale ? minScale : aScale.x);
		up = up * (aScale.y < minScale ? minScale : aScale.y);
		forward = forward * (aScale.z < minScale ? minScale : aScale.z);

		myMatrix.r[0].m128_f32[0] = right.x;
		myMatrix.r[0].m128_f32[1] = right.y;
		myMatrix.r[0].m128_f32[2] = right.z;

		myMatrix.r[1].m128_f32[0] = up.x;
		myMatrix.r[1].m128_f32[1] = up.y;
		myMatrix.r[1].m128_f32[2] = up.z;

		myMatrix.r[2].m128_f32[0] = forward.x;
		myMatrix.r[2].m128_f32[1] = forward.y;
		myMatrix.r[2].m128_f32[2] = forward.z;
	}
	//

	inline void TranslateWorld(const Vector3f& aMoveAmount)
	{
		Vector3f pos = myMatrix.r[3];
		SetPosition(pos + aMoveAmount);
	}

	inline void TranslateLocal(const Vector3f& aMoveAmount)
	{
		Vector3f forward = GetForward();
		Vector3f right = GetRight();
		Vector3f up = GetUp();

		Vector3f moveAmount = forward * aMoveAmount.z + right * aMoveAmount.x + up * aMoveAmount.y;

		TranslateWorld(moveAmount);
	}

	inline void RotateWorld(const Vector3f& aRotation)
	{
		Vector3f pos = GetPosition();
		SetPosition(Vector3f(0.0f, 0.0f, 0.0f));
		myMatrix = myMatrix * DirectX::XMMatrixRotationRollPitchYaw(aRotation.x, aRotation.y, aRotation.z);
		SetPosition(pos);

	}

	inline void RotateLocal(const Vector3f& aRotation)
	{
		myMatrix = DirectX::XMMatrixRotationRollPitchYaw(aRotation.x, aRotation.y, aRotation.z) * myMatrix;
	}

	inline void SetRotation(float aOrientationRad)
	{
		DirectX::XMVECTOR translation, rotation, scale;
		DirectX::XMMatrixDecompose(&scale, &rotation, &translation, myMatrix);

		rotation = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), aOrientationRad);


		myMatrix = DirectX::XMMatrixAffineTransformation(scale, DirectX::XMVectorZero(), rotation, translation);

	}

private:
	DirectX::XMMATRIX myMatrix;
};

