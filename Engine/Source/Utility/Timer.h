#pragma once
#include <mutex>
#include "Engine/Source/Windows/KittyEngineWin.h"

namespace KE
{
	class Timer
	{
	public:
		Timer();

		void Reset();
		float UpdateDeltaTime();
		float GetDeltaTime() const;
		float GetTotalTime();
		float GetFPS();
		size_t GetElapsedCycles() const;

	private:
		LARGE_INTEGER frequency;
		LARGE_INTEGER startTime;
		LARGE_INTEGER lastFrameTime;
		float deltaTime;
		float totalTime = 0.0f;
		float fps = 0.0f;
		std::mutex mutex;
	};
}
