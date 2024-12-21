#include "stdafx.h"
#include "DeferredPointLight.h"
#include "Engine\Source\Graphics\Primitives\SpherePrimitive.h"
#include <d3d11.h>

KE::DeferredPointLight::DeferredPointLight()
{
	SpherePrimitive sphere;
	auto sphereVertices = sphere.GetVertices();
	auto sphereIndices = sphere.GetIndices();

	std::vector<KE::Vertex> vertices;
	for (size_t i = 0; i < sphereVertices.size(); i++)
	{
		Vertex& vertex = vertices.emplace_back();
		vertex.x = sphereVertices[i].x;
		vertex.y = sphereVertices[i].y;
		vertex.z = sphereVertices[i].z;
	}

	std::vector<unsigned int> indices;
	for (size_t i = 0; i < sphereIndices.size(); i++)
	{
		indices.push_back(sphereIndices[i]);
	}

	myModelData.myMeshList = &myMesh;
#ifndef KITTYENGINE_NO_EDITOR
	myMesh.myFilePath = "Generated Mesh";
#endif

	Mesh mesh = myMesh.myMeshes.emplace_back();
	mesh.myVertices = vertices;
	mesh.myIndices = indices;

	myModelData.myTransform = &myTransform.GetMatrix();

	// TODO -> Set up modeldata with shader and test render
	auto resource = myModelData.myRenderResources.emplace_back();
	//resource.myPixelShader = "_PS.cso";
	//resource.myVertexShader = "_VS.cso";


	// Only for testing
	myTransform.SetPosition({ 5.0f, 2.0f, 5.0f });
	myTransform.SetScale({ 10.0f, 10.0f, 10.0f });
}
