#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include <External/Include/assimp/scene.h>

#include "ModelData.h"

struct ID3D11Device;

namespace KE
{
	struct PreloadData
	{

		std::vector<std::string> filePaths;

		enum class Type
		{
			Mesh,
			AnimatedMesh
		} type;

		std::mutex mutex;
		bool loaded = false;

		PreloadData(const std::vector<std::string>& aFilePaths, Type aType)
			: filePaths(aFilePaths)
			, type(aType)
		{}

		PreloadData(const PreloadData& aOther)
			: filePaths(aOther.filePaths)
			, type(aOther.type)
		{}
	};

	class ModelLoader
	{
	private:
		std::unordered_map<std::string, MeshList> myMeshes;
		std::unordered_map<std::string, SkeletalMeshList> mySkeletalMeshes;
		std::unordered_map<std::string, Skeleton> mySkeletons;
		std::unordered_map<std::string, AnimationClip> myAnimationClips;
		ID3D11Device* myDevice;

		void Process(
			const aiNode* aNode,
			const aiScene* aScene,
			MeshList& aMeshListToFill
		);

		// Overload to handle skeletal meshes.
		void Process(
			const aiNode* aNode,
			const aiScene* aScene,
			SkeletalMeshList& aSkeletalMeshListToFill
		);

		
		void AssignBoneIndices(const aiScene* aScene, SkeletalMeshList& aSkeletalMeshListToFill);

		std::vector<PreloadData> myPreloadData;
		bool myPreloadThreadRunning = false;
		std::thread myPreloadThread;
		void PreloadThread();

	public:
		std::unordered_map<std::string, MeshList>& GetMeshes() { return myMeshes; }
		std::unordered_map<std::string, SkeletalMeshList>& GetSkeletalMeshes() { return mySkeletalMeshes; }

		~ModelLoader();
		void Clear();

		void Init(ID3D11Device* aDevice);

		MeshList& Load(const std::string& aFilePath);

		bool IsModelSkeletal(const std::string& aFilePath);

		void LoadSkeletalModel(SkeletalModelData& aOutSkeletalModel, const std::string& aFilePath);
		void LoadAnimation(SkeletalModelData& aOutSkeletalModel, const std::string& aFilePath);
		AnimationClip* GetAnimationClip(const std::string& aFilePath);

		void Preload(const std::vector<std::string>& aFilePaths, PreloadData::Type aType);
	};
}
