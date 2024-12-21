#pragma once
#include <vector>
#include "Engine/Source/Math/Transform.h"

namespace KE
{
	struct LevelTransformData
	{
		std::vector<int> transformsIDs;
		std::vector<Transform> transforms;
	};


	/// PAW FILE PACKING ///
	/*
		__HEADER__	
		[1 uint]										- number of transforms

		__DATA__
		[int * number of transforms]					- Transform IDs

		[16 Byte Matrix4x4f * number of transforms]		- Transform Data
	*/



	class LevelTransformFile
	{
	public:
		LevelTransformFile() = default;
		~LevelTransformFile() = default;

		void Save(const std::string& aFilePath);
		bool Load(const std::string& aFilePath);

		LevelTransformData myData;
	};
}
