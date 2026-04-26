#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"

namespace P8
{
	class SpawnPointComponent : public KE::Component
	{
	private:
		int index = 0;

	public:
		SpawnPointComponent(KE::GameObject& aGameObject);

		void SetData(void* aDataObject = nullptr) override;

		int GetIndex() const { return index; }
	};
}
