#include "stdafx.h"
#include "Engine/Source/Reflection/Reflection.h"
#include "EditorInspection.h"

#include <regex>

#include "imgui/imgui.h"
#include "Math/Matrix3x3.h"

#ifndef KITTYENGINE_NO_EDITOR

KE_EDITOR::EditorInspectionSystem::EditorInspectionSystem()
{
	AddType<int>([](const char* variableName, void* variable) -> bool
	{
		return ImGui::DragInt(variableName, static_cast<int*>(variable));
	});
	AddType<Vector2i>([](const char* variableName, void* variable) -> bool
	{
		return ImGui::DragInt2(variableName, static_cast<int*>(variable));
	});
	AddType<Vector3i>([](const char* variableName, void* variable) -> bool
	{
		return ImGui::DragInt3(variableName, static_cast<int*>(variable));
	});
	AddType<Vector4i>([](const char* variableName, void* variable) -> bool
	{
		return ImGui::DragInt4(variableName, static_cast<int*>(variable));
	});


	AddType<float>([](const char* variableName, void* variable) -> bool
	{
		return ImGui::DragFloat(variableName, static_cast<float*>(variable));
	});
	AddType<Vector2f>([](const char* variableName, void* variable) -> bool
	{
		return ImGui::DragFloat2(variableName, static_cast<float*>(variable));
	});
	AddType<Vector3f>([](const char* variableName, void* variable) -> bool
	{
		return ImGui::DragFloat3(variableName, static_cast<float*>(variable));
	});
	AddType<Vector4f>([](const char* variableName, void* variable) -> bool
	{
		return ImGui::DragFloat4(variableName, static_cast<float*>(variable));
	});

	AddType<bool>([](const char* variableName, void* variable) -> bool
	{
		return ImGui::Checkbox(variableName, static_cast<bool*>(variable));
	});

	AddType<unsigned int>([](const char* variableName, void* variable) -> bool
	{
		return ImGui::DragScalar(variableName, ImGuiDataType_U32, static_cast<unsigned int*>(variable));
	});
	AddType<CommonUtilities::Vector2<unsigned int>>([](const char* variableName, void* variable) -> bool
	{
		return ImGui::DragScalarN(variableName, ImGuiDataType_U32, static_cast<unsigned int*>(variable), 2);
	});
	AddType<CommonUtilities::Vector3<unsigned int>>([](const char* variableName, void* variable) -> bool
	{
		return ImGui::DragScalarN(variableName, ImGuiDataType_U32, static_cast<unsigned int*>(variable), 2);
	});
	AddType<CommonUtilities::Vector4<unsigned int>>([](const char* variableName, void* variable) -> bool
	{
		return ImGui::DragScalarN(variableName, ImGuiDataType_U32, static_cast<unsigned int*>(variable), 2);
	});

	AddType<Matrix3x3f>([](const char* variableName, void* variable) -> bool
	{
		const char* rowNames[3] = { "Row 1", "Row 2", "Row 3" };

		ImGui::Text(variableName);
		bool hit = false;
		for (int i = 0; i < 3; i++)
		{
			hit |= ImGui::DragFloat3(rowNames[i], static_cast<float*>(variable) + i * 3);
		}
		return hit;
	});
	AddType<Matrix4x4f>([](const char* variableName, void* variable) -> bool
	{
		const char* rowNames[4] = { "Row 1", "Row 2", "Row 3", "Row 4" };

		ImGui::Text(variableName);
		bool hit = false;
		for (int i = 0; i < 4; i++)
		{
			hit |= ImGui::DragFloat3(rowNames[i], static_cast<float*>(variable) + i * 4);
		}
		return hit;
	});
}

KE_EDITOR::EditorInspectionSystem::~EditorInspectionSystem()
{
}

void KE_EDITOR::EditorInspectionSystem::Inspect(void* object, KE_SER::Data& serializationData)
{
	if (myInspectionFunctions.find(serializationData.typeName) == myInspectionFunctions.end()) { return; }

	char* variable = ((char*)object) + serializationData.offset;
	myInspectionFunctions[serializationData.typeName](BeautifyMemberName(serializationData.name).c_str(), variable);
}

const std::string& KE_EDITOR::EditorInspectionSystem::BeautifyMemberName(const std::string& aMemberName)
{
	if (beautifiedMemberNames.find(aMemberName) == beautifiedMemberNames.end())
	{
		std::string beautifiedName = aMemberName;

		//remove the my prefix
		if (beautifiedName.find("my") == 0)
		{
			beautifiedName = beautifiedName.substr(2);
		}

		//add spaces before capitals
		beautifiedName = std::regex_replace(beautifiedName, std::regex("([a-z])([A-Z])"), "$1 $2");

		//capitalize first letter
		beautifiedName[0] = static_cast<char>(toupper(beautifiedName[0]));

		//add spaces before numbers
		beautifiedName = std::regex_replace(beautifiedName, std::regex("([a-zA-Z])([0-9])"), "$1 $2");

		beautifiedMemberNames[aMemberName] = beautifiedName;
	}

	return beautifiedMemberNames[aMemberName];
}

#endif
