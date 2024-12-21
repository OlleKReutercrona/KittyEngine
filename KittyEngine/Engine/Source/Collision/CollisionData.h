#pragma once

namespace KE
{
	struct CharacterControllerUserData;
	class Collider;
	class GameObject;

	struct CollisionData
	{
		CollisionData(Collider& aCol, GameObject& aGO) : hitCollider(aCol), hitGameObject(aGO) {}

		Collider& hitCollider;
		GameObject& hitGameObject;
		bool isTrigger = false;
		int state = 0;
	};

	struct PhysXCollisionData
	{
		GameObject* objHit = nullptr;


	};
}
