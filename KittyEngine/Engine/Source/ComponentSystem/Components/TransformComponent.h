#pragma once
#include "ComponentSystem/Component.h"
#include "Math/Transform.h"

namespace KE
{
	class GameObject;

	class TransformComponent : public KE::Component
	{
	public:
		TransformComponent(KE::GameObject& aGameObject);
		~TransformComponent();

		Transform myTransform;
	private:
	};
}