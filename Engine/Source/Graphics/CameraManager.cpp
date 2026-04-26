#include "stdafx.h"
#include "CameraManager.h"

#include "Engine/Source/Utility/Logging.h"

namespace KE
{
	CameraManager::CameraManager()
	{
		for (int i = 0; i < KE_CAMERA_MAX; i++)
		{
			cameras[i].myIndex = i;
			cameras[i].SetPerspective(1, 1, 1.0f, 0.1f, 10.0f);
		}
	}

	CameraManager::~CameraManager() { }

	Camera* CameraManager::GetCamera(const int aIndex)
	{
		if (cameras.size() > aIndex)
		{
			return &cameras[aIndex];
		}

		KE_ERROR("Attempt to get Camera %i failed.", aIndex);
		return nullptr;
	}
}
