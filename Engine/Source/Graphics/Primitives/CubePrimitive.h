#pragma once
#include <array>
#include "Engine\Source\Graphics\GraphicsConstants.h"
#include "Engine\Source\Math\Vector4.h"

namespace KE
{
	class CubePrimitive
	{
	public:
		CubePrimitive();
		~CubePrimitive();

		inline static const std::array<VertexPoint, 8>& GetVertices() { return myVertices; };
		inline static const std::array<int, 24>& GetIndices() { return myIndices; };
	private:
		inline static std::array<VertexPoint, 8> myVertices;
		inline static std::array<int, 24> myIndices;
		inline static bool myIsInitialized = false;
	};
}