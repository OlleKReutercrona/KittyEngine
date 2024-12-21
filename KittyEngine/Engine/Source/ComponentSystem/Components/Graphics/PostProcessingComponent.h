#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Graphics/PostProcessing.h"

namespace KE
{
	struct PostProcessingComponentData
	{
		KE::PostProcessing* postProcessing;
	};

	class GameObject;

	class PostProcessingComponent : public Component
	{
	public:
		PostProcessingComponent(GameObject& aGameObject);
		~PostProcessingComponent() override;

		void Awake() override {}
		void LateUpdate() override {}
		void Update() override {}

		void OnDestroy() override {}
		//void OnTrigger() override {}
		//void OnCollision() override {};

		/*void SetActive(const bool aValue) override;*/

		void OnEnable() override {}
		void OnDisable() override {}


		void SetData(void* aData) override;

		PostProcessing* GetPostProcessing() const;
	private:
		PostProcessing* myPostProcessing;
	};
}
