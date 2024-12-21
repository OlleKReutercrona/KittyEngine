#pragma once

#include "Engine/Source/Math/Transform.h"
#include "Engine\Source\Math\Vector.h"
#include "Engine\Source\Math\Ray.h"

namespace KE
{
	class CameraManager;
	class Collider;
	class GameObject;

	enum class ProjectionType : char
	{
		Perspective,
		Orthographic,
		Count
	};
	inline const char* EnumToString(ProjectionType aType)
	{
		switch (aType)
		{
			case ProjectionType::Perspective: return "Perspective";
			case ProjectionType::Orthographic: return "Orthographic";
			default: return "Unknown";
		}
	}


	union ProjectionData
	{
		struct
		{
			float nearPlane;
			float farPlane;

			float width;
			float height;

			float fov;
			float verticalFov;

		} perspective;

		struct
		{
			float nearPlane;
			float farPlane;

			float width;
			float height;
			
		private:
			float unused[2];

		} orthographic;
	};


	class Camera
	{
		friend class KE::CameraManager;
		friend class KE::Camera;
		KE_EDITOR_FRIEND

	public:
		Transform transform;

		Camera(int aIndex = 0);

		inline const ProjectionData& GetProjectionData() { return myProjectionData; }
		inline const ProjectionType GetType() { return myType; }

		void SetFOV(const float aFOV);
		void SetPerspective(float aWidth, float aHeight, float aFov, float aNearPlane, float aFarPlane);
		void SetOrthographic(float aWidth, float aHeight, float aNearPlane, float aFarPlane);

		const DirectX::XMMATRIX GetViewMatrix();
		const DirectX::XMMATRIX& GetProjectionMatrix();

		Rayf GetRay(const Vector2f aMousePosition);

		int GetIndex() { return myIndex; }

		bool WorldToScreenPoint(const Vector3f& aWorldPoint, Vector2f& aOutScreenPoint, Vector2f aScreenDimensions = Vector2f(-1, -1));
		//bool NDCtoWorldSpace(const Vector2f& aPoint, OUT Vector3f& aOutWP);
		//Vector3f ScreenToWorldPoint(const Vector2f& aScreenPoint);

		void Rotate2D(float aX, float aY);

	private:
		Rayf ScreenPointToRay(const Vector2f& aScreenPoint);
		DirectX::XMMATRIX myProjectionMatrix;

		ProjectionData myProjectionData;
		ProjectionType myType;
		int myIndex;
	};
}