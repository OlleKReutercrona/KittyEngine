#include "stdafx.h"
#include "LightComponent.h"

#include "ComponentSystem/GameObject.h"
#include "ComponentSystem/GameObjectManager.h"
#include "SceneManagement/Scene.h"
#include "Graphics/DebugRenderer.h"
#include "Graphics/Lighting/DeferredLightManager.h"

namespace KE
{
	LightComponent::LightComponent(GameObject& aGameObject) : Component(aGameObject) 
	{ 
		
	}
	LightComponent::~LightComponent() = default;

	void LightComponent::SetData(void* aDataObject)
	{
		const LightComponentData* data = static_cast<LightComponentData*>(aDataObject);
		myLightComponentData = *data;
		myManager = data->myLightManager;
		UpdateLightData();
	}

	void LightComponent::Update()
	{
		UpdateLightData();

		// This should be handled in DrawDebug // DR
		//if (myIsDebugDraw)
		//{
		//	switch (myLightComponentData.myLightType)
		//	{
		//	case eLightType::Directional:
		//		break;
		//	case eLightType::Point: 
		//	{
		//		PointLightData* data = (PointLightData*)myLightComponentData.myLightData;
		//		float range = data->myRange;
		//		myGameObject.GetManager().GetScene()->myDebugRenderer.RenderSphere(
		//			myGameObject.myTransform.GetPosition(), range);
		//		break;
		//	}
		//	case eLightType::Spot:
		//		break;
		//	default:
		//		break;
		//	}
		//}
	}

	void LightComponent::ToggleDrawDebug()
	{
		myIsDebugDraw = !myIsDebugDraw;
	}

	void LightComponent::DrawDebug(KE::DebugRenderer& aDbg)
	{
		switch (myLightComponentData.myLightType)
		{
		case eLightType::Directional:
		{
			DirectionalLightData* lightData = static_cast<DirectionalLightData*>(myLightComponentData.myLightData);
			lightData->myDirection = myGameObject.myTransform.GetForward() * -1.0f;
		}
		break;
		case eLightType::Point:
		{
			//PointLightData* lightData = static_cast<PointLightData*>(myLightComponentData.myLightData);
			//lightData->myPosition = myGameObject.myWorldSpaceTransform.GetPosition();
			//
			//aDbg.RenderSphere(myGameObject.myWorldSpaceTransform.GetPositionRef(), lightData->myRange);
		}
		break;
		case eLightType::Spot:
		{
			SpotLightData* lightData = static_cast<SpotLightData*>(myLightComponentData.myLightData);
			lightData->myDirection = myGameObject.myTransform.GetForward() * -1.0f;
			lightData->myPosition = myGameObject.myWorldSpaceTransform.GetPosition();

			aDbg.RenderCone(
				myGameObject.myWorldSpaceTransform.GetPosition(), 
				Vector3f(-lightData->myDirection.x, -lightData->myDirection.y, -lightData->myDirection.z),
				lightData->myRange, 
				lightData->myOuterAngle,
				lightData->myInnerAngle
				);
		}
		break;
		}


	}

	void LightComponent::UpdateLightData() const
	{
		switch (myLightComponentData.myLightType)
		{
			case eLightType::Directional:
			{
				DirectionalLightData* lightData = static_cast<DirectionalLightData*>(myLightComponentData.myLightData);
				lightData->myDirection = myGameObject.myTransform.GetForward() * -1.0f;
			}
			break;
			case eLightType::Point:
			{
				PointLightData* lightData = static_cast<PointLightData*>(myLightComponentData.myLightData);
				lightData->myPosition = myGameObject.myWorldSpaceTransform.GetPosition();
			}
			break;
			case eLightType::Spot:
			{
				SpotLightData* lightData = static_cast<SpotLightData*>(myLightComponentData.myLightData);
				lightData->myDirection = myGameObject.myTransform.GetForward() * -1.0f;
				lightData->myPosition = myGameObject.myWorldSpaceTransform.GetPosition();
			}
			break;
		}
	}

	void LightComponent::OnEnable()
	{
		if (myLightComponentData.myLightType == eLightType::Point)
		{
			static_cast<PointLightData*>(myLightComponentData.myLightData)->isActive = 1;
		}
		else if (myLightComponentData.myLightType == eLightType::Spot)
		{
			static_cast<SpotLightData*>(myLightComponentData.myLightData)->isActive = 1;
		}
	}

	void LightComponent::OnDisable()
	{
		if (myLightComponentData.myLightType == eLightType::Point)
		{
			static_cast<PointLightData*>(myLightComponentData.myLightData)->isActive = 0;
		}
		else if (myLightComponentData.myLightType == eLightType::Spot)
		{
			static_cast<SpotLightData*>(myLightComponentData.myLightData)->isActive = 0;
		}
	}

	void LightComponent::OnDestroy()
	{
		myManager->RemoveLightData(myLightComponentData.myLightType, myLightComponentData.myLightData);
	}
}
