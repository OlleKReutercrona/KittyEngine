#include "stdafx.h"
#include "NavmeshBuilder.h"
#include <Engine/Source/Graphics/Graphics.h>
#include <Engine/Source/Files/KittyMesh.h>

KE::KittyMesh NavmeshBuilder::Build(std::vector<Transform>& someTransforms)
{
    KE::KittyMesh out;

    KE::Mesh* mesh;
    KE::Graphics* gfx = KE_GLOBAL::blackboard.Get<KE::Graphics>("graphics");
    auto& meshList = gfx->GetModelLoader().Load("Data/EngineAssets/NavmeshPlane.fbx");
    mesh = &meshList.myMeshes[0];
    
    std::vector<Vector3f> vertices;
    std::vector<unsigned int> indices;
    
    // Create triangles from the navmesh plane and save to indices and vertices.
    {
        int count = 0;
    
        for (auto& transform : someTransforms)
        {
            Vector4f vert0World = Vector4f(mesh->myVertices[0].x, mesh->myVertices[0].y, mesh->myVertices[0].z, 1.0f) * transform.GetCUMatrix();
            Vector4f vert1World = Vector4f(mesh->myVertices[1].x, mesh->myVertices[1].y, mesh->myVertices[1].z, 1.0f) * transform.GetCUMatrix();
            Vector4f vert2World = Vector4f(mesh->myVertices[2].x, mesh->myVertices[2].y, mesh->myVertices[2].z, 1.0f) * transform.GetCUMatrix();
            Vector4f vert3World = Vector4f(mesh->myVertices[3].x, mesh->myVertices[3].y, mesh->myVertices[3].z, 1.0f) * transform.GetCUMatrix();
    
            vertices.push_back({ vert0World.x, vert0World.y, vert0World.z });
            vertices.push_back({ vert1World.x, vert1World.y, vert1World.z });
            vertices.push_back({ vert2World.x, vert2World.y, vert2World.z });
            vertices.push_back({ vert3World.x, vert3World.y, vert3World.z });
    
            indices.push_back(0 + count * 4);
            indices.push_back(1 + count * 4);
            indices.push_back(2 + count * 4);
    
            indices.push_back(0 + count * 4);
            indices.push_back(2 + count * 4);
            indices.push_back(3 + count * 4);
    
            count++;
        }
    }

    std::vector<Vector3f> uniqueVertices;

    for (int i = 0; i < indices.size(); i++)
    {
        const Vector3f& trueVertex = vertices[indices[i]];

        bool foundMatching = false;
        for (int j = 0; j < uniqueVertices.size(); j++)
        {
            if ((trueVertex - uniqueVertices[j]).Length() < 0.05f)
            {
                indices[i] = j;
                foundMatching = true;
                break;
            }
        }

        if (!foundMatching)
        {
            uniqueVertices.push_back(trueVertex);
            indices[i] = static_cast<unsigned int>(uniqueVertices.size()) - 1;
        }
    }

    size_t ttt = uniqueVertices.size();

    out.myVertices = uniqueVertices;
    out.myIndices = indices;

#pragma region OLD

    //std::vector<Vector3f> cleanedVertices;
    //std::vector<unsigned int> cleanedIndices;
    //std::unordered_map<unsigned int, unsigned int> indexSwapMap;
    //
    //// Save the index of where the duplicate vertex was found.
    //for (int i = 0; i < vertices.size(); i++)
    //{
    //    for (int j = 0; j < i; j++)
    //    {
    //        const float distance = (vertices[i] - vertices[j]).Length();
    //
    //        if (distance < 0.05f)
    //        {
    //            if (indexSwapMap.find(j) == indexSwapMap.end())
    //            {
    //                indexSwapMap[j] = i;
    //            }
    //        }
    //    }
    //}
    //
    //for (int i = 0; i < indices.size(); i++)
    //{
    //    if (indexSwapMap.find(indices[i]) != indexSwapMap.end())
    //    {
    //        cleanedIndices.push_back(indexSwapMap[indices[i]]);
    //    }
    //    else
    //    {
    //        cleanedIndices.push_back(indices[i]);
    //    }
    //}
    //
    //std::vector<bool> usedVertices(vertices.size());
    //
    //// Identify vertices that are not being used
    //for (int i = 0; i < cleanedIndices.size(); i++)
    //{
    //    usedVertices[cleanedIndices[i]] = true;
    //}
    //
    ////std::vector<Vector3f> cleanedWorldVertices;
    //std::vector<unsigned int> cleanedIndexVector;
    //
    //cleanedIndexVector = cleanedIndices;
    //
    //for (unsigned int i = 0; i < vertices.size(); i++)
    //{
    //    if (usedVertices[i])
    //    {
    //        cleanedVertices.push_back(vertices[i]);
	//	}
    //    else
    //    {
    //        for (auto& index : cleanedIndexVector)
    //        {
    //            if (index >= i)
    //            {
	//				index--;
	//			}
	//		}
    //    }
	//}
    //
    //out.myVertices = cleanedVertices;
    //out.myIndices = cleanedIndexVector;
    //
    //std::cout << "World vertices: " << vertices.size() << std::endl;
    //for (int i = 0; i < vertices.size(); i++)
    //{
    //    std::cout << "obj " << i / 4 << "World vertex " << i << ": " << vertices[i].x << ", " << vertices[i].y << std::endl;
    //}

#pragma endregion
    return out;
}