#pragma once
#include <map>
#include <wrl/client.h>

//#include <vector>
//#include <string>

#include "GraphicsConstants.h"
//#include "Engine\Source\Math\Vector.h"
#include "Engine/Source/Math/Matrix3x3.h"
#include "Engine/Source/Graphics/Animation/AnimationData.h"

using Microsoft::WRL::ComPtr;
struct ID3D11Buffer;

namespace DirectX
{
	struct XMMATRIX;
}

struct ModelBounds
{
	Vector3f min;
	Vector3f max;
	Vector3f centerPoint;
	float myBoundingRadius = 0.0f;
};

namespace KE
{
	class CBuffer;
	struct Material;
	class VertexShader;
	class PixelShader;

	struct Vertex
	{
		float x, y, z, w;
		float u, v;

		float nx, ny, nz; //, nw;
		float tx, ty, tz; //, tw;
		float bx, by, bz; //, bw;

		Vertex& operator=(const Vector3f& aVector)
		{
			x = aVector.x;
			y = aVector.y;
			z = aVector.z;

			return *this;
		}

		void operator*=(const Matrix3x3f& aMatrix)
		{
			// This is Assars f-ed up way of doing this

			(*(Vector3f*)&x)  = (*(Vector3f*)&x)  * aMatrix;
			(*(Vector3f*)&nx) = (*(Vector3f*)&nx) * aMatrix;
			(*(Vector3f*)&tx) = (*(Vector3f*)&tx) * aMatrix;
			(*(Vector3f*)&bx) = (*(Vector3f*)&bx) * aMatrix;

			// More readable version down here

			//Vector3f position(x, y, z);
			
			//Vector3f normal(nx, ny, nz);
			//Vector3f binormal(bx, by, bz);
			//Vector3f tangent(tx, ty, tz);
			//
			//position = position * aMatrix;
			//normal = normal * aMatrix;
			//binormal = binormal * aMatrix;
			//tangent = tangent * aMatrix;
			//
			//x = position.x;
			//y = position.y;
			//z = position.z;
			//
			//nx = normal.x;
			//ny = normal.y;
			//nz = normal.z;
			//
			//bx = binormal.x;
			//by = binormal.y;
			//bz = binormal.z;
			//
			//tx = tangent.x;
			//ty = tangent.y;
			//tz = tangent.z;
		}
	};


	struct BoneVertex
	{
		float x, y, z, w;
		float u, v;

		float nx, ny, nz; //, nw;
		float tx, ty, tz; //, tw;
		float bx, by, bz; //, bw;

		int myBoneIndices[4];
		float myBoneWeights[4];
	};

	struct Bone
	{
		std::string myName;
		DirectX::XMMATRIX myBindPose = {};
		int myParentIndex = -1;
	};

	struct Skeleton
	{
		int myRootBoneIndex = -1; // Index of the root bone in the skeleton
		std::vector<Bone> myBones = {};
		std::vector<std::string> myBoneNames;
	};

	struct Mesh
	{
		std::vector<Vertex> myVertices;
		std::vector<unsigned int> myIndices;

		inline unsigned int GetIndexCount() const { return (unsigned int)myIndices.size(); }

		ComPtr<ID3D11Buffer> myVertexBuffer;
		ComPtr<ID3D11Buffer> myIndexBuffer;

		ModelBounds myBounds;
	};

	struct SkeletalMesh
	{
		std::vector<BoneVertex> myVertices;
		std::vector<unsigned int> myIndices;

		inline unsigned int GetIndexCount() const { return (unsigned int)myIndices.size(); }

		ComPtr<ID3D11Buffer> myVertexBuffer;
		ComPtr<ID3D11Buffer> myIndexBuffer;

		ModelBounds myBounds;
	};

	struct MeshList
	{
		std::vector<Mesh> myMeshes;
		std::vector<std::string> myMaterialNames;
		std::string myFilePath;
	};

	struct SkeletalMeshList
	{
		std::vector<SkeletalMesh> myMeshes;
		std::vector<std::string> myMaterialNames;
		std::string myFilePath;
	};

	struct RenderResources
	{
		VertexShader* myVertexShader;
		PixelShader* myPixelShader;
		Material* myMaterial;
		CBuffer* myCBuffer = nullptr;
		int myCBufferVSSlot = -1;
		int myCBufferPSSlot = -1;

	};

	typedef std::vector<RenderResources> RenderResourceList;

	struct ModelData // Final data blob that graphics engine uses to render a mesh/modelcomponent :)
	{
		MeshList* myMeshList = nullptr;
		std::vector<RenderResources> myRenderResources = {};

		DirectX::XMMATRIX* myTransform = nullptr;

		bool myActiveStatus = true;
		bool myRenderOutline = false; //note: move!! to attributes!!
		bool myIsInstanced = true;

		struct
		{
			float myUVXScale = 1.0f;
			float myUVYScale = 1.0f;
		} myAttributes;

		//this is kinda evil, but everything inside render resources is a pointer, so we can just memset it to 0.. i think
		~ModelData() { memset(&myRenderResources, 0, sizeof(myRenderResources));}

		eRenderLayers myRenderLayer = eRenderLayers::Main;
	};
	typedef std::vector<ModelData> ModelDataList;

	struct SkeletalModelData
	{
		SkeletalMeshList* myMeshList = nullptr;
		Skeleton* mySkeleton = nullptr;

		RenderResourceList myRenderResources = {};

		DirectX::XMMATRIX* myTransform = nullptr;

		bool myActiveStatus = true;
		bool myRenderOutline = false;

		std::map<std::string, AnimationClip*> myAnimationClipMap = {};

		std::vector<DirectX::XMMATRIX> myCombinedTransforms = {};
		std::vector<DirectX::XMMATRIX> myFinalTransforms = {};
		std::vector<DirectX::XMMATRIX> myWorldSpaceJoints = {};

		eRenderLayers myRenderLayer = eRenderLayers::Main;
	};
	typedef std::vector<SkeletalModelData> SkeletalModelDataList;
}
