#include "stdafx.h"
#include "BuildManager.h"

#include <filesystem>
#include <fstream>

#include <ctime>
#include <time.h>

#include <shobjidl_core.h>
#include <Windows.h>
#include "Engine/Source/Utility/StringUtils.h"
#include "Engine/Source/Utility/Timer.h"

#include <thread>

//void KE::BuildManager::SpecifyBuildPath(const std::string& aPath)
//{
//	myBuildPath = aPath;
//}



void KE::BuildManager::StartBuild()
{
	std::thread buildThread(&KE::BuildManager::Build);

	buildThread.detach();
}

void KE::BuildManager::SetBuildDirectory()
{
	if (myBinPath.empty())
	{
		myBinPath = std::filesystem::current_path().wstring();
	}

	myBuildPath = BrowseFolder(myBinPath);
}

void KE::BuildManager::Build()
{
	if (myBinPath.empty())
	{
		myBinPath = std::filesystem::current_path().wstring();
	}


	if (myBuildPath.empty())
	{
		myBuildPath = BrowseFolder(myBinPath);
	}

	if (!myBuildPath.empty())
	{
		myNumberOfScannedFiles = 0;
		myNumberOfFiles = 0;

		unsigned int numberOfFiles = 0;
		//for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(myBinPath))
		//{
		//	numberOfFiles++;
		//}
		//myNumberOfFiles = numberOfFiles;

		Timer timer;

		isBuilding = true;

		std::wstring ignoredFiles(L"Ignored Files: \n");

		struct FileToCopy
		{
			std::filesystem::path myPath;
			std::wstring myDirectory;
		};

		std::vector<FileToCopy> pathsToCopy;
		//pathsToCopy.reserve(numberOfFiles);

		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(myBinPath))
		{
			std::wstring extension = dirEntry.path().extension().wstring();

			if (extension._Equal(L".png") || extension._Equal(L".pdb") || extension._Equal(L".fbx.dds") || extension._Equal(L".meow"))
			{
				ignoredFiles += dirEntry.path().relative_path().wstring();
				ignoredFiles += L"\n"; // <- This is crash and I dont know why DDDDDDD':

				std::wcout << L"Skipping file: " << dirEntry.path().relative_path().wstring() << std::endl;

				//myNumberOfScannedFiles++;
				continue;
			}


			std::wstring filePath = dirEntry.path().wstring();
			std::wstring dirPath = filePath.substr(myBinPath.size());

			if (extension == L".exe")
			{
				std::wstring filename = dirEntry.path().filename().wstring();

				if (filename.find(L"Moggie") != std::wstring::npos || filename.find(L"Hybrid") != std::wstring::npos)
				{
					ignoredFiles += dirEntry.path().relative_path().wstring();
					ignoredFiles += L"\n";

					std::wcout << L"Skipping file: " << dirEntry.path().relative_path().wstring() << std::endl;
					//
					//myNumberOfScannedFiles++;
					continue;
				}
				else
				{
					if (myExeName.empty())
					{
						myExeName = GAME_NAME;
						myExeName += L".exe";
					}
					//  This is the .exe we want so if we have name we rename it!!
					dirPath = '\\';
					dirPath += myExeName;
				}
			}



			std::wstring copyPath(myBuildPath + dirPath);
			if (!std::filesystem::is_directory(copyPath))
			{
				std::wstring filename = dirEntry.path().filename().wstring();

				size_t begin = copyPath.find_last_of(L"\\");
				std::wstring directoryPath = copyPath.substr(0, begin);

				std::filesystem::create_directory(directoryPath);
			}

			if (dirEntry.path().extension() == "")
			{
				std::wcout << L"Skipping path: " << dirEntry.path().relative_path().wstring() << std::endl;

				//myNumberOfScannedFiles++;

				continue;
			}

			pathsToCopy.push_back({ dirEntry.path(), copyPath });
			numberOfFiles++;

			//// Remove read-only from file
			//SetFileAttributes(copyPath.c_str(),
			//	GetFileAttributes(copyPath.c_str()) & ~FILE_ATTRIBUTE_READONLY);


			//constexpr auto options = std::filesystem::copy_options::overwrite_existing;

			//std::error_code error;

			//std::filesystem::copy_file(dirEntry.path(), copyPath, options, error);

			//if (error.value() > 0)
			//{
			//	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);

			//	std::wstring outStr;

			//	NarrowStrToWideStr(error.message().c_str(), (wchar_t*)outStr.c_str());

			//	std::wcout << L"FAILED TO COPY: " << ("%s", copyPath) << " ::: " << outStr << std::endl;

			//	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
			//}
		}

		myNumberOfFiles = numberOfFiles;


		// Copy files
		{
			for (int i = 0; i < pathsToCopy.size(); i++)
			{
				FileToCopy& file = pathsToCopy[i];

				// Remove read-only from file
				SetFileAttributes(file.myDirectory.c_str(),
					GetFileAttributes(file.myDirectory.c_str()) & ~FILE_ATTRIBUTE_READONLY);


				constexpr auto options = std::filesystem::copy_options::overwrite_existing;

				std::error_code error;

				std::filesystem::copy_file(file.myPath, file.myDirectory, options, error);

				myNumberOfScannedFiles++;

				if (error.value() > 0)
				{
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);

					std::wstring outStr;

					NarrowStrToWideStr(error.message().c_str(), (wchar_t*)outStr.c_str());

					std::wcout << L"FAILED TO COPY: " << ("%s", file.myDirectory) << " ::: " << outStr << std::endl;

					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
				}
			}
		}

		std::cout << "\n\n\n\nSUCCESSFULLY BUILT TO '" << WideStringToNarrow(myBuildPath) << "'\n    Time:" << ("%.4f", timer.UpdateDeltaTime()) << " seconds" << std::endl;

		if (!ignoredFiles.empty())
		{
			std::time_t t = std::time(0);
#pragma warning(push)
#pragma warning(disable : 4996)
			std::tm* now = std::localtime(&t);
#pragma warning(pop)

			std::string fileName(WideStringToNarrow(myBuildPath));
			fileName += "\\IgnoredFiles_";
			fileName += std::to_string(now->tm_year + 1900);
			fileName += '-';
			fileName += std::to_string(now->tm_mon + 1);
			fileName += '-';
			fileName += std::to_string(now->tm_mday);
			fileName += ".txt";

			std::ofstream outFile(fileName);

			outFile << WideStringToNarrow(ignoredFiles) << std::endl;

			outFile.close();

			std::cout << "Created a file with ignored files @ '" << fileName << "'\n";
		}

		myNumberOfScannedFiles++;
	}

	isBuilding = false;

	ShellExecute(NULL, (LPCWSTR)L"open", (LPCWSTR)myBuildPath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

void KE::BuildManager::ScanBinForPNG()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(myBinPath))
	{
		if (dirEntry.path().extension() == ".png")
		{
			std::cout << dirEntry.path().relative_path() << std::endl;
			continue;
		}
	}
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

}

void KE::BuildManager::AssignEXEName(const std::wstring& aEXEName)
{
	myExeName = aEXEName;
	myExeName += L".exe";
}

void KE::BuildManager::ScanAndRemovePNGFromBuild()
{
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(myBuildPath))
	{
		if (dirEntry.path().extension() == ".png")
		{
			std::cout << "Removing " << dirEntry.path().filename() << " from " << dirEntry << std::endl;
			std::filesystem::remove(dirEntry.path());

			continue;
		}
	}
}


const std::wstring KE::BuildManager::BrowseFolder(const std::wstring& aSavedPath)
{
	LPWSTR path[MAX_PATH];

	IFileDialog* pfd;
	HRESULT result;
	result = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(result))
	{
		DWORD dwOptions;
		result = pfd->GetOptions(&dwOptions);
		if (SUCCEEDED(result))
		{
			pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
		}
		result = pfd->Show(NULL);
		if (SUCCEEDED(result))
		{
			IShellItem* psi;
			result = pfd->GetResult(&psi);
			if (SUCCEEDED(result))
			{
				result = psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, path);
				if (!SUCCEEDED(result))
				{
					MessageBox(NULL, L"GetIDListName() failed", NULL, NULL);
				}
				psi->Release();
			}
		}
		pfd->Release();

		if (SUCCEEDED(result))
		{
			return path[0];
		}
	}

	return aSavedPath;
}