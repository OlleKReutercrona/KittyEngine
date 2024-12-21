#pragma once
#include "Engine\Source\Graphics\ModelData.h"
#include "Engine\Source\Math\KittyMath.h"
#include "Engine\Source\Math\Transform.h"

namespace KE
{
	struct PointLightData
	{
		Vector4f position;  // x,y,z w = Strength
		Vector3f colour;	// Colour of the light
		float radius;		// Radius of the light
	};

	class DeferredPointLight
	{
	public:
		DeferredPointLight();
		~DeferredPointLight() {};

		inline const ModelData& GetModelData() const { return myModelData; }
		inline const int GetID() const { return myID; }

		Transform& GetTransform() { return myTransform; }
	private:
		Transform myTransform;

		ModelData myModelData;
		MeshList myMesh;
		PointLightData myPointLightData;
		int myID;
	};

}