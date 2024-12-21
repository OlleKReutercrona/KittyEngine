#include "stdafx.h"

#include "DecalComponent.h"

#include "ComponentSystem/GameObject.h"
#include "ComponentSystem/GameObjectManager.h"
#include "Graphics/DebugRenderer.h"
#include "Graphics/Decals/DecalManager.h"
#include "SceneManagement/Scene.h"

KE::DecalComponent::DecalComponent(GameObject& aParentGameObject) : Component(aParentGameObject)
{
	myDecalIndex = -1;
	myDecalManager = KE_GLOBAL::blackboard.Get<KE::DecalManager>("decalManager");
}

void KE::DecalComponent::SetData(void* aDataObject)
{
	myDecalIndex = static_cast<DecalComponentData*>(aDataObject)->myDecalIndex; 
}

void KE::DecalComponent::DefaultData()
{
	myDecalIndex = -1;
}

void KE::DecalComponent::Awake()
{
	
}

void KE::DecalComponent::LateUpdate()
{
	
}

void KE::DecalComponent::Update()
{
	if (auto* decal = myDecalManager->GetDecal(myDecalIndex))
	{
		decal->myTransform = myGameObject.myWorldSpaceTransform;
	}
}

void KE::DecalComponent::OnEnable()
{
	if (auto* decal = myDecalManager->GetDecal(myDecalIndex))
	{
		decal->myActiveState = DecalActiveState::Active;
	}
}

void KE::DecalComponent::OnDisable()
{
	if (auto* decal = myDecalManager->GetDecal(myDecalIndex))
	{
		decal->myActiveState = DecalActiveState::Inactive;
	}
}

void KE::DecalComponent::OnDestroy()
{
	myDecalManager->DestroyDecal(myDecalIndex);
}

void KE::DecalComponent::DrawDebug(KE::DebugRenderer& aDbg)
{
	if (auto* decal = myDecalManager->GetDecal(myDecalIndex))
	{
		aDbg.RenderCube(myGameObject.myWorldSpaceTransform, { 1.0f,1.0f,1.0f });
	}
}
