#include "stdafx.h"
#include "ModelLoader.h"
#include <d3d11.h>
#include <assimp\cimport.h>
#include <assimp/scene.h>

#include "Engine/Source/Utility/Logging.h"
#include "Engine\Source\Math\Vector.h"
#include <External/Include/assimp/Importer.hpp>
#include <External/Include/assimp/postprocess.h>
#include "TGAFBXImporter/source/FBXImporter.h"
#include "Utility/Timer.h"

namespace KE
{
	void FixMaterialName(std::string& aName)
	{
		if (aName.rfind("m_", 0) == 0)
		{
			aName.erase(aName.begin(), aName.begin() + 2);
		}
		//remove anything after the first : in the name

		size_t lastColon = aName.find_last_of(':');
		if (lastColon != std::string::npos)
		{
			//erase everything before the colon
			aName.erase(aName.begin(), aName.begin() + lastColon + 1);
		}
	}

	void ModelLoader::Process(const aiNode* aNode, const aiScene* aScene, MeshList& aMeshListToFill)
	{
		//aMeshListToFill.myMeshes.reserve(node->mNumMeshes);

		for (unsigned int i = 0; i < aNode->mNumMeshes; i++)
		{
			aiMesh* aiMesh = aScene->mMeshes[aNode->mMeshes[i]];


			// Get Material from .fbx
			{
				aiString matName;
				aiMaterial* mat = aScene->mMaterials[aiMesh->mMaterialIndex];

				if (AI_SUCCESS != mat->Get(AI_MATKEY_NAME, matName))
				{
					// FAILED
				}

				//std::cout << aiMesh->mName.C_Str() << " has material: " << matName.C_Str() << std::endl;

				std::string name = matName.C_Str();
				FixMaterialName(name);

				aMeshListToFill.myMaterialNames.push_back(name);
			}


			Vector3f min = { FLT_MAX, FLT_MAX, FLT_MAX };
			Vector3f max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

			Vector3f avrgPos;

			int numberOfVertices = 0;

			float radius = -FLT_MAX;


			std::vector<Vertex> vertices;
			vertices.reserve(aiMesh->mNumVertices);
			std::vector<unsigned int> indices;
			indices.reserve(aiMesh->mNumFaces * 3);

			Matrix3x3f rotationMatrix;
			/*rotationMatrix = Matrix4x4f::CreateRotationAroundY(3.14159f);*/
			rotationMatrix(1, 1) = -1.0f;
			rotationMatrix(3, 3) = -1.0f;

			//process vertices
			for (unsigned int j = 0; j < aiMesh->mNumVertices; j++)
			{
				const aiVector3D& position = aiMesh->mVertices[j];
				const aiVector3D& uv = aiMesh->mTextureCoords[0][j];

				const aiVector3D& normal = aiMesh->mNormals[j];
				const aiVector3D& tangent = aiMesh->mTangents[j];
				const aiVector3D& bitangent = aiMesh->mBitangents[j];

				Vertex vertex = {};
				vertex.x = position.x; // 100.0f;
				vertex.y = position.y; // 100.0f;
				vertex.z = position.z; // 100.0f;

				vertex.w = 1.0f;


				if (aiMesh->HasTextureCoords(0))
				{
					vertex.u = uv.x;
					vertex.v = uv.y;
				}
				else
				{
					vertex.u = 0.0f;
					vertex.v = 0.0f;
				}

				if (aiMesh->HasNormals())
				{
					vertex.nx = normal.x;
					vertex.ny = normal.y;
					vertex.nz = normal.z;
					//vertex.nw = 1.0f;
				}
				else
				{
					vertex.nx = 0.0f;
					vertex.ny = 0.0f;
					vertex.nz = 0.0f;
					//vertex.nw = 0.0f;
				}

				//vertex.nw = 1.0f;

				if (aiMesh->HasTangentsAndBitangents())
				{
					vertex.tx = tangent.x;
					vertex.ty = tangent.y;
					vertex.tz = tangent.z;
					//vertex.tw = 1.0f;

					vertex.bx = bitangent.x;
					vertex.by = bitangent.y;
					vertex.bz = bitangent.z;
					//vertex.bw = 1.0f;
				}
				else // if no tangents and bitangents, set them to zero (can we not just calculate them?)
				{
					vertex.tx = 0.0f;
					vertex.ty = 0.0f;
					vertex.tz = 0.0f;
					//vertex.tw = 0.0f;

					vertex.bx = 0.0f;
					vertex.by = 0.0f;
					vertex.bz = 0.0f;
					//vertex.bw = 0.0f;
				}

				vertex *= rotationMatrix;


				//___ CENTER PIVOT POINT ___//

				vertices.push_back(vertex);

				numberOfVertices++;
				avrgPos.x += vertex.x;
				avrgPos.y += vertex.y;
				avrgPos.z += vertex.z;


				if (vertex.x > max.x) { max.x = vertex.x; }
				if (vertex.y > max.y) { max.y = vertex.y; }
				if (vertex.z > max.z) { max.z = vertex.z; }

				if (vertex.x < min.x) { min.x = vertex.x; }
				if (vertex.y < min.y) { min.y = vertex.y; }
				if (vertex.z < min.z) { min.z = vertex.z; }
			}

			avrgPos /= (float)numberOfVertices;

			Vector3f distVec = avrgPos - max;

			radius = distVec.Length();

			//process indices

			for (unsigned int j = 0; j < aiMesh->mNumFaces; j++)
			{
				const aiFace& face = aiMesh->mFaces[j];

				for (unsigned int k = 0; k < face.mNumIndices; k++)
				{
					indices.push_back(face.mIndices[k]);
				}
			}


			Mesh& mesh = aMeshListToFill.myMeshes.emplace_back();
			mesh.myBounds.max = max;
			mesh.myBounds.min = min;
			mesh.myBounds.myBoundingRadius = radius;
			mesh.myBounds.centerPoint = avrgPos;

			// Change pivot 
			//for (size_t j = 0; j < vertices.size(); j++)
			//{
			//	vertices[j].x += avrgPos.x;
			//	vertices[j].y += avrgPos.y;
			//	vertices[j].z += avrgPos.z;
			//}


			mesh.myVertices = vertices;
			mesh.myIndices = indices;

			{
				//assign vertex buffer
				D3D11_BUFFER_DESC vertexBufferDesc = {};
				vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
				vertexBufferDesc.ByteWidth = sizeof(Vertex) * (UINT)vertices.size();
				vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				vertexBufferDesc.CPUAccessFlags = 0;
				vertexBufferDesc.MiscFlags = 0;

				D3D11_SUBRESOURCE_DATA vertexData = { 0 };
				vertexData.pSysMem = vertices.data();
				vertexData.SysMemPitch = 0;
				vertexData.SysMemSlicePitch = 0;

				HRESULT result = myDevice->CreateBuffer(&vertexBufferDesc, &vertexData,
					&mesh.myVertexBuffer);

				if (FAILED(result))
				{
					KE_ERROR("Failed to create vertex buffer.");
					//todo: error handling
				}
			}

			{
				//assign index buffer
				D3D11_BUFFER_DESC indexBufferDesc = { 0 };
				indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
				indexBufferDesc.ByteWidth = sizeof(unsigned int) * (UINT)indices.size();
				indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				indexBufferDesc.CPUAccessFlags = 0;
				indexBufferDesc.MiscFlags = 0;

				D3D11_SUBRESOURCE_DATA indexData = { 0 };
				indexData.pSysMem = indices.data();

				HRESULT result = myDevice->
					CreateBuffer(&indexBufferDesc, &indexData, &mesh.myIndexBuffer);

				if (FAILED(result))
				{
					KE_ERROR("Failed to create index buffer.");
					//todo: error handling
				}
			}
		}

		// Process child nodes recursively
		for (unsigned int i = 0; i < aNode->mNumChildren; i++)
		{
			Process(aNode->mChildren[i], aScene, aMeshListToFill);
		}
	}

	void ModelLoader::Process(const aiNode* aNode, const aiScene* aScene, SkeletalMeshList& aSkeletalMeshListToFill)
	{
		//aMeshListToFill.myMeshes.reserve(node->mNumMeshes);

		for (unsigned int i = 0; i < aNode->mNumMeshes; i++)
		{
			aiMesh* aiMesh = aScene->mMeshes[aNode->mMeshes[i]];


			// Get Material from .fbx
			{
				aiString matName;
				aiMaterial* mat = aScene->mMaterials[aiMesh->mMaterialIndex];

				if (AI_SUCCESS != mat->Get(AI_MATKEY_NAME, matName))
				{
					// FAILED
				}

				std::string name = matName.C_Str();
				FixMaterialName(name);


				aSkeletalMeshListToFill.myMaterialNames.push_back(name);
			}


			Vector3f min = { FLT_MAX, FLT_MAX, FLT_MAX };
			Vector3f max = { FLT_MIN, FLT_MIN, FLT_MIN };

			Vector3f avrgPos;

			int numberOfVertices = 0;

			float radius = FLT_MIN;


			std::vector<BoneVertex> vertices;
			//vertices.reserve(aiMesh->mNumVertices);
			std::vector<unsigned int> indices;
			//indices.reserve(aiMesh->mNumFaces * 3);

			//process vertices
			for (unsigned int j = 0; j < aiMesh->mNumVertices; j++)
			{
				const aiVector3D& position = aiMesh->mVertices[j];
				const aiVector3D& uv = aiMesh->mTextureCoords[0][j];

				const aiVector3D& normal = aiMesh->mNormals[j];
				const aiVector3D& tangent = aiMesh->mTangents[j];
				const aiVector3D& bitangent = aiMesh->mBitangents[j];

				BoneVertex vertex = {};
				vertex.x = position.x; // 100.0f;
				vertex.y = position.y; // 100.0f;
				vertex.z = position.z; // 100.0f;

				vertex.w = 1.0f;

				if (aiMesh->HasTextureCoords(0))
				{
					vertex.u = uv.x;
					vertex.v = uv.y;
				}
				else
				{
					vertex.u = 0.0f;
					vertex.v = 0.0f;
				}

				if (aiMesh->HasNormals())
				{
					vertex.nx = normal.x;
					vertex.ny = normal.y;
					vertex.nz = normal.z;
					//vertex.nw = 1.0f;
				}
				else
				{
					vertex.nx = 0.0f;
					vertex.ny = 0.0f;
					vertex.nz = 0.0f;
					//vertex.nw = 0.0f;
				}

				//vertex.nw = 1.0f;

				if (aiMesh->HasTangentsAndBitangents())
				{
					vertex.tx = tangent.x;
					vertex.ty = tangent.y;
					vertex.tz = tangent.z;
					//vertex.tw = 1.0f;

					vertex.bx = bitangent.x;
					vertex.by = bitangent.y;
					vertex.bz = bitangent.z;
					//vertex.bw = 1.0f;
				}
				else // if no tangents and bitangents, set them to zero (can we not just calculate them?)
				{
					vertex.tx = 0.0f;
					vertex.ty = 0.0f;
					vertex.tz = 0.0f;
					//vertex.tw = 0.0f;

					vertex.bx = 0.0f;
					vertex.by = 0.0f;
					vertex.bz = 0.0f;
					//vertex.bw = 0.0f;
				}

				//___ CENTER PIVOT POINT ___//

				vertices.push_back(vertex);

				numberOfVertices++;
				avrgPos.x += vertex.x;
				avrgPos.y += vertex.y;
				avrgPos.z += vertex.z;


				if (vertex.x > max.x)
				{
					max.x = vertex.x;
				}
				if (vertex.y > max.y)
				{
					max.y = vertex.y;
				}
				if (vertex.z > max.z)
				{
					max.z = vertex.z;
				}

				if (vertex.x < min.x)
				{
					min.x = vertex.x;
				}
				if (vertex.y < min.y)
				{
					min.y = vertex.y;
				}
				if (vertex.z < min.z)
				{
					min.z = vertex.z;
				}
			}

			avrgPos /= (float)numberOfVertices;

			Vector3f distVec = avrgPos - max;

			radius = distVec.Length();

			// Process bone indices.

			for (unsigned int j = 0; j < aiMesh->mNumFaces; j++)
			{
				const aiFace& face = aiMesh->mFaces[j];

				for (unsigned int k = 0; k < face.mNumIndices; k++)
				{
					indices.push_back(face.mIndices[k]);
				}
			}


			for (unsigned int j = 0; j < aiMesh->mNumBones; j++)
			{
				for (unsigned int a = 0; a < aiMesh->mBones[j]->mNumWeights; a++)
				{
					unsigned int vert = aiMesh->mBones[j]->mWeights[a].mVertexId;
					float weight = aiMesh->mBones[j]->mWeights[a].mWeight;


					for (int freeI = 0; freeI < 4; freeI++)
					{
						if (vertices[vert].myBoneIndices[freeI] == -1)
						{
							vertices[vert].myBoneIndices[freeI] = j;
							vertices[vert].myBoneWeights[freeI] = weight;
							break;
						}
					}
				}
			}

			SkeletalMesh& mesh = aSkeletalMeshListToFill.myMeshes.emplace_back();
			mesh.myBounds.max = max;
			mesh.myBounds.min = min;
			mesh.myBounds.myBoundingRadius = radius;
			mesh.myBounds.centerPoint = avrgPos;

			mesh.myVertices = vertices;
			mesh.myIndices = indices;

			{
				//assign vertex buffer
				D3D11_BUFFER_DESC vertexBufferDesc = {};
				vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
				vertexBufferDesc.ByteWidth = sizeof(BoneVertex) * (UINT)vertices.size();
				vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				vertexBufferDesc.CPUAccessFlags = 0;
				vertexBufferDesc.MiscFlags = 0;

				D3D11_SUBRESOURCE_DATA vertexData = { 0 };
				vertexData.pSysMem = vertices.data();
				vertexData.SysMemPitch = 0;
				vertexData.SysMemSlicePitch = 0;

				HRESULT result = myDevice->CreateBuffer(&vertexBufferDesc, &vertexData,
					&mesh.myVertexBuffer);

				if (FAILED(result))
				{
					KE_ERROR("Failed to create vertex buffer.");
					//todo: error handling
				}
			}

			{
				//assign index buffer
				D3D11_BUFFER_DESC indexBufferDesc = { 0 };
				indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
				indexBufferDesc.ByteWidth = sizeof(unsigned int) * (UINT)indices.size();
				indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				indexBufferDesc.CPUAccessFlags = 0;
				indexBufferDesc.MiscFlags = 0;

				D3D11_SUBRESOURCE_DATA indexData = { 0 };
				indexData.pSysMem = indices.data();

				HRESULT result = myDevice->
					CreateBuffer(&indexBufferDesc, &indexData, &mesh.myIndexBuffer);

				if (FAILED(result))
				{
					KE_ERROR("Failed to create index buffer.");
					//todo: error handling
				}
			}
		}

		// Process child nodes recursively
		for (unsigned int i = 0; i < aNode->mNumChildren; i++)
		{
			Process(aNode->mChildren[i], aScene, aSkeletalMeshListToFill);
		}
	}

	void ModelLoader::AssignBoneIndices(const aiScene* aScene, SkeletalMeshList& aMeshList)
	{
		// Go through all bones and assign indices through them.
		for (unsigned int i = 0; i < aScene->mNumMeshes; i++)
		{
			for (unsigned int j = 0; j < aScene->mMeshes[i]->mNumBones; j++)
			{
				for (unsigned int a = 0; a < aScene->mMeshes[i]->mBones[j]->mNumWeights; a++)
				{
					unsigned int vert = aScene->mMeshes[i]->mBones[j]->mWeights[a].mVertexId;
					float weight = aScene->mMeshes[i]->mBones[j]->mWeights[a].mWeight;

					aMeshList.myMeshes[i].myVertices[vert].myBoneIndices[0] = j;
					aMeshList.myMeshes[i].myVertices[vert].myBoneWeights[0] = weight;
				}
			}
		}
	}

	void ModelLoader::PreloadThread()
	{
		while (myPreloadThreadRunning)
		{
			if (myPreloadData.empty())
			{
				myPreloadThreadRunning = false;
				return;
			}

			bool allLoaded = true;
			for (auto& toLoad: myPreloadData)
			{
				if (toLoad.loaded) { continue; }
				allLoaded = false;

				switch(toLoad.type)
				{
				case PreloadData::Type::Mesh:
				{
					for (const auto& path : toLoad.filePaths)
					{
						Load(path);
					}
					toLoad.loaded = true;
					break;
				}
				case PreloadData::Type::AnimatedMesh:
				{
					SkeletalModelData loadedSkeletalModel;
					LoadSkeletalModel(loadedSkeletalModel, toLoad.filePaths[0]);

					for (size_t i = 0; i < toLoad.filePaths.size(); i++)
					{
						if (i == 0) { continue; }
						LoadAnimation(loadedSkeletalModel, toLoad.filePaths[i]);
					}
					toLoad.loaded = true;
					break;
				}
				}
			}

			if (allLoaded)
			{
				myPreloadThreadRunning = false;
				myPreloadData.clear();
				return;
			}
			
		}
	}

	ModelLoader::~ModelLoader()
	{
		myPreloadThread.join();
	}

	void ModelLoader::Clear()
	{
		for (auto& model : myMeshes)
		{
			for (auto& mesh : model.second.myMeshes)
			{
				mesh.myVertexBuffer = nullptr;
				mesh.myIndexBuffer = nullptr;
			}
		}

		for (auto& model : mySkeletalMeshes)
		{
			for (auto& mesh : model.second.myMeshes)
			{
				mesh.myVertexBuffer = nullptr;
				mesh.myIndexBuffer = nullptr;
			}
		}
	}

	void ModelLoader::Init(ID3D11Device* aDevice)
	{
		myDevice = aDevice;
		myPreloadData.reserve(128);
	}

	MeshList& ModelLoader::Load(const std::string& aFilePath)
	{
		if (myMeshes.find(aFilePath) != myMeshes.end())
		{
			return myMeshes[aFilePath];
		}


		//std::cout << "Loading model: " << aFilePath << std::endl;

		myMeshes[aFilePath] = MeshList();
		MeshList& meshList = myMeshes[aFilePath];

#ifndef KITTYENGINE_NO_EDITOR
		meshList.myFilePath = aFilePath;
#endif

		aiPropertyStore* pStore = aiCreatePropertyStore();
		aiSetImportPropertyFloat(pStore, AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f);

		Assimp::Importer importer;
		const aiScene* scene = aiImportFileExWithProperties(
			aFilePath.c_str(),
			aiProcessPreset_TargetRealtime_MaxQuality |
			aiProcess_ConvertToLeftHanded |
			aiProcess_GlobalScale,
			NULL,
			pStore);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			KE_ERROR("Failed to load model: %s", aFilePath.c_str());
			return meshList;
		}

		Process(scene->mRootNode, scene, meshList);

		aiReleaseImport(scene);
		aiReleasePropertyStore(pStore);

		return meshList;
	}

	bool ModelLoader::IsModelSkeletal(const std::string& aFilePath)
	{
		aiPropertyStore* pStore = aiCreatePropertyStore();
		aiSetImportPropertyFloat(pStore, AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f);

		Assimp::Importer importer;
		const aiScene* scene = aiImportFileExWithProperties(
			aFilePath.c_str(),
			aiProcess_FindInvalidData,
			NULL,
			pStore);

		if (!scene || !scene->mRootNode)
		{
			KE_ERROR("Failed to load model: %s", aFilePath.c_str());
			return false;
		}

		bool isSkeletal = scene->HasAnimations();

		aiReleaseImport(scene);
		aiReleasePropertyStore(pStore);

		return isSkeletal;
	}

	void ModelLoader::LoadSkeletalModel(SkeletalModelData& aOutSkeletalModel, const std::string& aFilePath)
	{
		TGA::FBXModel tgaModel;

		if (mySkeletons.contains(aFilePath))
		{
			aOutSkeletalModel.mySkeleton = &mySkeletons[aFilePath];

			if (mySkeletalMeshes.contains(aFilePath))
			{
				aOutSkeletalModel.myMeshList = &mySkeletalMeshes[aFilePath];
			}

			aOutSkeletalModel.myCombinedTransforms.resize(aOutSkeletalModel.mySkeleton->myBones.size());
			aOutSkeletalModel.myFinalTransforms.resize(aOutSkeletalModel.mySkeleton->myBones.size());
			aOutSkeletalModel.myWorldSpaceJoints.resize(aOutSkeletalModel.mySkeleton->myBones.size());

			for (unsigned int i = 0; i < aOutSkeletalModel.mySkeleton->myBones.size(); i++)
			{
				aOutSkeletalModel.myCombinedTransforms[i] = aOutSkeletalModel.mySkeleton->myBones[i].myBindPose;
				aOutSkeletalModel.myFinalTransforms[i] = aOutSkeletalModel.mySkeleton->myBones[i].myBindPose;
				aOutSkeletalModel.myWorldSpaceJoints[i] = aOutSkeletalModel.mySkeleton->myBones[i].myBindPose;
			}

			return;
		}

		if (TGA::FBXImporter::LoadModel(aFilePath, tgaModel))
		{
			mySkeletons[aFilePath] = Skeleton();
			Skeleton& skeleton = mySkeletons[aFilePath];
			aOutSkeletalModel.mySkeleton = &skeleton;

			mySkeletalMeshes[aFilePath] = SkeletalMeshList();
			SkeletalMeshList& skeletalMeshList = mySkeletalMeshes[aFilePath];
			aOutSkeletalModel.myMeshList = &skeletalMeshList;

			// Copy bone data from FBXImporter to our own model data
			for (unsigned int i = 0; i < tgaModel.Skeleton.Bones.size(); i++)
			{
				auto& bone = tgaModel.Skeleton.Bones[i];

				skeleton.myBones.emplace_back();
				auto& newBone = skeleton.myBones.back();

				// Name
				newBone.myName = bone.Name;

				// Matrix
				const auto& boneMatrix = bone.BindPoseInverse;

				newBone.myBindPose = DirectX::XMMatrixSet(
					boneMatrix.Data[0], boneMatrix.Data[1], boneMatrix.Data[2], boneMatrix.Data[3],
					boneMatrix.Data[4], boneMatrix.Data[5], boneMatrix.Data[6], boneMatrix.Data[7],
					boneMatrix.Data[8], boneMatrix.Data[9], boneMatrix.Data[10], boneMatrix.Data[11],
					boneMatrix.Data[12], boneMatrix.Data[13], boneMatrix.Data[14], boneMatrix.Data[15]
				);

				//newBone.myBindPose = DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f) * newBone.myBindPose;

				// Transpose the matrix
				newBone.myBindPose = DirectX::XMMatrixTranspose(newBone.myBindPose);

				// Parent
				newBone.myParentIndex = bone.Parent;

				// Add bone name
				skeleton.myBoneNames.push_back(bone.Name);

				aOutSkeletalModel.myCombinedTransforms.resize(skeleton.myBones.size());
				aOutSkeletalModel.myFinalTransforms.resize(skeleton.myBones.size());
				aOutSkeletalModel.myWorldSpaceJoints.resize(skeleton.myBones.size());

				// Add bone offset matrix to bind pose
				aOutSkeletalModel.myCombinedTransforms[i] = newBone.myBindPose;
				aOutSkeletalModel.myFinalTransforms[i] = newBone.myBindPose;
			}

			// Copy model data from FBXImporter to our own model data
			skeletalMeshList.myMeshes.resize(tgaModel.Meshes.size());
			for (size_t i = 0; i < skeletalMeshList.myMeshes.size(); ++i)
			{
				// Imported data
				TGA::FBXModel::FBXMesh& fbxMesh = tgaModel.Meshes[i];

				// Our own data
				SkeletalMesh& mesh = skeletalMeshList.myMeshes[i];

				mesh.myVertices.resize(fbxMesh.Vertices.size());
				mesh.myIndices = fbxMesh.Indices;

				// Copy vertex data
				for (size_t v = 0; v < mesh.myVertices.size(); ++v)
				{
					// Position
					mesh.myVertices[v].x = fbxMesh.Vertices[v].Position[0];
					mesh.myVertices[v].y = fbxMesh.Vertices[v].Position[1];
					mesh.myVertices[v].z = fbxMesh.Vertices[v].Position[2];
					mesh.myVertices[v].w = 1.0f;

					// Normals
					mesh.myVertices[v].nx = fbxMesh.Vertices[v].Normal[0];
					mesh.myVertices[v].ny = fbxMesh.Vertices[v].Normal[1];
					mesh.myVertices[v].nz = fbxMesh.Vertices[v].Normal[2];

					// Binormals
					mesh.myVertices[v].bx = fbxMesh.Vertices[v].Binormal[0];
					mesh.myVertices[v].by = fbxMesh.Vertices[v].Binormal[1];
					mesh.myVertices[v].bz = fbxMesh.Vertices[v].Binormal[2];

					// Tangents
					mesh.myVertices[v].tx = fbxMesh.Vertices[v].Tangent[0];
					mesh.myVertices[v].ty = fbxMesh.Vertices[v].Tangent[1];
					mesh.myVertices[v].tz = fbxMesh.Vertices[v].Tangent[2];

					// UVs
					mesh.myVertices[v].u = fbxMesh.Vertices[v].UVs[0][0];
					mesh.myVertices[v].v = fbxMesh.Vertices[v].UVs[0][1];

					// Bone indices
					mesh.myVertices[v].myBoneIndices[0] = fbxMesh.Vertices[v].BoneIDs[0];
					mesh.myVertices[v].myBoneIndices[1] = fbxMesh.Vertices[v].BoneIDs[1];
					mesh.myVertices[v].myBoneIndices[2] = fbxMesh.Vertices[v].BoneIDs[2];
					mesh.myVertices[v].myBoneIndices[3] = fbxMesh.Vertices[v].BoneIDs[3];

					// Bone weights
					mesh.myVertices[v].myBoneWeights[0] = fbxMesh.Vertices[v].BoneWeights[0];
					mesh.myVertices[v].myBoneWeights[1] = fbxMesh.Vertices[v].BoneWeights[1];
					mesh.myVertices[v].myBoneWeights[2] = fbxMesh.Vertices[v].BoneWeights[2];
					mesh.myVertices[v].myBoneWeights[3] = fbxMesh.Vertices[v].BoneWeights[3];
				}

				{
					// Assign vertex buffer
					D3D11_BUFFER_DESC vertexBufferDesc = {};
					vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
					vertexBufferDesc.ByteWidth = sizeof(BoneVertex) * (UINT)mesh.myVertices.size();
					vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					vertexBufferDesc.CPUAccessFlags = 0;
					vertexBufferDesc.MiscFlags = 0;

					D3D11_SUBRESOURCE_DATA vertexData = { 0 };
					vertexData.pSysMem = mesh.myVertices.data();
					vertexData.SysMemPitch = 0;
					vertexData.SysMemSlicePitch = 0;

					HRESULT result = myDevice->CreateBuffer(&vertexBufferDesc, &vertexData,
						&mesh.myVertexBuffer);

					if (FAILED(result))
					{
						KE_ERROR("Failed to create vertex buffer.");
						//todo: error handling
					}
				}

				{
					// Assign index buffer
					D3D11_BUFFER_DESC indexBufferDesc = { 0 };
					indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
					indexBufferDesc.ByteWidth = sizeof(unsigned int) * (UINT)mesh.myIndices.size();
					indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
					indexBufferDesc.CPUAccessFlags = 0;
					indexBufferDesc.MiscFlags = 0;

					D3D11_SUBRESOURCE_DATA indexData = { 0 };
					indexData.pSysMem = mesh.myIndices.data();

					HRESULT result = myDevice->
						CreateBuffer(&indexBufferDesc, &indexData, &mesh.myIndexBuffer);

					if (FAILED(result))
					{
						KE_ERROR("Failed to create index buffer.");
						//todo: error handling
					}

					std::string name = tgaModel.Materials[fbxMesh.MaterialIndex].MaterialName;
					FixMaterialName(name);

					aOutSkeletalModel.myMeshList->myMaterialNames.push_back(name);
				}

				// TODO Create bounds

				for (auto& skMesh : aOutSkeletalModel.myMeshList->myMeshes)
				{
					Vector3f min = { FLT_MAX, FLT_MAX, FLT_MAX };
					Vector3f max = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

					for (const auto& vertex : skMesh.myVertices)
					{
						for (int vi = 0; vi < 3; ++vi)
						{
							min[vi] = (std::min)(min[vi], (&vertex.x)[vi]);
							max[vi] = (std::max)(max[vi], (&vertex.x)[vi]);
						}
					}

					skMesh.myBounds.min = min;
					skMesh.myBounds.max = max;

					const Vector3f avrgPos = (min + max) * 0.5f;
					const Vector3f distVec = avrgPos - max;
					const float radius = distVec.Length();

					skMesh.myBounds.centerPoint = avrgPos;
					skMesh.myBounds.myBoundingRadius = radius;
				}
			}
		}
		else
		{
			// TODO This shouldn't crash the engine
			assert(false && "Failed to load model.");
		}
	}

	void ModelLoader::LoadAnimation(SkeletalModelData& aOutSkeletalModel, const std::string& aFilePath)
	{

		if (myAnimationClips.contains(aFilePath))
		{
			aOutSkeletalModel.myAnimationClipMap[myAnimationClips[aFilePath].name] = &myAnimationClips[aFilePath];
			return;
		}

		std::vector<TGA::FBXAnimation> animation;

		if (TGA::FBXImporter::LoadAnimation(aFilePath, aOutSkeletalModel.mySkeleton->myBoneNames, animation))
		{
			KE::Timer timer;
			myAnimationClips[aFilePath] = AnimationClip();
			AnimationClip& newAnimation = myAnimationClips[aFilePath];

			auto& anim = animation.back(); //this is kinda odd. Works for the mixamo godman animations though. 

			newAnimation.name = anim.Name;

			// Trim path from name
			const size_t index = newAnimation.name.find_last_of('/');
			newAnimation.name.erase(newAnimation.name.begin(), newAnimation.name.begin() + index + 1);



			//newAnimation.myFps = animation[0].FramesPerSecond;
			//unsigned int length = 0;
			//double duration = 0;
			//size_t frameCount = 0;
			//std::vector<const TGA::FBXAnimation::Frame*> frames;
			//
			//for (const auto& anim : animation)
			//{
			//    duration += anim.Duration;
			//    length += anim.Length;
			//    frameCount += anim.Frames.size();
			//	for (auto& frame : anim.Frames)
			//	{
			//		frames.push_back(&frame);
			//	}
			//}

			newAnimation.fps = anim.FramesPerSecond;
			newAnimation.length = (float)anim.Length; //this might not be correct
			newAnimation.duration = (float)anim.Duration;
			newAnimation.keyframes.resize(anim.Frames.size());


			for (size_t f = 0; f < newAnimation.keyframes.size(); f++)
			{
				newAnimation.keyframes[f].boneTransforms.resize(anim.Frames[f].LocalTransforms.size());

				for (size_t t = 0; t < anim.Frames[f].LocalTransforms.size(); t++)
				{
					DirectX::XMMATRIX localMatrix = {};
					memcpy(&localMatrix, &anim.Frames[f].LocalTransforms[t], sizeof(float) * 16);

					//// Extract translation, rotation, and scaling components
					//DirectX::XMVECTOR translation, rotation, scale;
					//DirectX::XMMatrixDecompose(&scale, &rotation, &translation, localMatrix);

					//// Scale the translation component
					//DirectX::XMMATRIX translationScaleMatrix = DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f);
					//translation = DirectX::XMVector3Transform(translation, translationScaleMatrix);

					//// Recreate the local matrix with the scaled translation
					//localMatrix = DirectX::XMMatrixAffineTransformation(scale, DirectX::XMVECTOR{0, 0, 0, 1}, rotation, translation);

					newAnimation.keyframes[f].boneTransforms[t] = localMatrix;
				}
			}

			aOutSkeletalModel.myAnimationClipMap[newAnimation.name] = &newAnimation;

			timer.UpdateDeltaTime();

			KE_LOG_CHANNEL("ModelLoader", "Loaded animation [%s] in %.5f seconds", newAnimation.name.c_str(), timer.GetDeltaTime());
		}
	}

	AnimationClip* ModelLoader::GetAnimationClip(const std::string& aFilePath)
	{
		if (myAnimationClips.contains(aFilePath))
		{
			return &myAnimationClips[aFilePath];
		}

		return nullptr;
	}

	void ModelLoader::Preload(const std::vector<std::string>& aFilePaths, PreloadData::Type aType)
	{
		myPreloadData.push_back({ aFilePaths, aType });

		if (!myPreloadThreadRunning)
		{
			myPreloadThread = std::thread(&ModelLoader::PreloadThread, this);
			myPreloadThreadRunning = true;
		}
	}

}
