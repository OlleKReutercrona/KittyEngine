#include "stdafx.h"
#ifndef KITTYENGINE_NO_EDITOR
#include "EditorData.h"

#include "MeowFile.h"
#include <External/Include/nlohmann/json.hpp>
#include <fstream>

#include "Editor.h"

namespace KE_EDITOR
{
	void MeowFile::Save(const std::string& aFilePath)
	{
		nlohmann::json json;

		json["/window/x"_json_pointer] = windowSettings.windowX;
		json["/window/y"_json_pointer] = windowSettings.windowY;
		json["/window/width"_json_pointer] = windowSettings.windowWidth;
		json["/window/height"_json_pointer] = windowSettings.windowHeight;
		json["/window/isMaximized"_json_pointer] = windowSettings.isWindowMaximized;
		json["/window/isFullscreen"_json_pointer] = windowSettings.isWindowFullscreen;

		json["/console/x"_json_pointer] = windowSettings.consoleX;
		json["/console/y"_json_pointer] = windowSettings.consoleY;
		json["/console/width"_json_pointer] = windowSettings.consoleWidth;
		json["/console/height"_json_pointer] = windowSettings.consoleHeight;
		json["/console/isMaximized"_json_pointer] = windowSettings.isConsoleMaximized;

		json["/debugRendering/navmeshLevel"_json_pointer] = (char)debugRenderData.renderNavmesh;
		json["/debugRendering/skeletonLevel"_json_pointer] = (char)debugRenderData.renderSkeleton;

		json["editorWindows"] = nlohmann::json::array();
		for (auto& window : KE_GLOBAL::editor->GetWindows())
		{
			nlohmann::json windowJson;
			nlohmann::json windowData;
			window->Serialize(&windowData);
			windowJson["windowName"] = window->GetName();
			windowJson["customData"] = windowData;

			json["editorWindows"].push_back(windowJson);
		}

		if (
			std::filesystem::exists(aFilePath) &&
			std::filesystem::is_regular_file(aFilePath) &&
			std::filesystem::status(aFilePath).permissions() == std::filesystem::perms::_File_attribute_readonly
			)
		{
			std::filesystem::permissions(aFilePath, std::filesystem::perms::owner_write, std::filesystem::perm_options::replace);
		}

		//prettify json
		std::string jsonStr = json.dump(4);
		std::ofstream file(aFilePath);
		file << jsonStr;
		file.close();
	}

	void MeowFile::Load(const std::string& aFilePath)
	{
		nlohmann::json json;

		std::ifstream file(aFilePath);
		file >> json;
		file.close();

		windowSettings.windowX =			json["/window/x"_json_pointer];
		windowSettings.windowY =			json["/window/y"_json_pointer];
		windowSettings.windowWidth =		json["/window/width"_json_pointer];
		windowSettings.windowHeight =		json["/window/height"_json_pointer];
		windowSettings.isWindowMaximized =	json["/window/isMaximized"_json_pointer];
		windowSettings.isWindowFullscreen = json["/window/isFullscreen"_json_pointer];

		windowSettings.consoleX =			json["/console/x"_json_pointer];
		windowSettings.consoleY =			json["/console/y"_json_pointer];
		windowSettings.consoleWidth =		json["/console/width"_json_pointer];
		windowSettings.consoleHeight =		json["/console/height"_json_pointer];
		windowSettings.isConsoleMaximized = json["/console/isMaximized"_json_pointer];

		//make sure we don't crash if we load a file that doesn't have these values
		if (json.contains("/debugRendering/navmeshLevel"_json_pointer) == false) { return;}

		debugRenderData.renderNavmesh =		(DebugRenderLevel)json["/debugRendering/navmeshLevel"_json_pointer];
		debugRenderData.renderSkeleton =	(DebugRenderLevel)json["/debugRendering/skeletonLevel"_json_pointer];


		for (const auto& windowData : json["editorWindows"])
		{
			std::string windowName = windowData["windowName"];
			nlohmann::json customData = windowData["customData"];

			auto* window = KE_GLOBAL::editor->myWindowRegistry[windowName].myCreationFunc({});
			window->Deserialize(&customData);
		}
	}
}
#endif