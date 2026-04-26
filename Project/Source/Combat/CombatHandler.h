#pragma once

namespace KE
{
	class CollisionHandler;
	class RaycastHandler;
	class Shape;
}
class HitBox;

class CombatHandler
{
public:
	CombatHandler();
	~CombatHandler();

	static void Init(KE::CollisionHandler& aCollisionHandler, KE::RaycastHandler& aRaycastHandler);
	static bool Hit(const HitBox& aHitbox);
};
