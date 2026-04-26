#pragma once
#include "Engine/Source/Math/Transform.h"

namespace KE
{
	class GBuffer;
}

namespace KE
{
	struct MeshList;
	class Graphics;
	struct Material;

	enum class DecalActiveState: unsigned char
	{
		Active,
		Inactive,
		Destroyed
	};

	struct Decal
	{
		Material* myMaterial;
		Transform myTransform;
		DecalActiveState myActiveState;
		Vector4f myTextureIntensities;
	};

	class DecalManager
	{
	private:
		std::vector<Decal> myDecals;
		std::vector<unsigned int> myFreeDecalIndices;
		MeshList* myMeshList;

	public:
		DecalManager();
		~DecalManager();

		void Init(Graphics* aGraphics);

		int CreateDecal(Material* aMaterial, const Transform& aTransform);
		void DestroyDecal(int aIndex);
		Decal* GetDecal(int aIndex);
		const std::vector<Decal>& GetDecals() { return myDecals; }
		void PrepareDecalRendering(Graphics* aGraphics, GBuffer* aWorkingGBuffer, GBuffer* aCopyGBuffer);

	};

}
