#pragma once
namespace KE
{
	class KittyMesh
	{
	public:
		void Save(const std::string& aPath);
		bool Load(const std::string& aPath);

		std::vector<Vector3f> myVertices;
		std::vector<unsigned int> myIndices;
	};
}

