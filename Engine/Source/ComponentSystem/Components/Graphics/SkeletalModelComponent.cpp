#include "stdafx.h"
#include "SkeletalModelComponent.h"
#include "Engine/Source/Graphics/Animation/AnimationPlayer.h"
#include "Engine/Source/Graphics/DebugRenderer.h"
#include "Engine/Source/SceneManagement/Scene.h"

KE::SkeletalModelComponent::SkeletalModelComponent(GameObject& aGameObject) : Component(aGameObject) {}

KE::SkeletalModelComponent::~SkeletalModelComponent() 
{
	myModelData->myActiveStatus = false;
}

void KE::SkeletalModelComponent::Update()
{
	myAnimationPlayer.Animate(KE_GLOBAL::deltaTime);

	//RenderSkeleton();
	// TODO Someone sets the scale to 1.0f somewhere so this is an ugly hack to fix that
	//myGameObject.myTransform.SetScale({0.01f, 0.01f, 0.01f});
}

void KE::SkeletalModelComponent::LateUpdate()
{
	myAnimationPlayer.UpdateWorldSpaceJoints();
}

KE::SkeletalModelData* KE::SkeletalModelComponent::GetModelData() const
{
	return myModelData;
}

void KE::SkeletalModelComponent::SetData(void* aDataObject)
{
	SkeletalModelComponentData& data = *(SkeletalModelComponentData*)aDataObject;
	myModelData = data.skeletalModelData;

	myAnimationPlayer.Init(myModelData);
}

void KE::SkeletalModelComponent::RenderSkeleton()
{

}
