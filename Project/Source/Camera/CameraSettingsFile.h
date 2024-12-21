#pragma once
#include "Project/Source/Camera/ActionCameraComponent.h"

#define CAMERASETTINGSFILEPATH "CameraSettings"

namespace P8
{
	class CameraSettingsFile
	{
	public:
		static void Save(const P8::CameraSettingsData& someData);
		static const bool Load(P8::CameraSettingsData* someData);
		static void Delete();
	};
}

