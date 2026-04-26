#pragma once
#include <vector>
#include "Engine\Source\Graphics\GraphicsConstants.h"
#include "Engine\Source\Math\Vector4.h"

namespace KE
{
	struct CapsuleMesh
	{
		std::vector<VertexPoint> myVertices = {};
		std::vector<unsigned int> myIndices = {};

		/*
			x = Bottom
			y = Top
			Index 0 = Left X
			Index 1 = Right X
			Index 2 = Back Z
			Index 3 = Front Z
		*/
		Vector2i lengthIndices[4];
		int halfWayIndex = -1;
	};

	class CapsulePrimitive
	{
	public:
		CapsulePrimitive();
		~CapsulePrimitive() {};

		inline static const CapsuleMesh& GetMesh() { return myMesh; }
	private:

		inline static int myHighPolyLatDiv = 35;
		inline static int myHighPolyLongDiv = 35;

		inline static CapsuleMesh myMesh;

		inline static bool hasInit = false;
	};
}

