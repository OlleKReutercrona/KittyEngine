#pragma once

#include "Engine/Source/Graphics/ModelData.h"
#include "Engine/Source/ComponentSystem/Components/Component.h"

namespace KE
{
	struct ModelComponentData
	{
		ModelData* modelData;
	};

	class DebugRenderer;

	class GameObject;

	class ModelComponent : public Component
	{
	public:
		ModelComponent(GameObject& aGameObject);
		~ModelComponent() override;

		void LateUpdate() override {};

		void OnDestroy() override {};
		//void OnTrigger() override {};
		//void OnCollision() override {};

		/*void SetActive(const bool aValue) override;*/

		void OnEnable() override { myModelData->myActiveStatus = true; }
		void OnDisable() override { myModelData->myActiveStatus = false; }

		//if you use this function, you are so stupid!
		void SetModelData(ModelData& aModelData);
		ModelData* GetModelData() const;

		void SetData(void* aDataObject = nullptr) override;
		void DefaultData() override;
	private:
		ModelData* myModelData; //this works for now i guess but it is unsustainable
		//Graphics* myGraphics = nullptr;
		//int myModelDataIndex = -1;
	};
}
