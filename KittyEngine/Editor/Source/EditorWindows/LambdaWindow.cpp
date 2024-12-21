#include "stdafx.h"
#include "LambdaWindow.h"

#include "Editor/Source/Editor.h"
#include "nlohmann/json.hpp"
#ifndef KITTYENGINE_NO_EDITOR

#include "DeferredView.h"

KE_EDITOR::LambdaWindow::LambdaWindow(EditorWindowInput aStartupData) : EditorWindowBase(aStartupData)
{
	if (aStartupData.has_value())
	{
		myFunctionData = std::any_cast<LambdaWindowInput>(aStartupData);
	}

	myName = "LambdaWindow";/*myFunctionData.name.c_str();*/
}

void KE_EDITOR::LambdaWindow::Init()
{

}

void KE_EDITOR::LambdaWindow::Update()
{

}

void KE_EDITOR::LambdaWindow::Render()
{
	if (myFunctionData.func)
	{
		myFunctionData.func();
	}
}

void KE_EDITOR::LambdaWindow::Serialize(void* aWorkingData)
{
	auto& data = *(nlohmann::json*)aWorkingData;
	auto funcName = GetWindowName();
	data["name"] = funcName;
}

void KE_EDITOR::LambdaWindow::Deserialize(void* aWorkingData)
{
	auto& data = *(nlohmann::json*)aWorkingData;
	auto funcName = data["name"].get<std::string>();

	myFunctionData = KE_GLOBAL::editor->myLambdaWindowRegistry[funcName];
}

#endif