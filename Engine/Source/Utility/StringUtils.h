#pragma once

// Returns a narrowed version of a wide string. Use at your own risk.
void WideStrToNarrowStr(const wchar_t* aString, char* anOutArray);

void NarrowStrToWideStr(const char* aString, wchar_t* anOutArray);

std::string WideStringToNarrow(const std::wstring& wide);

std::wstring NarrowStringToWide(const std::string& aNarrow);