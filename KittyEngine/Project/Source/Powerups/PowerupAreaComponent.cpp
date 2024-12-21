#include "stdafx.h"

#include "PowerupAreaComponent.h"

#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/GameObjectManager.h"
#include "Engine/Source/Graphics/DebugRenderer.h"
#include "Player/Player.h"

void P8::PowerupAreaComponent::SetData(void* aDataObject)
{
	auto* data = static_cast<PowerupAreaData*>(aDataObject);

	myMin = data->min;
	myMax = data->max;
}

void P8::PowerupAreaComponent::Awake()
{

}

void P8::PowerupAreaComponent::Update()
{
	
}

void P8::PowerupAreaComponent::OnEnable()
{
	
}

void P8::PowerupAreaComponent::OnDisable()
{
	
}

void P8::PowerupAreaComponent::DrawDebug(KE::DebugRenderer& aDbg)
{
	Vector3f average = (myMin + myMax) * 0.5f;
	Vector3f size = myMax - myMin;
	aDbg.RenderCube(average, size, {1.0f, 1.0f, 0.0f, 1.0f});
}
