#include "stdafx.h"
#include "DecalManager.h"

#include <d3d11.h>
#include <wrl/client.h>

#include "Graphics/Graphics.h"

KE::DecalManager::DecalManager()
{
}

KE::DecalManager::~DecalManager()
{
}

void KE::DecalManager::Init(Graphics* aGraphics)
{
	myMeshList = &aGraphics->GetModelLoader().Load("Data/EngineAssets/Cube.fbx");

	KE_GLOBAL::blackboard.Register("decalManager", this);
}

int KE::DecalManager::CreateDecal(Material* aMaterial, const Transform& aTransform)
{
	if (myFreeDecalIndices.size() > 0)
	{
		int index = myFreeDecalIndices.back();
		myFreeDecalIndices.pop_back();
		myDecals[index] = { aMaterial, aTransform, DecalActiveState::Active, {1.0f, 1.0f,1.0f,1.0f}};
		return index;
	}

	myDecals.push_back({ aMaterial, aTransform });
	return (int)myDecals.size() - 1;
}

void KE::DecalManager::DestroyDecal(int aIndex)
{
	if (aIndex < 0 || aIndex >= myDecals.size()) { return; }
	myDecals[aIndex].myActiveState = DecalActiveState::Destroyed;
	myFreeDecalIndices.push_back(aIndex);
}

KE::Decal* KE::DecalManager::GetDecal(int aIndex)
{
	if (aIndex < 0 || aIndex >= myDecals.size()) { return nullptr; }
	if (myDecals[aIndex].myActiveState == DecalActiveState::Destroyed) { return nullptr; }
		
	return &myDecals[aIndex];
}

void KE::DecalManager::PrepareDecalRendering(Graphics* aGraphics, GBuffer* aWorkingGBuffer, GBuffer* aCopyGBuffer)
{
	auto* context = aGraphics->GetContext().Get();

	aCopyGBuffer->CopyFrom(context, aWorkingGBuffer);
	aCopyGBuffer->SetAllAsResources(context, 0u);
}
