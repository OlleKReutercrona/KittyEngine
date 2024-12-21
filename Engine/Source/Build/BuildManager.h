#pragma once


namespace KE
{
	class BuildManager
	{
	public:
		//static void SpecifyBuildPath(const std::string& aPath);
		static void StartBuild();

		static void SetBuildDirectory();
		static void ScanBinForPNG();
		static void AssignEXEName(const std::wstring& aEXEName);

		static inline float GetProgress() { return (float)myNumberOfScannedFiles / (float)myNumberOfFiles ; }
		static inline std::wstring GetApplicationName() { return myExeName; }

		static inline bool IsBuilding() { return isBuilding; }
	private:
		static void Build();
		static void ScanAndRemovePNGFromBuild();
		static const std::wstring BrowseFolder(const std::wstring& aSavedPath);
	private:
		static inline std::wstring myExeName;
		static inline std::wstring myBuildPath;
		static inline std::wstring myBinPath;

		static inline bool isBuilding = false;
		static inline unsigned int myNumberOfFiles = 0;
		static inline unsigned int myNumberOfScannedFiles = 0;
	};
}

