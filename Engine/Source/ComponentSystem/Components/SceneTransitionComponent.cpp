#include "stdafx.h"
#include "SceneTransitionComponent.h"
#include "Collision\CollisionData.h"
#include "Collision\Layers.h"
#include "ComponentSystem\GameObject.h"
#include "ComponentSystem\GameObjectManager.h"
#include "SceneManagement\Scene.h"

KE::SceneTransitionComponent::SceneTransitionComponent(KE::GameObject& aGameObject) : KE::Component(aGameObject)
{
}

KE::SceneTransitionComponent::~SceneTransitionComponent()
{
}

void KE::SceneTransitionComponent::OnTriggerEnter(const CollisionData& aCollisionData)
{
	if (!KE::Collision::IsOnLayer(aCollisionData.hitGameObject.myLayer, KE::Collision::Layers::Player)) return;

	myGameObject.GetManager().GetScene()->SetLevelFromBuildIndex(mySceneIndex);
}

void KE::SceneTransitionComponent::OnCollisionEnter(const CollisionData& aCollisionData)
{
	if (!KE::Collision::IsOnLayer(aCollisionData.hitGameObject.myLayer, KE::Collision::Layers::Player)) return;

	myGameObject.GetManager().GetScene()->SetLevelFromBuildIndex(mySceneIndex);
}

void KE::SceneTransitionComponent::SetData(void* someData)
{
	SceneTransitionComponentData* data = (SceneTransitionComponentData*)someData;

	mySceneIndex = data->sceneIndex;
}
