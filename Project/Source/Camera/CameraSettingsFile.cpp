#include "stdafx.h"
#include "CameraSettingsFile.h"
#include <fstream>

#define PREPATH "Data/ComponentSettings/"
#define FILEENDING ".CamSet"

void P8::CameraSettingsFile::Save(const P8::CameraSettingsData& someData)
{
	P8::CameraSettingsData data = someData;


	// If Folder doesnt exist, try to create one 
	if (CreateDirectoryA(PREPATH, NULL) || ERROR_ALREADY_EXISTS == GetLastError())
	{
		std::fstream file;
		
		std::string path(PREPATH);
		path += CAMERASETTINGSFILEPATH FILEENDING;

		//is file read only? if so, unlock it
		DWORD fileAttributes = GetFileAttributesA(path.c_str());
		if (fileAttributes & FILE_ATTRIBUTE_READONLY)
		{
			fileAttributes &= ~FILE_ATTRIBUTE_READONLY;
			SetFileAttributesA(path.c_str(), fileAttributes);
		}

		file.open(path, std::ios_base::trunc | std::ios_base::binary | std::ios_base::out);

		file.clear();

		file.write(reinterpret_cast<char*>(&data), sizeof(P8::CameraSettingsData));
		file.close();
	}
	else
	{
		KE_ERROR("Failed to create a level settings folder!");
	}
}

const bool P8::CameraSettingsFile::Load(P8::CameraSettingsData* someData)
{
	std::ifstream file;

	std::string path(PREPATH);
	path += CAMERASETTINGSFILEPATH FILEENDING;

	file.open(path, std::ios::in | std::ios::binary);
	if (file.good())
	{
		const unsigned int size = sizeof(P8::CameraSettingsData);

		char buffer[size];

		file.read(buffer, size);

		*someData = *(P8::CameraSettingsData*)buffer;

		file.close();

		return true;
	}
	else
	{
		std::string errormsg("FAILED TO LOAD CAMERA SETTINGS FROM FILE. PATH = ");
		errormsg += path.c_str();
		KE_ERROR(errormsg.c_str());
	}

	return false;
}

void P8::CameraSettingsFile::Delete()
{
	int result = std::remove(PREPATH CAMERASETTINGSFILEPATH FILEENDING);

	if (result == 0)
	{
		KE_LOG("CameraSettings.CamSet has been removed");
	}
	else
	{
		KE_LOG("Failed to remove CameraSettings.CamSet");
	}
}

#undef FILEENDING
#undef PREPATH
