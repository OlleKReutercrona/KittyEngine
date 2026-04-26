#include "stdafx.h"
#include "SpawnPointComponent.h"

P8::SpawnPointComponent::SpawnPointComponent(KE::GameObject& aGameObject) : KE::Component(aGameObject)
{
}

void P8::SpawnPointComponent::SetData(void* aDataObject)
{
	index = *static_cast<int*>(aDataObject);
}
