
#include "stdafx.h"

#include <codecvt>

#include "NodeEditor.h"
#include "SceneManagement/SceneManager.h"
#include "Script/ScriptManager.h"
#include "Utility/DebugTimeLogger.h"
#ifndef KITTYENGINE_NO_EDITOR
#include "Editor/Source/EditorWindows/Window.h"
#include "AssetBrowser.h"
#include "../ImGui/ImGuiHandler.h"
#include "../Editor.h"
#include <Engine/Source/Graphics/Graphics.h>
#include "Engine/Source/Utility/StringUtils.h"



#define IS_IMAGE(x) ((x) == ".png" || (x) == ".PNG" || (x) == ".dds" || (x) == ".DDS" || (x) == ".jpg" || (x) == ".JPG" || (x) == ".jpeg" || (x) == ".JPEG")
#define IS_CUBEMAP(x) ((x).find("cubemap") != std::string::npos || (x).find("Cubemap") != std::string::npos || (x).find("CubeMap") != std::string::npos)

#define IS_MODEL(x) ((x) == ".fbx" || (x) == ".FBX" || (x) == ".obj" || (x) == ".OBJ")

KE_EDITOR::EditorAssetBrowser::EditorAssetBrowser(EditorWindowInput aStartupData) :
	myEngineFileWatcher(
		L"../Engine/",
		[this](const std::wstring& path, const filewatch::Event change_type)
		{
			ProcessFileChangeEngine(path, change_type);
		}),
	myBinFileWatcher(
		L"Data",
		[this](const std::wstring& path, const filewatch::Event change_type)
		{
			ProcessFileChangeBin(path, change_type);
		})
{
	myFileTree = {};
}

KE_EDITOR::EditorAssetBrowser::~EditorAssetBrowser()
{
	//delete all nodes
}

void KE_EDITOR::EditorAssetBrowser::Init()
{
	myCurrentPath = myBasePath;

	myFileFilter.resize(256);
	myFileFilter = "";

	GenerateEditorFiles();
}

//std::string WideStringToNarrow(const std::wstring& wide)
//{
//	std::string str(wide.length(), 0);
//	std::transform(wide.begin(), wide.end(), str.begin(), [](wchar_t c) {
//		return (char)c;
//	});
//	return str;
//}

void KE_EDITOR::EditorAssetBrowser::ProcessFileChangeEngine(const std::wstring& path, const filewatch::Event change_type)
{
	//std::wcout << path << L" : ";
	switch (change_type)
	{
	case filewatch::Event::modified:
		if(path.ends_with(L".hlsl"))
		{
			//const std::string file = std::string(path.begin(), path.end()); //this way gives a warning
			//convert to string in a safer way, as the path is a wstring
			const std::string file = WideStringToNarrow(path);

			std::string sub = file.substr(file.find_last_of("\\") + 1);
			KE_GLOBAL::editor->myShaderLoader->RecompileShader(sub);
		}

		break;
	}


}
void KE_EDITOR::EditorAssetBrowser::ProcessFileChangeBin(const std::wstring& path, const filewatch::Event change_type)
{
	//std::wcout << path << L" : ";
	switch (change_type)
	{
	case filewatch::Event::added:
	case filewatch::Event::modified:
	case filewatch::Event::renamed_new:
	{
		if (makingThumbnail/* && path.ends_with(L".fbx.dds")*/)
		{
			return;
		}

		if (path.ends_with(L"export.done"))
		{
			KE_GLOBAL::editor->mySceneManager->ReloadScene();
			shouldRegenerate = true;

			std::string file = WideStringToNarrow(path);
			std::filesystem::remove("Data/" + file);

			break;
		}
	}
	case filewatch::Event::removed:
	{
			
	}
	default:break;
	};
}


void KE_EDITOR::EditorAssetBrowser::RecursiveFolderTreeGen(const std::filesystem::path& aPath, FileTreeNode* aNode)
{
	if (!std::filesystem::exists(aPath))
	{
		return;
	}

	for (auto& entry : std::filesystem::directory_iterator(aPath))
	{
		std::filesystem::path path = entry.path();
		std::filesystem::path fileName = entry.path().filename();

		if (entry.is_directory())
		{
			aNode->myChildren.emplace_back();
			aNode->myFolders.emplace_back(EditorFileType::eFolder, path.string(), fileName.string(), "../Editor/EditorAssets/Icons/Folder.png");

			RecursiveFolderTreeGen(entry.path(), &aNode->myChildren.back());
		}
		else
		{
			if(IS_IMAGE(fileName.extension()))
			{
				if (IS_CUBEMAP(fileName.string()))
				{
					aNode->myFiles.emplace_back(EditorFileType::eCubemap, path.string(), fileName.string(), "../Editor/EditorAssets/Icons/CubemapIcon.png");
				}
				else
				{
					aNode->myFiles.emplace_back(EditorFileType::eTexture, path.string(), fileName.string(), path.string());
				}
			}
			else if (IS_MODEL(fileName.extension()))
			{
				std::string modelThumbnailPath = "Data/InternalAssets/ModelThumbnails/" + fileName.string() + ".dds";
				if (!myModelsToThumbnail.contains(path.string()))
				{
					if (!std::filesystem::exists(modelThumbnailPath))
					{
						myModelsToThumbnail[path.string()] = true;
					}
				}

				aNode->myFiles.emplace_back(EditorFileType::eModel, path.string(), fileName.string(), modelThumbnailPath);
			}
			else if (fileName.extension() == ".kittyVFX")
			{
				aNode->myFiles.emplace_back(EditorFileType::eVFXSequence, path.string(), fileName.string(), "../Editor/EditorAssets/Icons/KittyVFXIcon.png");
			}
			else if (fileName.extension() == ".scritch")
			{
				aNode->myFiles.emplace_back(EditorFileType::eScript, path.string(), fileName.string(), "../Editor/EditorAssets/Icons/ScritchIcon.png");
			}
			else
			{
				aNode->myFiles.emplace_back(EditorFileType::eUnknown, path.string(), fileName.string(), "../Editor/EditorAssets/Icons/File.png");
			}

		}
	}
}

void KE_EDITOR::EditorAssetBrowser::GenerateEditorFiles()
{
	myFiles["..."] = EditorFile(EditorFileType::eFolder, "...", "...", "../Editor/EditorAssets/Icons/Return.png");

	myFileTree.myRoot = FileTreeNode();
	FileTreeNode* treePointer = &myFileTree.myRoot;
	myFileTree.myActiveNode = &myFileTree.myRoot;

	const std::filesystem::path path = myBasePath;
	RecursiveFolderTreeGen(path, treePointer);
}

void KE_EDITOR::EditorAssetBrowser::Regenerate()
{
	auto openStack = myFileTree.myOpenedStack;
	myFileTree = {};

	myFilesToDisplay.clear();
	myFiles.clear();
	//myModelsToThumbnail.clear();

	GenerateEditorFiles();

	myFileTree.myOpenedStack = openStack;
}

void KE_EDITOR::EditorAssetBrowser::GenerateThumbnails(const int aNumber)
{
	makingThumbnail = true;
	int iterations = 0;

	for (auto& [key, value] : myModelsToThumbnail)
	{
		if (!value) { continue; }

		RenderModelThumbnail(KE_GLOBAL::editor->myGraphics, key, true);
		myModelsToThumbnail[key] = false;

		iterations++;
		if (iterations >= aNumber) { break; }
	}

	if (iterations == 0)
	{
		allThumbnailsGenerated = true;
	}
}

void KE_EDITOR::EditorAssetBrowser::Update()
{
	if (!allThumbnailsGenerated)
	{
		GenerateThumbnails(1);
	}
	GenerateThumbnails(1);
	myFilesToDisplay.clear();

	if (shouldRegenerate)
	{
		Regenerate();
		shouldRegenerate = false;
	}

	makingThumbnail = false;
}

void KE_EDITOR::EditorAssetBrowser::RenderFileGrid(const ImVec2& aContentRegionSize)
{
	const float iconSize = 64.0f;
	const float textVArea = 40.0f;

	const unsigned int columnCount = (unsigned int)(aContentRegionSize.x / (iconSize + 10.0f));
	if (columnCount == 0) { return; }

	bool caseSensitive = false;
	static bool globalFilter = false;
	static char buffer[64] = {};

	if (ImGui::Button("Refresh")) { Regenerate(); }
	ImGui::SameLine();
	if (ImGui::InputText("Search", buffer, 64))
	{
		myFileFilter = buffer;
		for (auto& c : myFileFilter)
		{
			if (std::isupper(c)) { caseSensitive = true; } //this is evil ass visual studio style implementation :(
		}
	}


	ImGui::SameLine();
	ImGui::Checkbox("Global Filter", &globalFilter);

	ImGui::BeginTable("FileGrid", columnCount, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ScrollY);

	std::vector<EditorFile> files;
	std::vector<EditorFile> folders;

	if (globalFilter && !myFileFilter.empty())
	{
		for (auto& [fPath, file] : myFiles)
		{
			if (!PassesFilter(file.GetName(), myFileFilter, caseSensitive)) { continue; }
			if (file.GetType() == EditorFileType::eFolder)
			{
				folders.push_back(file);
			}
			else
			{
				files.push_back(file);
			}
		}
	}
	else
	{
		folders = myFileTree.myActiveNode->myFolders;
		files = myFileTree.myActiveNode->myFiles;
	}

	if (myFileTree.myOpenedStack.size() > 0)
	{
		ImGui::TableNextColumn();
		switch (myFiles["..."].Display(KE_GLOBAL::editor->myTextureLoader))
		{
		case EditorFileInteraction::eLeftDoubleClick:
		{
			myFileTree.Ascend();
			myCurrentPath = myCurrentPath.parent_path();
			break;
		}
		default: {}
		}

	}
	KE::DebugTimeLogger::BeginLogVar("DisplayFolder");
	for (int i = 0; i < myFileTree.myActiveNode->myFolders.size(); i++)
	{
		auto& folder = myFileTree.myActiveNode->myFolders[i];
		if (!PassesFilter(folder.GetName(), myFileFilter, caseSensitive)) { continue; }

		ImGui::TableNextColumn();

		switch (folder.Display(KE_GLOBAL::editor->myTextureLoader))
		{
		case EditorFileInteraction::eLeftDoubleClick:
		{
			myCurrentPath /*/*/ = folder.GetPath();
			myFileTree.Descend(i);
			break;
		}
		default: {}
		}

	}
	KE::DebugTimeLogger::EndLogVar("DisplayFolder");

	KE::DebugTimeLogger::BeginLogVar("DisplayFile");
	for (int i = 0; i < files.size(); i++)
	{
		auto& file = files[i];
		if (!PassesFilter(file.GetName(), myFileFilter, caseSensitive)) { continue; }

		ImGui::TableNextColumn();
		switch (file.Display(KE_GLOBAL::editor->myTextureLoader))
		{
		case EditorFileInteraction::eLeftDoubleClick:
		{
			switch (file.GetType())
			{
			case EditorFileType::eVFXSequence:
			{
				KE::VFXManager& mgr = KE_GLOBAL::editor->myGraphics->GetVFXManager();
				std::string fileName = file.GetName();
				//remove extension
				fileName = fileName.substr(0, fileName.find_last_of("."));
				// also remove path
				fileName = fileName.substr(fileName.find_last_of("/") + 1);

				ImGuiHandler::vfxEditorActiveSequenceIndex = mgr.GetVFXSequenceFromName(fileName);
				break;
			}
			case EditorFileType::eScript:
			{
				KE::ScriptManager* mgr = KE_GLOBAL::blackboard.Get<KE::ScriptManager>("scriptManager");
				std::string fileName = file.GetName();
				//remove extension
				fileName = fileName.substr(0, fileName.find_last_of("."));
				// also remove path
				fileName = fileName.substr(fileName.find_last_of("/") + 1);

				KE::Script* script = mgr->GetScript(fileName);
				if (!script)
				{
					mgr->LoadScript(fileName, file.GetPath());
					script = mgr->GetScript(fileName);
				}
				if (script)
				{
					auto scriptEditorWindows = KE_GLOBAL::editor->GetEditorWindowsOfType<NodeEditor>();
					bool found = false;
					for (auto& window : scriptEditorWindows)
					{
						auto* windowScript = window->GetScript();
						if (!windowScript)
						{
							window->SetScript(script);

							found = true;
							break;
						}
						if (windowScript == script)
						{
							found = true;
							break;
						}

					}
					if (!found)
					{
						NodeEditor* nodeEditor = (NodeEditor*)KE_GLOBAL::editor->myWindowRegistry["NodeEditor"].myCreationFunc({});
						nodeEditor->SetScript(script);
					}

				}

				break;
			}
			default:
			{
				std::filesystem::path filePath = std::filesystem::absolute(myCurrentPath);
				filePath /= file.GetPath();

				ShellExecuteA(NULL, "open", filePath.string().c_str(), NULL, NULL, NULL);
				break;
			}
			}
			break;
		}
		case EditorFileInteraction::eHovered:
		{
			switch (file.GetType())
			{
			case EditorFileType::eModel:
			{
				if (ImGui::BeginTooltip())
				{
					KE::RenderTarget* rt = nullptr;
					RenderModelThumbnail(KE_GLOBAL::editor->myGraphics, file.GetPath(), false, &rt);
					if (rt)
					{
						ImGui::Image(rt->GetShaderResourceView(), ImVec2(256.0f, 256.0f));
					}
					ImGui::EndTooltip();
				}
				break;
			}
			default: {}
			}
		}
		default: {}
		}
	}
	KE::DebugTimeLogger::EndLogVar("DisplayFile");

	ImGui::EndTable();
}

void KE_EDITOR::EditorAssetBrowser::DrawNodeChildFolders(FileTreeNode& aNode)
{
	for (int i = 0; i < aNode.myFolders.size(); i++)
	{
		if (ImGui::TreeNode(aNode.myFolders[i].GetName().c_str()))
		{
			DrawNodeChildFolders(aNode.myChildren[i]);
			ImGui::TreePop();
		}
	}
}

void KE_EDITOR::EditorAssetBrowser::Render()
{
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(0.12f, 0.12f, 0.12f, 1.0f).Value);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	KE::DebugTimeLogger::BeginLogVar("NodeChildFolders");
	if (ImGui::BeginChild("Sidebar", { 0,0 }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_Border))
	{
		DrawNodeChildFolders(myFileTree.myRoot);
	}
	ImGui::EndChild();
	KE::DebugTimeLogger::EndLogVar("NodeChildFolders");

	ImGui::SameLine();

	KE::DebugTimeLogger::BeginLogVar("FileGrid");
	if (ImGui::BeginChild("FileGrid", { 0,0 }, ImGuiChildFlags_Border))
	{
		const ImVec2 contentArea = ImGui::GetContentRegionAvail();
		RenderFileGrid(contentArea);
	}
	KE::DebugTimeLogger::EndLogVar("FileGrid");
	ImGui::EndChild();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

bool KE_EDITOR::EditorAssetBrowser::PassesFilter(const std::string& aFileName, const std::string& aFilter, bool aCaseSensitive)
{
	if (aFilter.empty()) { return true; }

	std::string fileName = aFileName;
	std::string filter = aFilter;

	if (!aCaseSensitive)
	{
		for (auto& c: fileName) { c = std::tolower(c); }
		for (auto& c: filter) { c = std::tolower(c); }
	}

	if (fileName.find(filter) != std::string::npos)
	{
		return true;
	}
	return false;
}

#endif
