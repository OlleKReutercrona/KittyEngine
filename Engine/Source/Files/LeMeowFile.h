#pragma once
#include <Engine/Source/Graphics/PostProcessAttributes.h>

namespace KE
{
	class LevelSettingsMeowFile
	{
	public:
		KE::PostProcessAttributes myPPData;

		void Save(const std::string& aFilePath);
		bool Load(const std::string& aFilePath);
	};
}