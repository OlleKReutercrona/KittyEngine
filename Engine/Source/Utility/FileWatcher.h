#pragma once
#include <Windows.h>
#include <iostream>
#include <string>

class FileWatcher {
private:
	std::wstring myWatchedPath;
	HANDLE myDirectoryHandle;
	BYTE myNotifyBuffer[1024];
	OVERLAPPED myOverlapped;
	bool isWatching;
	bool hasChanged;
	std::wstring lastChangedFile;

public:
	FileWatcher(const std::wstring& path);
	~FileWatcher();

	bool Update();
	// bool HasChanged() const;
	std::string GetLastChangedFile() const;
};