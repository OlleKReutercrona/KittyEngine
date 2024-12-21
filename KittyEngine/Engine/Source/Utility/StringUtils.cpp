#include "stdafx.h"
#include <string.h>

void WideStrToNarrowStr(const wchar_t* aString, char* anOutArray)
{
	for (unsigned int i = 0; i < wcslen(aString); i++)
	{
		anOutArray[i] = (char)aString[i];
	}
}

void NarrowStrToWideStr(const char* aString, wchar_t* anOutArray)
{
	for (unsigned int i = 0; i < strlen(aString); i++)
	{
		anOutArray[i] = (wchar_t)aString[i];
	}
}

std::string WideStringToNarrow(const std::wstring& wide)
{
	std::string str(wide.length(), 0);
	std::transform(wide.begin(), wide.end(), str.begin(), [](wchar_t c)
		{
			return (char)c;
		});
	return str;
}

std::wstring NarrowStringToWide(const std::string& aNarrow)
{
	std::wstring str(aNarrow.length(), 0);
	std::transform(aNarrow.begin(), aNarrow.end(), str.begin(), [](char c)
		{
			return (wchar_t)c;
		});
	return str;
}