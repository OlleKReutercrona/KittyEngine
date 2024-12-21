#include "stdafx.h"
#include <Engine/Source/AI/Pathfinding/Navmesh.h>
#include <Engine/Source/Files/KittyMesh.h>
#include <Engine/Source/Graphics/ModelLoader.h>
#include <Engine/Source/Graphics/Graphics.h>



namespace KE
{
	void Navmesh::LoadKittyMesh(KE::KittyMesh& aFile)
	{
		myNodes.clear();
		myIndiceNodeMap.clear();
		myVertices.clear();
		myIndices.clear();

		Vector3f minBounds;
		Vector3f maxBounds;

		CreateNodes(aFile, minBounds, maxBounds);

		for (int i = 0; i < myNodes.size(); i++)
		{
			AttachNodes(&myNodes[i]);
		}

		for (int i = 0; i < myNodes.size(); i++)
		{
			GenerateLineColliders(&myNodes[i]);
		}


		myNavGrid.Init(5.0f, minBounds, maxBounds, myNodes);
	}
	void Navmesh::SaveKittyMesh(KE::KittyMesh& aFile)
	{
		aFile.myVertices = myVertices;
		aFile.myIndices = myIndices;

	}
	void Navmesh::ClearNavmesh()
	{
		myNodes.clear();
		myIndiceNodeMap.clear();
		myVertices.clear();
		myIndices.clear();
	}
	bool KE::Navmesh::Init(Graphics* aGraphics, std::string& aObjFilepath)
	{
		MeshList* meshList = &aGraphics->GetModelLoader().Load(aObjFilepath);

		if (!meshList  || meshList->myMeshes.empty()) {
			KE_ERROR("Navmesh failed to get MeshList from Graphics for file %s", aObjFilepath.c_str());
			return false;
		}

		const Mesh& mesh = meshList->myMeshes[0];

		myNodes.clear();
		myIndiceNodeMap.clear();

		CreateNodes(mesh);

		for (int i = 0; i < myNodes.size(); i++)
		{
			AttachNodes(&myNodes[i]);
		}

		for (int i = 0; i < myNodes.size(); i++)
		{
			GenerateLineColliders(&myNodes[i]);
		}

		
		myNavGrid.Init(5.0f, mesh.myBounds.min, mesh.myBounds.max, myNodes);

		return true;
	}

	void Navmesh::DebugRender(Graphics* aGraphics)
	{
		LineVertex v1;
		LineVertex v2;
		v1.colour = { 1,0,1,1 };
		v2.colour = { 1,0,1,1 };
		v1.position.w = 1.0f;
		v2.position.w = 1.0f;

		for (int i = 0; i < myNodes.size(); i++)
		{
			NavNode& node = myNodes[i];

			// <[Navmesh Triangles]> //
			{
				v1.colour = { 1,1,1,1 };
				v2.colour = { 1,1,1,1 };

				v1.position = { node.vertices[0]->x, node.vertices[0]->y, node.vertices[0]->z, 1.0f };
				v2.position = { node.vertices[1]->x, node.vertices[1]->y, node.vertices[1]->z, 1.0f };
				aGraphics->AddLine(v1, v2);

				v1.position = v2.position;
				v2.position = { node.vertices[2]->x, node.vertices[2]->y, node.vertices[2]->z, 1.0f };
				aGraphics->AddLine(v1, v2);

				v1.position = v2.position;
				v2.position = { node.vertices[0]->x, node.vertices[0]->y, node.vertices[0]->z, 1.0f };
				aGraphics->AddLine(v1, v2);
			}

			// <[Line Colliders]> //
			{
				//v1.colour = { 1,0,0,1 };
				//v2.colour = { 1,0,0,1 };
				//for (Linef& line : node.myLineColliders)
				//{
				//	v1.position = { line.myPoint.x, 2.0f, line.myPoint.y, 1.0f };
				//	v2.position = { line.myPoint.x + line.myDirection.x * line.debugLength , 2.0f, line.myPoint.y + line.myDirection.y * line.debugLength , 1.0f };
				//	aGraphics->AddLine(v1, v2);
				//
				//	Vector4f middle = {
				//		line.myPoint.x - line.myDebugPoint.x / 2.0f,
				//		2.0f,
				//		line.myPoint.y - line.myDebugPoint.y / 2.0f,
				//		1.0f
				//	};
				//	v1.position = { line.myPoint.x + line.myDirection.x * (line.debugLength / 2.0f) , 2.0f, line.myPoint.y + line.myDirection.y * (line.debugLength / 2.0f), 1.0f };
				//	v2.position = v1.position;
				//	v2.position.x += line.myNormal.x * 1.0f;
				//	v2.position.z += line.myNormal.y * 1.0f;
				//	aGraphics->AddLine(v1, v2);
				//}
			}
		}

		// <[NavGrid]> //
		{
			//v1.colour = { 0,1,0,1 };
			//v2.colour = { 0,1,0,1 };
			//v1.position.y = 7.5f;
			//v2.position.y = 7.5f;
			//for (int y = 0; y < myNavGrid.myGridSize.y + 1; y++)
			//{
			//	v1.position.x = myNavGrid.myMin.x;
			//	v1.position.z = myNavGrid.myMin.y + y * myNavGrid.myCellSize;
			//
			//	v2.position.x = myNavGrid.myMin.x + (myNavGrid.myGridSize.x) * myNavGrid.myCellSize;
			//	v2.position.z = myNavGrid.myMin.y + y * myNavGrid.myCellSize;
			//
			//	aGraphics->AddLine(v1, v2);
			//}
			//for (int x = 0; x < myNavGrid.myGridSize.x + 1; x++)
			//{
			//	v1.position.x = myNavGrid.myMin.x + x * myNavGrid.myCellSize;
			//	v1.position.z = myNavGrid.myMin.y;
			//
			//	v2.position.x = myNavGrid.myMin.x + x * myNavGrid.myCellSize;
			//	v2.position.z = myNavGrid.myMin.y + (myNavGrid.myGridSize.y) * myNavGrid.myCellSize;
			//
			//	aGraphics->AddLine(v1, v2);
			//}
		}
	}

	void Navmesh::CreateNodes(const Mesh& aMesh)
	{
		myVertices.reserve(aMesh.myVertices.size());
		myNodes.reserve(aMesh.myIndices.size() / 3);

		Vector3f vertex;
		for (int i = 0; i < aMesh.myVertices.size(); i++)
		{
			const Vertex& meshVertex = aMesh.myVertices[i];
			
			vertex = { meshVertex.x, meshVertex.y, meshVertex.z };

			myVertices.push_back(vertex);

			myIndiceNodeMap.insert(std::pair<int, NodesAttachedToIndex>(i, NodesAttachedToIndex(i)));
		}
		for (unsigned int nodeIndex = 0; nodeIndex < aMesh.myIndices.size(); nodeIndex += 3)
		{
			int id = nodeIndex / 3;
			std::array<unsigned int, 3> indices =
			{
				aMesh.myIndices[nodeIndex + 0],
				aMesh.myIndices[nodeIndex + 1],
				aMesh.myIndices[nodeIndex + 2]
			};

			myIndices.push_back(indices[0]);
			myIndices.push_back(indices[1]);
			myIndices.push_back(indices[2]);

			std::array<const Vector3f*, 3> vertices = {
				&myVertices[indices[0]],
				&myVertices[indices[1]],
				&myVertices[indices[2]]
			};

			NavNode node = NavNode(id, indices, vertices);

			myNodes.push_back(node);

			myIndiceNodeMap.at(indices[0]).vertex = *vertices[0];
			myIndiceNodeMap.at(indices[0]).nodes.push_back(&myNodes.back());
			myIndiceNodeMap.at(indices[1]).vertex = *vertices[1];
			myIndiceNodeMap.at(indices[1]).nodes.push_back(&myNodes.back());
			myIndiceNodeMap.at(indices[2]).vertex = *vertices[2];
			myIndiceNodeMap.at(indices[2]).nodes.push_back(&myNodes.back());
		}
	}

	void Navmesh::CreateNodes(const KE::KittyMesh& aMesh, Vector3f& aMin, Vector3f& aMax)
	{
		aMin = { FLT_MAX, FLT_MAX, FLT_MAX };
		aMax = { FLT_MIN, FLT_MIN, FLT_MIN };
		Vector3f vertex;

		for (int i = 0; i < aMesh.myVertices.size(); i++)
		{
			vertex = aMesh.myVertices[i];

			myVertices.push_back(vertex);
			myIndiceNodeMap.insert(std::pair<int, NodesAttachedToIndex>(i, NodesAttachedToIndex(i)));

			if (vertex.x > aMax.x) { aMax.x = vertex.x; }
			if (vertex.y > aMax.y) { aMax.y = vertex.y; }
			if (vertex.z > aMax.z) { aMax.z = vertex.z; }

			if (vertex.x < aMin.x) { aMin.x = vertex.x; }
			if (vertex.y < aMin.y) { aMin.y = vertex.y; }
			if (vertex.z < aMin.z) { aMin.z = vertex.z; }
		}
		for (unsigned int nodeIndex = 0; nodeIndex < aMesh.myIndices.size(); nodeIndex += 3)
		{
			int id = nodeIndex / 3;
			std::array<unsigned int, 3> indices =
			{
				aMesh.myIndices[nodeIndex + 0],
				aMesh.myIndices[nodeIndex + 1],
				aMesh.myIndices[nodeIndex + 2]
			};

			myIndices.push_back(indices[0]);
			myIndices.push_back(indices[1]);
			myIndices.push_back(indices[2]);


			std::array<const Vector3f*, 3> vertices = {
				&myVertices[indices[0]],
				&myVertices[indices[1]],
				&myVertices[indices[2]]
			};

			NavNode node = NavNode(id, indices, vertices);

			myNodes.push_back(node);

			myIndiceNodeMap.at(indices[0]).vertex = *vertices[0];
			myIndiceNodeMap.at(indices[1]).vertex = *vertices[1];
			myIndiceNodeMap.at(indices[2]).vertex = *vertices[2];

			myIndiceNodeMap.at(indices[0]).nodes.push_back(&myNodes.back());
			myIndiceNodeMap.at(indices[1]).nodes.push_back(&myNodes.back());
			myIndiceNodeMap.at(indices[2]).nodes.push_back(&myNodes.back());
		}
	}

	void Navmesh::AttachNodes(NavNode* aNode)
	{
		for (int i = 0; i < myNodes.size(); i++)
		{
			if (aNode == &myNodes[i]) continue;
			int neighbour = 0;

			for (unsigned int vertex : myNodes[i].indices)
			{
				for (auto aNodeVert : aNode->indices)
				{
					if (vertex == aNodeVert)
					{
						neighbour++;
					}
				}
			}
			if (neighbour > 1)
			{
				aNode->myNeighbours.push_back(&myNodes[i]);
			}
		}
	}

	void Navmesh::GenerateLineColliders(NavNode* aNode)
	{
		bool line0_shared = false;
		bool line1_shared = false;
		bool line2_shared = false;

		std::array<unsigned int, 2> line0 = { aNode->indices[0], aNode->indices[1] };
		std::array<unsigned int, 2> line1 = { aNode->indices[1], aNode->indices[2] };
		std::array<unsigned int, 2> line2 = { aNode->indices[2], aNode->indices[0] };

		for (int i = 0; i < aNode->myNeighbours.size(); i++)
		{
			NavNode& neighbour = *aNode->myNeighbours[i];

			for (unsigned int indice : neighbour.indices)
			{
				if (GotIndices(line0, neighbour)) line0_shared = true;
				if (GotIndices(line1, neighbour)) line1_shared = true;
				if (GotIndices(line2, neighbour)) line2_shared = true;
			}
		}

		if (!line0_shared)
		{
			Vector2f v0 = { myVertices[line0[0]].x, myVertices[line0[0]].z };
			Vector2f v1 = { myVertices[line0[1]].x, myVertices[line0[1]].z };
			Linef line = Linef(v0, v1);

			Vector2f v2_to_check = { myVertices[aNode->indices[2]].x, myVertices[aNode->indices[2]].z };

			if (line.GetNormal().Dot(v2_to_check - v0) < 0)
			{
				line.FlipNormal();
			}

			aNode->myLineColliders.push_back(line);
		}
		if (!line1_shared)
		{
			Vector2f v0 = { myVertices[line1[0]].x, myVertices[line1[0]].z };
			Vector2f v1 = { myVertices[line1[1]].x, myVertices[line1[1]].z };
			Linef line = Linef(v0, v1);

			Vector2f v2_to_check = { myVertices[aNode->indices[0]].x, myVertices[aNode->indices[0]].z };

			if (line.GetNormal().Dot(v2_to_check - v0) < 0)
			{
				line.FlipNormal();
			}

			aNode->myLineColliders.push_back(line);
		}
		if (!line2_shared)
		{
			Vector2f v0 = { myVertices[line2[0]].x, myVertices[line2[0]].z };
			Vector2f v1 = { myVertices[line2[1]].x, myVertices[line2[1]].z };
			Linef line = Linef(v0, v1);

			Vector2f v2_to_check = { myVertices[aNode->indices[1]].x, myVertices[aNode->indices[1]].z };

			if (line.GetNormal().Dot(v2_to_check - v0) < 0)
			{
				line.FlipNormal();
			}

			aNode->myLineColliders.push_back(line);
		}
	}

	bool Navmesh::GotIndices(std::array<unsigned int, 2> aLine, NavNode& aNode)
	{
		int count = 0;
		for (unsigned int indice : aNode.indices)
		{
			if (indice == aLine[0] || indice == aLine[1])
			{
				count++;
			}
		}

		return (count < 2);
	}
}