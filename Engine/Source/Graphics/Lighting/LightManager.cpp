#include "stdafx.h"
#include "LightManager.h"

#include <string>

#include "Lighting.h"
#include "ComponentSystem/Components/LightComponent.h"
#include "Utility/Logging.h"

namespace KE
{
	LightManager::LightManager(ID3D11Device* aDevice, const UINT aSlot)
		:
		mySlot(aSlot)
	{
		myLightComponentData.reserve(MAX_LIGHT_VECTOR_CAPACITY);
		myLightData.reserve(MAX_LIGHT_VECTOR_CAPACITY);

		D3D11_BUFFER_DESC cbd = {};
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(myLightBuffer);
		cbd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &myLightBuffer;
		const HRESULT result = aDevice->CreateBuffer(&cbd, &csd, &myConstantBuffer);
		if (FAILED(result))
		{
			KE_ERROR("Failed to create Light constant buffer!");
		}
	}

	LightManager::~LightManager()
	{
		for (const LightData* data : myLightData)
		{
			delete data;
		}
	}

	void LightManager::UpdateBuffer(ID3D11DeviceContext* aContext)
	{
		for (const LightComponentData& lightComponentData : myLightComponentData)
		{
			const UINT index = lightComponentData.myIndex;
			const UINT typeIndex = lightComponentData.myTypeIndex;

			switch (lightComponentData.myLightType)
			{
				case eLightType::Directional:
				{
					const DirectionalLightData* data = static_cast<DirectionalLightData*>(myLightData[index]);
					myLightBuffer.myDirectionalLightData.myDirection = data->myDirection;
					myLightBuffer.myDirectionalLightData.myColour = data->myColour;
					myLightBuffer.myDirectionalLightData.myDirectionalLightIntensity = data->
						myDirectionalLightIntensity;
					myLightBuffer.myDirectionalLightData.myAmbientLightIntensity = data->myAmbientLightIntensity;
				}
				break;
				case eLightType::Point:
				{
					const PointLightData* data = static_cast<PointLightData*>(myLightData[index]);
					myLightBuffer.myPointLightData[typeIndex].myPosition = data->myPosition;
					myLightBuffer.myPointLightData[typeIndex].myColour = data->myColour;
					myLightBuffer.myPointLightData[typeIndex].myIntensity = data->myIntensity;
					myLightBuffer.myPointLightData[typeIndex].myRange = data->myRange;
					myLightBuffer.myPointLightData[typeIndex].isActive = data->isActive;
				}
				break;
				case eLightType::Spot:
				{
					const SpotLightData* data = static_cast<SpotLightData*>(myLightData[index]);
					myLightBuffer.mySpotLightData[typeIndex].myPosition = data->myPosition;
					myLightBuffer.mySpotLightData[typeIndex].myDirection = data->myDirection;
					myLightBuffer.mySpotLightData[typeIndex].myColour = data->myColour;
					myLightBuffer.mySpotLightData[typeIndex].myIntensity = data->myIntensity;
					myLightBuffer.mySpotLightData[typeIndex].myRange = data->myRange;
					myLightBuffer.mySpotLightData[typeIndex].myInnerAngle = data->myInnerAngle;
					myLightBuffer.mySpotLightData[typeIndex].myOuterAngle = data->myOuterAngle;
					myLightBuffer.mySpotLightData[typeIndex].isActive = data->isActive;
				}
				break;
				default: ;
			}
		}

		myLightBuffer.myNumberOfPointLights = myNumberOfPointLights;
		myLightBuffer.myNumberOfSpotLights = myNumberOfSpotLights;

		D3D11_MAPPED_SUBRESOURCE msr;
		aContext->Map(
			myConstantBuffer.Get(), 0u,
			D3D11_MAP_WRITE_DISCARD, 0u,
			&msr
		);
		memcpy(msr.pData, &myLightBuffer, sizeof(myLightBuffer));
		aContext->Unmap(myConstantBuffer.Get(), 0u);
	}

	void LightManager::BindBuffer(ID3D11DeviceContext* aContext)
	{
		UpdateBuffer(aContext);

		aContext->PSSetConstantBuffers(mySlot, 1u, myConstantBuffer.GetAddressOf());
	}

	LightComponentData& LightManager::CreateLightData(const eLightType aLightType)
	{
		const UINT index = static_cast<UINT>(myLightComponentData.size());
		myLightComponentData.emplace_back();
#ifdef _DEBUG
		assert(index <= MAX_LIGHT_VECTOR_CAPACITY && "Light vector had to resize beyond max lights");
#endif

		switch (aLightType)
		{
			case eLightType::Directional:
			{
				if (myNumberOfDirectionalLights > 0u)
				{
					KE_ERROR("There is more than one Directional Light!");
				}

				myLightData.push_back(new DirectionalLightData());
				myLightComponentData.back().myLightData = myLightData.back();
				myLightComponentData.back().myLightType = aLightType;
				myLightComponentData.back().myLightTypeName = "Directional Light";
				myLightComponentData.back().myIndex = index;
				myLightComponentData.back().myTypeIndex = myNumberOfDirectionalLights;

				++myNumberOfDirectionalLights;
			}
			break;
			case eLightType::Point:
			{
				myLightData.push_back(new PointLightData());
				myLightComponentData.back().myLightData = myLightData.back();
				myLightComponentData.back().myLightType = aLightType;
				myLightComponentData.back().myLightTypeName = "Point Light";
				myLightComponentData.back().myIndex = index;
				myLightComponentData.back().myTypeIndex = myNumberOfPointLights;

				++myNumberOfPointLights;
			}
			break;
			case eLightType::Spot:
			{
				myLightData.push_back(new SpotLightData());
				myLightComponentData.back().myLightData = myLightData.back();
				myLightComponentData.back().myLightType = aLightType;
				myLightComponentData.back().myLightTypeName = "Spot Light";
				myLightComponentData.back().myIndex = index;
				myLightComponentData.back().myTypeIndex = myNumberOfSpotLights;

				++myNumberOfSpotLights;
			}
			break;
		}
		return myLightComponentData.back();
	}

	void LightManager::Clear()
	{
		myNumberOfDirectionalLights = 0u;
		myNumberOfPointLights = 0u;
		myNumberOfSpotLights = 0u;

		for (const LightData* data : myLightData)
		{
			delete data;
		}
		myLightData.clear();
		myLightComponentData.clear();
	}
}
