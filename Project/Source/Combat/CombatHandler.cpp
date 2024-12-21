#include "stdafx.h"
#include "CombatHandler.h"

#include <Engine/Source/Collision/CollisionHandler.h>
#include <Engine/Source/Collision/RaycastHandler.h>
#include <Engine/Source/Collision/Collider.h>
#include <Engine/Source/Collision/Layers.h>
#include <Engine/Source/ComponentSystem/GameObject.h>

#include "Project/Source/Combat/IDamageable.h"
#include "Project/Source/Combat/Hitboxh.h"

static KE::CollisionHandler* collisionHandler = nullptr;
static KE::RaycastHandler* raycastHandler = nullptr;

CombatHandler::CombatHandler() {}
CombatHandler::~CombatHandler() {}

void CombatHandler::Init(KE::CollisionHandler& aCollisionHandler, KE::RaycastHandler& aRaycastHandler)
{
	collisionHandler = &aCollisionHandler;
}

bool CombatHandler::Hit(const HitBox& aHitbox)
{
	bool objectHit = false;

	int layer = 0;
	layer = layer | static_cast<int>(KE::Collision::Layers::Player);
	std::vector<KE::Collider*> hitObjects = collisionHandler->BoxCast(aHitbox.box, layer);

	for (int i = 0; i < hitObjects.size(); i++)
	{
		std::vector<KE::Component*> components = hitObjects[i]->myComponent->GetGameObject().GetComponentsRaw();

		for (int j = 0; j < components.size(); j++)
		{
			if (IDamageable* hitObject = dynamic_cast<IDamageable*>(components[j]))
			{
				objectHit = true;
				hitObject->OnHit(aHitbox.combatData);
			}
		}
	}

	return objectHit;
}