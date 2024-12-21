#include "stdafx.h"
#include "AttachComponent.h"

#include "ComponentSystem/GameObject.h"

KE::AttachComponent::AttachComponent(GameObject& aParentGameObject) : Component(aParentGameObject)
{
	
}

KE::AttachComponent::~AttachComponent()
{
}

void KE::AttachComponent::SetData(void* aDataObject)
{
	AttachComponentData* data = static_cast<AttachComponentData*>(aDataObject);
	myAttachedTransform = data->attachedTransform;
}

void KE::AttachComponent::DefaultData()
{
	
}

void KE::AttachComponent::Awake()
{

}

void KE::AttachComponent::Update()
{
	if (myAttachedTransform)
	{
		myGameObject.myWorldSpaceTransform = *myAttachedTransform;
		auto& children = myGameObject.GetChildren();
		for (auto& child : children)
		{
			child->myWorldSpaceTransform = child->myTransform * myGameObject.myWorldSpaceTransform;
		}
	}
}
