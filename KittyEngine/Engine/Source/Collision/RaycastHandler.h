#pragma once
#include <Engine/Source/Math/Vector2.h>
#include <Engine/Source/Math/Vector3.h>

namespace KE
{
	class CollisionHandler;
	class Camera;

	class RaycastHandler
	{
	public:
		RaycastHandler() = default;
		~RaycastHandler() = default;

		void Init(Camera& aSceneCamera, CollisionHandler& aCollisionHandler);

		// Returns a valid position within a triangle.
		// NOTE: If no valid position within a triangle was found it returns a 0 length vector.
		Vector3f Raycast(Vector2f aMousePos, Vector3f aPlayerPosition);
		bool RayCastMissed(Vector2f aMousePos, Vector3f aPlayerPosition);
		Vector3f RaycastPlayerAction(Vector2f aMousePos, Vector3f aPlayerPosition);
		Vector3f RaycastEnemy(Vector2f aMousePos);
		bool RayCastEnemyHit(Vector2f aMousePos);
		void AssignCamera(Camera& aCamera);

	private:
		Camera* mySceneCamera = nullptr;
		CollisionHandler* myCollisionHandler = nullptr;
	};

}