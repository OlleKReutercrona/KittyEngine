#include "stdafx.h"
#include "PawFile.h"

#include <filesystem>
#include <fstream>

#define PAW_PATH "Data/Levels/"

void KE::LevelTransformFile::Save(const std::string& aFilePath)
{
	std::fstream file;
	file.open(PAW_PATH + aFilePath, std::ios_base::trunc | std::ios_base::binary | std::ios_base::out);

	file.clear();

	// Number of Transforms
	size_t vsize = myData.transforms.size();
	file.write(reinterpret_cast<char*>(&vsize), sizeof(vsize));

	// Transform IDs
	size_t idSize = myData.transformsIDs.size();
	idSize *= sizeof(int);
	file.write(reinterpret_cast<char*>(&myData.transformsIDs[0]), idSize);

	// Transforms
	vsize *= sizeof(Transform);
	file.write(reinterpret_cast<char*>(&myData.transforms[0]), vsize);

	file.close();
}

bool KE::LevelTransformFile::Load(const std::string& aFilePath)
{
	std::ifstream file;

	file.open(PAW_PATH + aFilePath, std::ios::in | std::ios::binary);
	if (file.good())
	{
		// Determine size of file
		const auto size = std::filesystem::file_size(PAW_PATH + aFilePath);

		// Allocate a buffer for the file to be read
		char* buffer = new char[size];

		// read file
		file.read(buffer, size);

		// Create a pointer to the read data
		char* pointer = buffer;

		// Number of Transforms / IDs
		size_t numberOfTransforms = *(size_t*)pointer;
		pointer += sizeof(size_t);

		// Allocating memory for the vectors
		myData.transforms.resize(numberOfTransforms);
		myData.transformsIDs.resize(numberOfTransforms);

		// Transform ID memcpy
		char* idStart = (char*)myData.transformsIDs.data();
		memcpy(idStart, pointer, sizeof(int) * numberOfTransforms);
		pointer += sizeof(int) * numberOfTransforms;

		// Transform matrix memcpy
		char* vertStart = (char*)myData.transforms.data();
		memcpy(vertStart, pointer, sizeof(Transform) * numberOfTransforms);
		pointer += sizeof(Transform) * numberOfTransforms;

		delete[] buffer;

		file.close();

		return true;
	}

	return false;
}
