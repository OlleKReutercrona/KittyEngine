#pragma once
#include <vector>
#include <unordered_map>

#include "Engine\Source\Graphics\GraphicsConstants.h"
#include "Engine\Source\Math\Vector4.h"

namespace KE
{
	enum class SphereLOD
	{
		lowPoly,
		highPoly,
		axisSphere,
	};

	struct SphereData
	{
		SphereData() = default;
		std::vector<VertexPoint> myVertices = {};
		std::vector<unsigned int> myIndices = {};
	};

	class SpherePrimitive
	{
	public:
		SpherePrimitive();
		~SpherePrimitive() {};

		inline static const std::vector<VertexPoint>& GetVertices(KE::SphereLOD anAmount = KE::SphereLOD::lowPoly) { return mySphereData[anAmount].myVertices; };
		inline static const std::vector<unsigned int>& GetIndices(KE::SphereLOD anAmount = KE::SphereLOD::lowPoly) { return mySphereData[anAmount].myIndices; };

	private:
		void ContructSphere(KE::SphereLOD aLOD, const int aLat, const int aLong);
		void ContructSphere2(KE::SphereLOD aLOD, const int someStacks, const int someSlices);
		void ContructAxisSphere(const int aPolyCount);

		inline static int myLowPolyLatDiv = 7;
		inline static int myLowPolyLongDiv = 7;

		inline static int myHighPolyLatDiv =	35;
		inline static int myHighPolyLongDiv =	35;

		inline static bool myIsInitialized = false;

		inline static std::unordered_map<KE::SphereLOD, KE::SphereData> mySphereData;
	};
}
