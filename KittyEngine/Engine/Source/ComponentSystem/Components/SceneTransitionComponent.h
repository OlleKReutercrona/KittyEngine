#pragma once
#include "Component.h"
namespace KE
{
	struct SceneTransitionComponentData
	{
		int sceneIndex;
	};

	class SceneTransitionComponent :public Component
	{
	public:
		SceneTransitionComponent(KE::GameObject& aGameObject);
		~SceneTransitionComponent();

		void OnTriggerEnter(const CollisionData& aCollisionData) override;
		void OnCollisionEnter(const CollisionData& aCollisionData) override;


		void SetData(void* someData) override;
	private:
		int mySceneIndex = 0;
	};
}