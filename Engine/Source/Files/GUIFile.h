#pragma once
//#include "GUIScene.h"

namespace KE
{
	class GUIScene;

	struct GUIElementData
	{
		// Element
		std::string myName;
		std::string myTexturePath;
		std::string mySecondaryTexturePath;
		std::string myText;
		std::string myEventName;
		Vector4f myTextColour;
		//int myType;
		int myAlignType;
		int myProgressionDirection;
		bool isButton;
		bool hasText;
		bool hideAtStart;
		// Box
		Vector2f myOffsetResolutionFactor;
		Vector2f mySizeResolutionFactor;
	};

	struct GUISceneData
	{
		std::string myName;
		std::vector<GUIElementData> myElements;
	};


	class GUIFile
	{
	public:
		GUIFile() = default;
		GUIFile(std::unordered_map<std::string, GUIScene>& aSceneMap);
		void Save(const std::string& aFilePath);
		void Load(const std::string& aFilePath);

		std::vector<GUISceneData> myScenesData;

	private:
	};
}
