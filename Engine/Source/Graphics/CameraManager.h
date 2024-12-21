#pragma once
#include "Camera.h"

#define KE_CAMERA_MAX 32
#define KE_MAIN_CAMERA_INDEX 0
#define KE_DEBUG_CAMERA_INDEX KE_CAMERA_MAX - 1

namespace KE
{
	class CameraManager
	{
	private:
		std::array<Camera, KE_CAMERA_MAX> cameras;
		unsigned int highlightedCamera = KE_MAIN_CAMERA_INDEX;


	public:
		CameraManager();
		~CameraManager();

		Camera* GetCamera(const int aIndex);
		inline Camera* GetMainCamera() { return &cameras[KE_MAIN_CAMERA_INDEX]; }
		inline Camera* GetHighlightedCamera() { return &cameras[highlightedCamera]; }

		inline void SetHighlightedCamera(const int aIndex) { highlightedCamera = aIndex; }
	};
}