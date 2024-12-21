#include "stdafx.h"
#include "Engine/Source/Graphics/ModelData.h"
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/Renderers/InstancedRenderer.h"

#include <d3d11.h>
#include "Engine/Source/Graphics/Graphics.h"
#include "imgui/imgui.h"

void KE::InstancedRenderer::RenderPackage(const InstancedRenderPackage& aPackage, const InstanceRenderInput& aInput)
{
	if (aPackage.modelDataIndices.empty()) { return; }
	if (aPackage.renderResources.empty())  { return; }
	
	//prepare some resources we'll want
	const ModelDataList& modelDataList = myGraphics->GetModelData();
	const RenderResourceList& renderResources = aPackage.renderResources;
	size_t instanceCount = aPackage.modelDataIndices.size();
	const auto& graphicsContext = myGraphics->GetContext();
	auto& shaderLoader = myGraphics->GetShaderLoader();

	std::vector<InstancedRenderData> instanceData;
	instanceData.reserve(instanceCount);

	for (size_t i = 0; i < instanceCount; i++)
	{
		if (!modelDataList[aPackage.modelDataIndices[i]].myActiveStatus) { continue; }


		auto transform = *modelDataList[aPackage.modelDataIndices[i]].myTransform;
		Transform t = transform;

		Transform camT = myGraphics->GetCameraManager().GetHighlightedCamera()->transform;
		Vector3f modelPosition = t.GetPositionRef();
		Vector3f cameraPosition = camT.GetPositionRef();

		Vector3f direction = (cameraPosition - modelPosition).GetNormalized();
		Vector3f cameraForward = camT.GetForward();

		instanceData.push_back(transform);
	}

	instanceCount = instanceData.size();
	if (instanceCount == 0) { return; }

	//ComPtr<ID3D11Buffer> instanceBuffer;
	myInstanceBuffer.MapBuffer(instanceData.data(), static_cast<int>(sizeof(InstancedRenderData) * instanceCount), graphicsContext.Get());

	//CreateInstanceBuffer(aInput, instanceCount, instanceData, instanceBuffer);
	//

	const KE::MeshList* meshList = modelDataList[aPackage.modelDataIndices[0]].myMeshList;
	for (size_t instance = 0; instance < meshList->myMeshes.size(); instance++)
	{
		const size_t renderResourceIndex = instance >= renderResources.size() ? 0 : instance;
		const Material* material = renderResources[instance >= renderResources.size() ? 0 : instance].myMaterial;

		const auto* vertexShader = aInput.overrideVertexShader ?
			aInput.overrideVertexShader : //use overriden shader if it exists
			renderResources[renderResourceIndex].myVertexShader;

		const auto* pixelShader = aInput.overridePixelShader ?
			aInput.overridePixelShader : //use overriden shader if it exists
			renderResources[renderResourceIndex].myPixelShader;

		const KE::Mesh* mesh = &meshList->myMeshes[instance];

		//

		unsigned int strides[2];
		unsigned int offsets[2];
		ID3D11Buffer* bufferPointers[2];

		strides[0] = sizeof(Vertex);
		strides[1] = sizeof(InstancedRenderData);

		offsets[0] = 0;
		offsets[1] = 0;

		bufferPointers[0] = mesh->myVertexBuffer.Get();
		bufferPointers[1] = myInstanceBuffer.GetBuffer().Get();


		myGraphics->BindMaterial(material, 0u);
		graphicsContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
		graphicsContext->IASetIndexBuffer(mesh->myIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		graphicsContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		graphicsContext->VSSetShader(vertexShader->GetShader(), nullptr, 0u);
		graphicsContext->IASetInputLayout(vertexShader->GetInputLayout());
		graphicsContext->PSSetShader(pixelShader->GetShader(), nullptr, 0u);



		graphicsContext->DrawIndexedInstanced((UINT)mesh->myIndices.size(), (UINT)instanceCount, 0, 0, 0);
		myGraphics->AddDrawCall();
	}

}

void KE::InstancedRenderer::Init(Graphics* aGraphics)
{
	Renderer::Init(aGraphics);

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.ByteWidth = sizeof(InstanceRenderBufferData);
		bufferDesc.StructureByteStride = 0u;

		myRenderingBuffer.Init(aGraphics->GetDevice(), &bufferDesc);
	}

	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.ByteWidth = sizeof(InstancedRenderData) * (UINT)INSTANCING_BUFFER_SIZE;
		bufferDesc.StructureByteStride = sizeof(InstancedRenderData);

		myInstanceBuffer.Init(aGraphics->GetDevice(), &bufferDesc);
	}
}

void KE::InstancedRenderer::SortRenderPackages(InstancedRenderPackageList* aPackageList)
{
	const Vector3f& cameraPosition = myGraphics->GetCameraManager().GetHighlightedCamera()->transform.GetPositionRef();
	const auto& modelDatas = myGraphics->GetModelData();

	for (size_t i = 0; i < aPackageList->size(); i++)
	{
		InstancedRenderPackage& package = aPackageList->at(i);

		float nearestDistance = FLT_MAX;
		for (size_t j = 0; j < package.modelDataIndices.size(); j++)
		{
			const ModelData& modelData = modelDatas[package.modelDataIndices[j]];
			const Vector3f& modelPosition = ((Transform*)modelData.myTransform)->GetPositionRef();

			float distance = (cameraPosition - modelPosition).LengthSqr();
			if (distance < nearestDistance)
			{
				nearestDistance = distance;
			}
		}

		package.distanceToCamera = nearestDistance;
	}

	//sort the packages based on distance
	std::sort(aPackageList->begin(), aPackageList->end(), [](const InstancedRenderPackage& a, const InstancedRenderPackage& b)
	{
		return a.distanceToCamera < b.distanceToCamera;
	});

	//if (ImGui::Begin("Sorted Result"))
	//{
	//	for (size_t i = 0; i < aPackageList->size(); i++)
	//	{
	//		ImGui::Text("Package %d: %f (%s)", i, aPackageList->at(i).distanceToCamera, modelDatas[aPackageList->at(i).modelDataIndices[0]].myMeshList->myFilePath.c_str());
	//	}
	//}
	//ImGui::End();

}

void KE::InstancedRenderer::GenerateInstancingData(const std::vector<size_t>& aModelDataIndices, const ModelDataList& aModelDatas, InstancedRenderPackageList* aOutRenderPackages)
{
	std::unordered_map<KE::MeshList*, InstancedRenderPackageList> instanceBuilderMap;

	for (const size_t& i : aModelDataIndices)
	{
		//if (aModelDatas[i].myActiveStatus != true) { continue; }
		if (aModelDatas[i].myIsInstanced != true) { continue; }


		KE::MeshList* meshList = aModelDatas[i].myMeshList;
		const RenderResourceList& renderResources = aModelDatas[i].myRenderResources;

		if (instanceBuilderMap.find(meshList) == instanceBuilderMap.end())
		{
			instanceBuilderMap[meshList] = InstancedRenderPackageList();
		}


		bool foundPackage = false;
		for (InstancedRenderPackage& package : instanceBuilderMap[meshList])
		{
			if (package.TryAddModelData(i, renderResources))
			{
				foundPackage = true;
				break;
			}
		}
		if (!foundPackage)
		{
			InstancedRenderPackage& newPackage = instanceBuilderMap[meshList].emplace_back();
			newPackage.renderResources = renderResources;
			newPackage.modelDataIndices.push_back(i);
		}
	}
		

	for (auto& pair : instanceBuilderMap)
	{
		for (InstancedRenderPackage& package : pair.second)
		{
			aOutRenderPackages->push_back(package);
		}
	}
	
}

void KE::InstancedRenderer::Render(const InstanceRenderInput& aInput)
{
	InstanceRenderBufferData bufferData;
	bufferData.worldToClip = aInput.viewMatrix * aInput.projectionMatrix;

	myRenderingBuffer.MapBuffer(&bufferData, sizeof(bufferData), myGraphics->GetContext().Get());
	myRenderingBuffer.BindForVS(1u, myGraphics->GetContext().Get());

	//if (ImGui::IsKeyDown(ImGuiKey_K))
	//{
	//	SortRenderPackages(aInput.renderPackages);
	//}

	for (InstancedRenderPackage& package : *aInput.renderPackages)
	{
		RenderPackage(package, aInput);
	}
}
