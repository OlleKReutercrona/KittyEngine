#pragma once


#include <string>

struct DebugRenderData;

namespace KE_EDITOR
{

	class MeowFile
	{
	public:
		//EXPOSE(
		struct
		{
			//window settings
			float windowX = -1.0f;
			float windowY = -1.0f;
			float windowWidth = -1.0f;
			float windowHeight = -1.0f;

			float consoleX = -1.0f;
			float consoleY = -1.0f;
			float consoleWidth = -1.0f;
			float consoleHeight = -1.0f;

			bool isWindowMaximized = false;
			bool isConsoleMaximized = false;

			bool isWindowFullscreen = false;
		} windowSettings;
		
		DebugRenderData debugRenderData;

		void Save(const std::string& aFilePath);
		void Load(const std::string& aFilePath);
	};
}