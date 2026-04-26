#pragma once
#include "Project/Source/Combat/IDamageable.h"
#include "Engine/Source/Collision/Shape.h"
#include "Engine/Source/Graphics/DebugRenderer.h"

namespace KE
{
	class Shape;
}

class HitBox
{
public:
	HitBox(const Vector3f& aPosition, const Vector3f aOffset, float aSize) : 
		box(aSize, aPosition)
	{
		box.myOffset = aOffset;
	}

	void DebugRender()
	{
		KE::DebugRenderer& dbg = *KE_GLOBAL::blackboard.Get<KE::DebugRenderer>("debugRenderer");

		Vector3f position = box.myPosition + box.myOffset;
		Vector3i size = { 1, 1, 1 };

		dbg.RenderCube(position, size);
	}

	KE::Box box;
	int layerMask = (int)KE::Collision::Layers::Default;
	CombatData combatData;
};