#pragma once

#include "../EditorGraphics.h"
#include "../EditorFile.h"
#include <filesystem>
#include <External/Include/FileWatch/FileWatch.hpp>

namespace KE_EDITOR
{
	struct FileTreeNodePath
	{
		std::vector<unsigned int> myTurnStack = {};
	};

	struct FileTreeNode
	{

		std::vector<FileTreeNode> myChildren;
		std::vector<EditorFile> myFiles;
		std::vector<EditorFile> myFolders;
	};

	struct FileTree
	{
		FileTreeNode myRoot;
		FileTreeNode* myActiveNode = nullptr;
		std::vector<unsigned int> myOpenedStack;
		
		FileTree() { myRoot = FileTreeNode(); }

		FileTreeNode* GetOpenedNode()
		{
			FileTreeNode* currentNode = &myRoot;

			for (unsigned int i = 0; i < myOpenedStack.size(); ++i)
			{
				if (currentNode->myChildren.size() <= myOpenedStack[i])
				{
					return currentNode;
				}
				currentNode = &currentNode->myChildren[myOpenedStack[i]];
			}

			return currentNode;
		}
		inline void Ascend()
		{
			myOpenedStack.pop_back();
			myActiveNode = GetOpenedNode();
		}
		inline void Descend(int anIndex)
		{
			if (myActiveNode->myChildren.size() > anIndex)
			{
				myOpenedStack.push_back(anIndex);
				myActiveNode = &myActiveNode->myChildren[anIndex];
			}
		}
	};

	class EditorAssetBrowser : public EditorWindowBase
	{
	private:
		const std::string myBasePath = "Data";
		std::filesystem::path myCurrentPath;
		const float myIconSize[2] = { 64.0f, 64.0f };

		filewatch::FileWatch<std::wstring> myEngineFileWatcher;
		filewatch::FileWatch<std::wstring> myBinFileWatcher;

		std::unordered_map<std::string, EditorFile> myFiles;
		FileTree myFileTree;

		std::vector<EditorFile*> myFilesToDisplay;
		std::string myFileFilter = "";

		std::unordered_map<std::string, bool> myModelsToThumbnail;
		bool allThumbnailsGenerated = false;

		bool makingThumbnail = false;

		bool shouldRegenerate = false;

	public:
		EditorAssetBrowser(EditorWindowInput aStartupData = {});
		~EditorAssetBrowser();
		
		void Init();

		void ProcessFileChangeEngine(const std::wstring& path, const filewatch::Event change_type);
		void ProcessFileChangeBin(const std::wstring& path, const filewatch::Event change_type);

		void RecursiveFolderTreeGen(const std::filesystem::path& aPath, FileTreeNode* aNode);
		void GenerateEditorFiles();

		void Regenerate();

		void GenerateThumbnails(const int aNumber = -1);

		void Update() override;
		void RenderFileGrid(const ImVec2& aContentRegionSize);
		void DrawNodeChildFolders(FileTreeNode& aNode);
		const char* GetWindowName() const override { return "Asset Browser"; }
		void Render() override;

		bool PassesFilter(const std::string& aFileName, const std::string& aFilter, bool aCaseSensitive);
	};
}