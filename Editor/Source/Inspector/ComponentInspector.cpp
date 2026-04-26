#include "stdafx.h"
#include "ComponentInspector.h"

#include <map>
#include <unordered_map>
#include <vector>

#include "ComponentSystem/Components/Collider/SphereColliderComponent.h"

static const char* cppKeywords[] = {
	"const",
	"auto",
	"static",

	//etc etc
};

static const std::unordered_map<std::string, size_t> typeSizes = 
{
	{"pointer", sizeof(int*)},
	{"map", sizeof(std::map<int,int>)},


	{"int", sizeof(int)},
	{"float", sizeof(float)},
	{"bool", sizeof(bool)},
	{"Vector4f", (size_t)(sizeof(float)*4.0f)},
	{"Vector3f", (size_t)(sizeof(float)*3.0f)},
};

#ifndef KITTYENGINE_NO_EDITOR

namespace KE_EDITOR
{
	ComponentInspector::ComponentInspector()
	{}

	ComponentInspector::~ComponentInspector()
	{}

	std::string ComponentInspector::StructureData(const char* aDirtyData, KE::SphereColliderComponent* aColliderComponent)
	{
		ImGui::Text(aDirtyData);

		//split aDirtyData by semicolon such that each element is a string that ends with a semicolon
		std::string dirty = aDirtyData;
		std::vector<std::string> splitData;
		while(true)
		{
			size_t splitPoint = dirty.find_first_of(';');
			if (splitPoint == std::string::npos) { break; }
			splitData.push_back(dirty.substr(0, splitPoint));
			dirty = dirty.substr(splitPoint + 1);
		}

		std::vector<std::string> typeNames;
		std::vector<std::string> variableNames;

		for (int i = 0; i < splitData.size(); i++)
		{
			std::string typeName = "";
			std::string variableName = "";

			std::vector<std::string> stringWords;
			std::string data = splitData[i];

			//try to find the typename, which is the first word in the string that is not a keyword
			while (true)
			{
				size_t splitPoint = data.find_first_of(' ');

				if (data.find_first_of("<") < splitPoint && data.find_first_of(">") > splitPoint)
				{
					const auto offset = data.find_first_of(">");
					splitPoint = data.find_first_of(' ', offset);
				}

				stringWords.push_back(data.substr(0, splitPoint));
				data = data.substr(splitPoint + 1);

				if (splitPoint == std::string::npos) { break; }
			}

			for (int j = 0; j < stringWords.size(); j++)
			{
				for (auto& cppKeyword : cppKeywords)
				{
					if (stringWords[j] != cppKeyword)
					{
						if (typeName == "")
						{
							typeName = stringWords[j];
							break;
						}
						else if (variableName == "")
						{
							variableName = stringWords[j];
							break;
						}
						else
						{
							break;
						}
					}
				}
			}

			typeNames.push_back(typeName);
			variableNames.push_back(variableName);
		}

		size_t byteOffset = 17; //17 because 8 is from vtable pointer in component, 8 is from GameObject reference in component, and 1 from bool in component
		for (int i = 0; i < typeNames.size(); i++)
		{
			size_t size = 0;
			//if typename ends with a *, it is a pointer
			if (typeNames[i].back() == '*')
			{
				size = typeSizes.at("pointer");
			}
			else if (typeNames[i].find("std::map") != std::string::npos)
			{
				size = typeSizes.at("map");
			}
			else
			{
				size = typeSizes.at(typeNames[i]);
			}

			if (_stricmp(typeNames[i].c_str(), "Vector4f") == 0)
			{
				Vector4f* vec = (Vector4f*)(((char*)aColliderComponent) + byteOffset);
				ImGui::DragFloat4(variableNames[i].c_str(), &vec->x);
			}


			byteOffset += size;
		}

		return aDirtyData;
	}

	void ComponentInspector::Inspect(const char* aDirtyData, KE::SphereColliderComponent* aColliderComponent)
	{
		std::string cleaner = StructureData(aDirtyData, aColliderComponent);
	}
}

#endif

