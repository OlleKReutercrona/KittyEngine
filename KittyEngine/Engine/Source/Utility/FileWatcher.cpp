#include "stdafx.h"
#include "FileWatcher.h"

#include <algorithm>

FileWatcher::FileWatcher(const std::wstring& path)
    : myWatchedPath(path), isWatching(false), hasChanged(false)
{

    myDirectoryHandle = CreateFileW(
        myWatchedPath.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );

    myOverlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (myOverlapped.hEvent == NULL)
    {
        std::wcerr << L"Failed to create event for overlapped operation" << std::endl;
    }

    if (myDirectoryHandle == INVALID_HANDLE_VALUE)
    {
        std::wcerr << L"Failed to open the directory: " << path << std::endl;
    }
}

FileWatcher::~FileWatcher()
{
    if (isWatching)
    {
        CancelIo(myDirectoryHandle);
        isWatching = false;
    }

    CloseHandle(myDirectoryHandle);
}

bool FileWatcher::Update()
{
    if (myDirectoryHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    if (!isWatching || hasChanged)
    {
        //include all file changes
        DWORD notifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
			FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

        DWORD bytesReturned;
        isWatching = ReadDirectoryChangesW(
            myDirectoryHandle,
            myNotifyBuffer,
            sizeof(myNotifyBuffer),
            TRUE,
            notifyFilter,
            &bytesReturned,
            &myOverlapped,
            NULL
        );

        hasChanged = false;

        if (!isWatching)
        {
            DWORD error = GetLastError();

            if (error == ERROR_INVALID_PARAMETER)
			{
				std::wcerr << L"Invalid parameter when starting to watch directory: " << myWatchedPath << std::endl;
			}
			else if (error == ERROR_ACCESS_DENIED)
			{
				std::wcerr << L"Access denied when starting to watch directory: " << myWatchedPath << std::endl;
			}
			else if (error == ERROR_NOT_ENOUGH_MEMORY)
			{
				std::wcerr << L"Not enough memory when starting to watch directory: " << myWatchedPath << std::endl;
			}
			else if (error == ERROR_INVALID_HANDLE)
			{
				std::wcerr << L"Invalid handle when starting to watch directory: " << myWatchedPath << std::endl;
			}
			else if (error == ERROR_OPERATION_ABORTED)
			{
				std::wcerr << L"Operation aborted when starting to watch directory: " << myWatchedPath << std::endl;
			}
			else
			{
				std::wcerr << L"Failed to start watching directory: " << myWatchedPath << std::endl;
			}

            return false;
        }
    }

    DWORD bytesReturned;
    if (!GetOverlappedResult(myDirectoryHandle, &myOverlapped, &bytesReturned, FALSE))
    {
        // Overlapped operation is still in progress or there was an error
        DWORD error = GetLastError();
        if (error == ERROR_IO_INCOMPLETE)
        {
            // Operation is still in progress
            return false;
        }
        else
        {
            // Error occurred
            std::wcerr << L"GetOverlappedResult failed with error: " << error << std::endl;
            return false;
        }
    }

    // Notify buffer contains information about the changed file
    FILE_NOTIFY_INFORMATION* notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(myNotifyBuffer);

    // Check if the file name is empty, which means the directory itself was changed
    if (notifyInfo->FileNameLength == 0)
    {
        hasChanged = true;
    }
    else
    {
        // Convert the wide string to a narrow string, without causing conversion warnings
        lastChangedFile = std::wstring(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(wchar_t));
        std::string str(lastChangedFile.length(), 0);
        std::transform(lastChangedFile.begin(), lastChangedFile.end(), str.begin(), [](wchar_t c) { return (char)c; });


        if (notifyInfo->Action == FILE_ACTION_ADDED)
        {
            //std::wcout << L"File added: " << notifyInfo->FileName << std::endl;
            KE_LOG_CHANNEL("fileWatcher", "File Added: %s", str.c_str());
        }
        if (notifyInfo->Action == FILE_ACTION_REMOVED)
        {
            //std::wcout << L"File removed: " << notifyInfo->FileName << std::endl;
            KE_LOG_CHANNEL("fileWatcher", "File Removed: %s", str.c_str());
        }
        if (notifyInfo->Action == FILE_ACTION_MODIFIED)
        {
            //std::wcout << L"File modified: " << notifyInfo->FileName << std::endl;
            KE_LOG_CHANNEL("fileWatcher", "File Modified: %s", str.c_str());
        }
        if (notifyInfo->Action == FILE_ACTION_RENAMED_OLD_NAME)
        {
            //std::wcout << L"File renamed old name: " << notifyInfo->FileName << std::endl;
            KE_LOG_CHANNEL("fileWatcher", "File Renamed Old Name: %s", str.c_str());
        }
        if (notifyInfo->Action == FILE_ACTION_RENAMED_NEW_NAME)
        {
            //std::wcout << L"File renamed new name: " << notifyInfo->FileName << std::endl;
            KE_LOG_CHANNEL("fileWatcher", "File Renamed New Name: %s", str.c_str());
        }

        hasChanged = true;
    }

    // Reset the overlapped structure for the next operation
    ZeroMemory(&myOverlapped, sizeof(myOverlapped));

    return hasChanged;
}

std::string FileWatcher::GetLastChangedFile() const
{
    // Convert the wide string to a narrow string, without causing conversion warnings

    std::string str(lastChangedFile.length(), 0);
    std::transform(lastChangedFile.begin(), lastChangedFile.end(), str.begin(), [](wchar_t c) { return (char)c; });

    return str;
}