#pragma once

#include "Engine/Source/Graphics/ModelData.h"
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Graphics/Animation/AnimationPlayer.h"

namespace KE
{
	struct SkeletalModelComponentData
	{
		SkeletalModelData* skeletalModelData;
	};

	class GameObject;

	class SkeletalModelComponent : public Component
	{
	public:
		SkeletalModelComponent(GameObject& aGameObject);
		virtual ~SkeletalModelComponent() override;

		virtual void Awake() override {}
		virtual void LateUpdate() override;
		virtual void Update() override;

		SkeletalModelData* GetModelData() const;

		AnimationPlayer& GetAnimationPlayer() { return myAnimationPlayer; }

		virtual void SetData(void* aDataObject = nullptr) override;

		void OnEnable() override { myModelData->myActiveStatus = true; }
		void OnDisable() override { myModelData->myActiveStatus = false; }

		void RenderSkeleton();

	private:
		SkeletalModelData* myModelData;

		// TODO AnimationPlayer could be an AnimationComponent?
		AnimationPlayer myAnimationPlayer;
	};
}
