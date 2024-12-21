#pragma once
#include <stdio.h>
#include <Windows.h>

#define LogDivider "+===================================================================================================+\n"

#define LogLevelWarning "WARNING"

#define LogLevelError "ERROR"

#define LogLevelInfo "INFO"

#define Log(level, stream, msg, ...) \
const char* relFilePath = strstr(__FILE__, "Project"); \
	fprintf(stream, LogDivider level " IN ..\\%s\nIN FUNCTION: " __FUNCSIG__ "\nAT LINE: %i.\n" msg "\n" LogDivider, relFilePath, __LINE__, __VA_ARGS__); \
	fflush(stream); \


/*
  Change the color of the text, print stuff out, then restore the color to what it was.
  The scope is there to avoid polluting the calling code with names that will lead to a name collision and thus a compilation error.
*/
#define kE_LogError(msg, ...) \
	{ \
		CONSOLE_SCREEN_BUFFER_INFO col; \
		HANDLE conH = GetStdHandle(STD_OUTPUT_HANDLE); \
		GetConsoleScreenBufferInfo(conH, &col); \
		SetConsoleTextAttribute(conH, FOREGROUND_RED); \
		Log(LogLevelError, stderr, msg, __VA_ARGS__); \
		SetConsoleTextAttribute(conH, col.wAttributes); \
	}


#define kE_LogWarning(msg, ...) \
	{ \
		CONSOLE_SCREEN_BUFFER_INFO col; \
		HANDLE conH = GetStdHandle(STD_OUTPUT_HANDLE); \
		GetConsoleScreenBufferInfo(conH, &col); \
		SetConsoleTextAttribute(conH, 14u); \
		Log(LogLevelWarning, stderr, msg, __VA_ARGS__); \
		SetConsoleTextAttribute(conH, col.wAttributes); \
	}

#define kE_LogMessage(msg, ...) Log(LogLevelInfo, stderr, msg, __VA_ARGS__)