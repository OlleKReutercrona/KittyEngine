#pragma once
#include "Component.h"

namespace KE
{
	struct AttachComponentData
	{
		Transform* attachedTransform = nullptr;
	};

	class AttachComponent : public Component
	{
	private:
		Transform* myAttachedTransform = nullptr;

	public:
		AttachComponent(GameObject& aParentGameObject);
		~AttachComponent();

		void SetData(void* aDataObject) override;
		void DefaultData() override;

		void Awake() override;
		void Update() override;
	};

}
