#include "stdafx.h"
#include "KittyMesh.h"

#include <filesystem>
#include <fstream>


#define MEOWMESH_PATH "Data/Navmesh/"

void KE::KittyMesh::Save(const std::string& aPath)
{
	std::fstream file;
	file.open(MEOWMESH_PATH + aPath, std::ios_base::trunc | std::ios_base::binary | std::ios_base::out);

	file.clear();

	size_t vsize = myVertices.size();
	file.write(reinterpret_cast<char*>(&vsize), sizeof(vsize));

	size_t isize = myIndices.size();
	file.write(reinterpret_cast<char*>(&isize), sizeof(isize));

	vsize *= sizeof(Vector3f);
	file.write(reinterpret_cast<char*>(&myVertices[0]), vsize);

	isize *= sizeof(unsigned int);
	file.write(reinterpret_cast<char*>(&myIndices[0]), isize);

	file.close();
}

bool KE::KittyMesh::Load(const std::string& aPath)
{
	std::ifstream file;


	file.open(MEOWMESH_PATH + aPath, std::ios::in | std::ios::binary);
	if (file.good())
	{

		// Determine size of file
		const auto size = std::filesystem::file_size(MEOWMESH_PATH + aPath);

		// Allocate a buffer for the file to be read
		char* buffer = new char[size];

		// read file
		file.read(buffer, size);

		// Create a pointer to the read data
		char* pointer = buffer;


		size_t sizeofV = *(size_t*)pointer;
		pointer += sizeof(size_t);

		size_t sizeofI = *(size_t*)pointer;
		pointer += sizeof(size_t);

		// Vertices
		{
			myVertices.resize(sizeofV);

			char* vertStart = (char*)myVertices.data();
			memcpy(vertStart, pointer, sizeof(Vector3f) * sizeofV);

			pointer += sizeof(Vector3f) * sizeofV;
		}

		// Indices
		{
			size_t indsize = sizeof(unsigned int);

			myIndices.resize(sizeofI);

			char* indicesStart = (char*)myIndices.data();
			memcpy(indicesStart, pointer, indsize * sizeofI);

			pointer += indsize * sizeofI;
		}


		delete[] buffer;

		file.close();
		
		return true;
	}

	return false;
}
