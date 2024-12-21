#include "stdafx.h"
#include "ComponentSystem/GameObject.h"
#include "Engine/Source/Graphics/FX/VFX.h"
#include "VFXComponent.h"

KE::VFXComponent::VFXComponent(GameObject& aGameObject) : Component(aGameObject)
{
}

KE::VFXComponent::~VFXComponent()
{
}

void KE::VFXComponent::Awake()
{
	//if (shouldAutoPlay)
	//{
	//	for (auto& index : myVFXSequenceIndices)
	//	{
	//		VFXRenderInput in(myGameObject.myWorldSpaceTransform, true, false);
	//		myVFXManager->TriggerVFXSequence(index, in);
	//	}
	//}
}

void KE::VFXComponent::Update()
{

}

void KE::VFXComponent::OnDestroy()
{
	StopAllVFX();
}

void KE::VFXComponent::TriggerAllVFX(bool aLooping, bool aStationary)
{
	for (auto& index : myVFXSequenceIndices)
	{
		VFXRenderInput in(myGameObject.myWorldSpaceTransform, true, false);
		myVFXManager->TriggerVFXSequence(index, in);
	}
}

void KE::VFXComponent::StopAllVFX()
{
	for (auto& index : myVFXSequenceIndices)
	{
		VFXRenderInput in(myGameObject.myWorldSpaceTransform);
		myVFXManager->StopVFXSequence(index, in);
	}
}

void KE::VFXComponent::TriggerVFX(int anIndex, bool aLooping, bool aStationary)
{
	if (myVFXSequenceIndices.size() <= anIndex)
	{
		KE_ERROR("VFXComponent::TriggerVFX: Index %i out of range", anIndex);
		return;
	}
	VFXRenderInput in(myGameObject.myWorldSpaceTransform, aLooping, aStationary);
	myVFXManager->TriggerVFXSequence(myVFXSequenceIndices[anIndex], in);
}

void KE::VFXComponent::TriggerVFXAt(int anIndex, const Vector3f& aPosition, bool aLooping)
{
	if (myVFXSequenceIndices.size() <= anIndex)
	{
		KE_ERROR("VFXComponent::TriggerVFXAt: Index %i out of range", anIndex);
		return;
	}
	Transform t(aPosition);
	VFXRenderInput in(t, aLooping, true);
	myVFXManager->TriggerVFXSequence(myVFXSequenceIndices[anIndex], in);
}

void KE::VFXComponent::TriggerVFXCustom(int anIndex, const VFXRenderInput& anInput)
{
	if (myVFXSequenceIndices.size() <= anIndex)
	{
		KE_ERROR("VFXComponent::TriggerVFXCustom: Index %i out of range", anIndex);
		return;
	}
	myVFXManager->TriggerVFXSequence(myVFXSequenceIndices[anIndex], anInput);
}

void KE::VFXComponent::StopVFX(int anIndex)
{
	if (myVFXSequenceIndices.size() <= anIndex)
	{
		KE_ERROR("VFXComponent::StopVFX: Index %i out of range", anIndex);
		return;
	}
	VFXRenderInput in(myGameObject.myWorldSpaceTransform);
	myVFXManager->StopVFXSequence(myVFXSequenceIndices[anIndex], in);
}

void KE::VFXComponent::StopVFXCustom(int anIndex, const VFXRenderInput& anInput)
{
	if (myVFXSequenceIndices.size() <= anIndex)
	{
		KE_ERROR("VFXComponent::StopVFXCustom: Index %i out of range", anIndex);
		return;
	}
	myVFXManager->StopVFXSequence(myVFXSequenceIndices[anIndex], anInput);
}

void KE::VFXComponent::SetData(void* aData)
{
	VFXComponentData& data = *(VFXComponentData*)aData;
	myVFXManager = data.myVFXManager;
	shouldAutoPlay = data.myAutoPlay;
	SetSequences(data.myVFXNames);
}

void KE::VFXComponent::SetSequences(const std::vector<std::string>& someVFXSequenceNames)
{
	myVFXSequenceIndices.clear();
	for (auto& name : someVFXSequenceNames)
	{
		myVFXSequenceIndices.push_back(myVFXManager->GetVFXSequenceFromName(name));
	}
}

void KE::VFXComponent::OnSceneChange()
{
	myVFXManager->ClearVFX();
}
