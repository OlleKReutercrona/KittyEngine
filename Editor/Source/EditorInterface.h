#pragma once


#ifndef KITTYENGINE_NO_EDITOR
namespace KE_EDITOR
{
	class Editor;
	class ImGuiHandler;
}

#define IF_EDITOR(x) x

#define KE_EDITOR_FRIEND friend class KE_EDITOR::Editor; friend class KE_EDITOR::ImGuiHandler;

#include <Editor/Source/EditorWindows/Window.h>
#include <Editor/Source/EditorWindows/Console.h>
#include <Editor/Source/EditorWindows/LambdaWindow.h>

namespace KE_GLOBAL
{
	inline KE_EDITOR::Editor* editor = nullptr;
	inline KE_EDITOR::ConsoleInternal console;

}
#define KE_LOG(message, ...) KE_GLOBAL::console.LogMessage("log", __FILE__, __LINE__, __FUNCTION__, message, __VA_ARGS__)
#define KE_WARNING(message, ...) KE_GLOBAL::console.LogMessage("warning", __FILE__, __LINE__, __FUNCTION__, message, __VA_ARGS__)
#define KE_ERROR(message, ...) KE_GLOBAL::console.LogMessage("error", __FILE__, __LINE__, __FUNCTION__, message, __VA_ARGS__)
#define KE_LOG_CHANNEL(channel, message, ...) KE_GLOBAL::console.LogMessage(channel, __FILE__, __LINE__, __FUNCTION__, message, __VA_ARGS__)

namespace KE_EDITOR
{
	struct EditorPreloadData
	{
		std::vector<KE_EDITOR::LambdaWindowInput> lambdaWindows;
	};

	inline EditorPreloadData editorPreload;
}

#define ADD_LAMBDA_WINDOW(name, lambda) KE_EDITOR::editorPreload.lambdaWindows.push_back({name, lambda})

#else

#define IF_EDITOR(x)
#define KE_EDITOR_FRIEND
#define KE_LOG(message, ...)
#define KE_WARNING(message, ...)
#define KE_ERROR(message, ...)
#define KE_LOG_CHANNEL(channel, message, ...)
#define ADD_LAMBDA_WINDOW(name, lambda)

#endif