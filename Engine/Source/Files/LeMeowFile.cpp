#include "stdafx.h"
#include "LeMeowFile.h"

#include <External/Include/nlohmann/json.hpp>
#include <fstream>

#define LEMEOW_PATH "Data/LevelSettings/"
#define LEMEOW_FILEENDING ".LeMeow"

namespace KE
{
	void LevelSettingsMeowFile::Save(const std::string& aFilePath)
	{
		// If Folder doesnt exist, try to create one 
		if (CreateDirectoryA(LEMEOW_PATH, NULL) || ERROR_ALREADY_EXISTS == GetLastError())
		{
			std::fstream file;
			file.open(LEMEOW_PATH + aFilePath, std::ios_base::trunc | std::ios_base::binary | std::ios_base::out);

			file.clear();

			file.write(reinterpret_cast<char*>(&myPPData), sizeof(KE::PostProcessAttributes));
			file.close();
		}
		else
		{
			KE_ERROR("Failed to create a level settings folder!");
		}
	}

	bool LevelSettingsMeowFile::Load(const std::string& aFilePath)
	{
		std::ifstream file;

		file.open(LEMEOW_PATH + aFilePath, std::ios::in | std::ios::binary);
		if (file.good())
		{
			char buffer[sizeof(KE::PostProcessAttributes)];

			file.read(buffer, sizeof(KE::PostProcessAttributes));

			myPPData = *(KE::PostProcessAttributes*)buffer;

			file.close();
			return true;
		}

		return false;
	}
}