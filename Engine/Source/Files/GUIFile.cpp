#include "stdafx.h"
#include "GUIFile.h"
#include <External/Include/nlohmann/json.hpp>
#include <fstream>

#include "Graphics/Texture/Texture.h"
#include "UI/GUIScene.h"

//#include "GUIHandler.h"

namespace KE
{
	//void GUIFile::Init(GUIHandler* aGUIHandler)
	//{
	//	myGUIHandler = aGUIHandler;
	//}

	GUIFile::GUIFile(std::unordered_map<std::string, GUIScene>& aSceneMap)
	{
		for (auto& scene : aSceneMap)
		{
			myScenesData.emplace_back();
			myScenesData.back().myName = scene.first;

			for (auto& element : scene.second.GetGUIElements())
			{
				myScenesData.back().myElements.emplace_back();
				// Element
				myScenesData.back().myElements.back().myName = element.myName;
				myScenesData.back().myElements.back().myTexturePath = element.myDisplayTexture->myMetadata.myFilePath;
				myScenesData.back().myElements.back().mySecondaryTexturePath = element.mySecondaryTexture->myMetadata.myFilePath;
				myScenesData.back().myElements.back().myText = element.myText;
				myScenesData.back().myElements.back().myTextColour = element.myTextColour;
				myScenesData.back().myElements.back().myEventName = element.myEvent.myEventName;
				//myScenesData.back().myElements.back().myType = (int)element.myType;
				myScenesData.back().myElements.back().myAlignType = (int)element.myAlignType;
				myScenesData.back().myElements.back().myProgressionDirection = (int)element.myProgressionDirection;
				myScenesData.back().myElements.back().isButton = element.isButton;
				myScenesData.back().myElements.back().hasText = element.hasText;
				myScenesData.back().myElements.back().hideAtStart = element.shouldHideAtStart;
				// Box
				myScenesData.back().myElements.back().myOffsetResolutionFactor = element.myBox.myOffsetResolutionFactor;
				myScenesData.back().myElements.back().mySizeResolutionFactor = element.myBox.mySizeResolutionFactor;
			}
		}
	}

	void GUIFile::Save(const std::string& aFilePath)
	{
		nlohmann::ordered_json scenes;

		// Store header string for scenes "scenes"
		for (auto& sceneData : myScenesData)
		{
			nlohmann::ordered_json scene;
			scene["name"] = sceneData.myName;
			scene["elements"] = nlohmann::json::array();

			for (auto& element : sceneData.myElements)
			{
				// Store header string for elements "elements"
				nlohmann::ordered_json elementData;
				elementData["name"] = element.myName;
				elementData["texturePath"] = element.myTexturePath;
				elementData["secondaryTexturePath"] = element.mySecondaryTexturePath;
				elementData["text"] = element.myText;
				elementData["textColour"]["r"] = element.myTextColour.x;
				elementData["textColour"]["g"] = element.myTextColour.y;
				elementData["textColour"]["b"] = element.myTextColour.z;
				elementData["textColour"]["a"] = element.myTextColour.w;
				elementData["eventName"] = element.myEventName;
				//elementData["type"] = element.myType;
				elementData["alignType"] = element.myAlignType;
				elementData["progressionDirection"] = element.myProgressionDirection;
				elementData["button"] = element.isButton;
				elementData["hasText"] = element.hasText;
				elementData["hideAtStart"] = element.hideAtStart;
				elementData["offsetResolutionFactor"]["x"] = element.myOffsetResolutionFactor.x;
				elementData["offsetResolutionFactor"]["y"] = element.myOffsetResolutionFactor.y;
				elementData["sizeResolutionFactor"]["x"] = element.mySizeResolutionFactor.x;
				elementData["sizeResolutionFactor"]["y"] = element.mySizeResolutionFactor.y;

				//bool isBad = false;
				//std::string badName = "";

				//for (int p = 1; p < 5; ++p)
				//{
				//	if (isBad)
				//	{
				//		break;
				//	}
				//	badName = "P" + std::to_string(p) + "Score";
				//	for (int i = 0; i < elementData.size(); ++i)
				//	{
				//		std::string name = badName + std::to_string(i);
				//		if (element.myName == name)
				//		{
				//			isBad = true;
				//			badName = name;
				//			break;
				//		}
				//	}
				//}
				//if (!isBad)
				//{
				//	scene["elements"].push_back(elementData);
				//}
				//else
				//{
				//	std::cout << "\n" + badName + " is a bad name. I won't save this one!";
				//}
				scene["elements"].push_back(elementData);
			}

			scenes.push_back(scene);
		}

		// Save the file
		if (
			std::filesystem::exists(aFilePath) &&
			std::filesystem::is_regular_file(aFilePath) &&
			std::filesystem::status(aFilePath).permissions() == std::filesystem::perms::_File_attribute_readonly
			)
		{
			std::filesystem::permissions(aFilePath, std::filesystem::perms::owner_write, std::filesystem::perm_options::replace);
		}

		std::string jsonStr = scenes.dump(4);
		std::ofstream file(aFilePath);
		file << jsonStr;
		file.close();
	}

	void GUIFile::Load(const std::string& aFilePath)
	{
		if (!std::filesystem::exists(aFilePath))
		{
			return;
		}

		if (!myScenesData.empty())
		{
			myScenesData.clear();
		}

		std::ifstream ifs(aFilePath);

		nlohmann::json guiFile;
		if (ifs.good())
		{
			guiFile = nlohmann::json::parse(ifs);
		}

		ifs.close();

		for (size_t i = 0; i < guiFile.size(); ++i)
		{
			myScenesData.emplace_back();
			myScenesData.back().myName = guiFile[i]["name"].get<std::string>();

			for (auto& element : guiFile[i]["elements"])
			{
				myScenesData.back().myElements.emplace_back();
				// Element
				myScenesData.back().myElements.back().myName = element["name"].get<std::string>();
				myScenesData.back().myElements.back().myTexturePath = element["texturePath"].get<std::string>();
				myScenesData.back().myElements.back().mySecondaryTexturePath = element["secondaryTexturePath"].get<std::string>();
				myScenesData.back().myElements.back().myText = element["text"].get<std::string>();
				myScenesData.back().myElements.back().myTextColour.x = element["textColour"]["r"].get<float>();
				myScenesData.back().myElements.back().myTextColour.y = element["textColour"]["g"].get<float>();
				myScenesData.back().myElements.back().myTextColour.z = element["textColour"]["b"].get<float>();
				myScenesData.back().myElements.back().myTextColour.w = element["textColour"]["a"].get<float>();
				myScenesData.back().myElements.back().myEventName = element["eventName"].get<std::string>();
				//myScenesData.back().myElements.back().myType = element["type"].get<int>();
				myScenesData.back().myElements.back().myAlignType = element["alignType"].get<int>();
				myScenesData.back().myElements.back().myProgressionDirection = element["progressionDirection"].get<int>();
				myScenesData.back().myElements.back().isButton = element["button"].get<bool>();
				myScenesData.back().myElements.back().hasText = element["hasText"].get<bool>();
				myScenesData.back().myElements.back().hideAtStart = element["hideAtStart"].get<bool>();
				// Box
				myScenesData.back().myElements.back().myOffsetResolutionFactor.x = element["offsetResolutionFactor"]["x"].get<float>();
				myScenesData.back().myElements.back().myOffsetResolutionFactor.y = element["offsetResolutionFactor"]["y"].get<float>();
				myScenesData.back().myElements.back().mySizeResolutionFactor.x = element["sizeResolutionFactor"]["x"].get<float>();
				myScenesData.back().myElements.back().mySizeResolutionFactor.y = element["sizeResolutionFactor"]["y"].get<float>();
			}
		}
	}
}
