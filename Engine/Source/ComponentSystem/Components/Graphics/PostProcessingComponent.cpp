#include "stdafx.h"
#include "PostProcessingComponent.h"

KE::PostProcessingComponent::PostProcessingComponent(GameObject& aGameObject)
	: Component(aGameObject) 
	, myPostProcessing(nullptr)
{

}

KE::PostProcessingComponent::~PostProcessingComponent()
{
}

void KE::PostProcessingComponent::SetData(void* aData)
{
	myPostProcessing = static_cast<PostProcessingComponentData*>(aData)->postProcessing;
}

KE::PostProcessing* KE::PostProcessingComponent::GetPostProcessing() const
{
	return myPostProcessing;
}
