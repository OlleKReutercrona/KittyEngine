#include "stdafx.h"
#include "WaterComponent.h"

#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/ModelComponent.h"
#include "Engine/Source/Graphics/Graphics.h"


P8::WaterPlane::WaterPlane(KE::GameObject& aGO): Component(aGO)
{
	auto* graphics = KE_GLOBAL::blackboard.Get<KE::Graphics>();


	//cbuffer
	{
		
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.ByteWidth = sizeof(WaterBufferData);
		bufferDesc.StructureByteStride = 0u;

		waterBuffer.Init(graphics->GetDevice().Get(), &bufferDesc);
	}
	//


	waterModel.myMeshList = &graphics->GetModelLoader().Load("Data/ProjectAssets/waterPlane.fbx");
	waterModel.myTransform = &myGameObject.myWorldSpaceTransform.GetMatrix();
	auto& renderResource = waterModel.myRenderResources.emplace_back();

	renderResource.myVertexShader = graphics->GetShaderLoader().GetVertexShader(
		SHADER_LOAD_PATH "PondWater_VS.cso"
	);
	renderResource.myPixelShader = graphics->GetShaderLoader().GetPixelShader(
		SHADER_LOAD_PATH "PondWater_PS.cso"
	);

	renderResource.myMaterial = graphics->GetTextureLoader().GetDefaultMaterial();

	renderResource.myCBuffer = &waterBuffer;
	renderResource.myCBufferPSSlot = 7;
}

P8::WaterPlane::~WaterPlane()
{
}

void P8::WaterPlane::Awake()
{
	auto* graphics = KE_GLOBAL::blackboard.Get<KE::Graphics>();
	graphics->AddWaterModel(&waterModel);
}

void P8::WaterPlane::SetData(void* aData)
{
	waterBufferData = *static_cast<WaterBufferData*>(aData);
}

void P8::WaterPlane::Update()
{
	UpdateBuffer();
}

void P8::WaterPlane::UpdateBuffer()
{
	auto* graphics = KE_GLOBAL::blackboard.Get<KE::Graphics>();
	waterBuffer.MapBuffer(&waterBufferData, sizeof(waterBufferData), graphics->GetContext().Get());
}

void P8::WaterPlane::OnDestroy()
{
	auto* graphics = KE_GLOBAL::blackboard.Get<KE::Graphics>();
	graphics->ClearWaterModels();
}
