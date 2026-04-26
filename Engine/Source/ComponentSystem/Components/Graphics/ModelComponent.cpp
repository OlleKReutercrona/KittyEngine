#include "stdafx.h"
#include "ModelComponent.h"
#include "Engine/Source/Graphics/Graphics.h"
#include "ComponentSystem/GameObject.h"
#include "CameraComponent.h"

#include <Engine\Source\Graphics\DebugRenderer.h>
#include <Engine\Source\SceneManagement\Scene.h>
#include <Engine\Source\ComponentSystem\GameObjectManager.h>

KE::ModelComponent::ModelComponent(GameObject& aGameObject) : Component(aGameObject)
{
}

KE::ModelComponent::~ModelComponent()
{
}

void KE::ModelComponent::SetModelData(ModelData& aModelData)
{
	myModelData = &aModelData;
}

KE::ModelData* KE::ModelComponent::GetModelData() const
{
	return myModelData;
}

void KE::ModelComponent::SetData(void* aData)
{
	ModelComponentData& data = *(ModelComponentData*)aData;
	myModelData = data.modelData;
}

void KE::ModelComponent::DefaultData()
{
	Graphics* gfx = KE_GLOBAL::blackboard.Get<Graphics>("graphics");
	myModelData = gfx->CreateModelData("Data/EngineAssets/Cube.fbx");
	myModelData->myTransform = &myGameObject.myTransform.GetMatrix();
}
