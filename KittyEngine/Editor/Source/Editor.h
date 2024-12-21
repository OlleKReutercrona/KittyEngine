#pragma once
#include <functional>

#include "Inspector/EditorInspection.h"

#ifndef KITTYENGINE_NO_EDITOR
#include "Editor/Source/ImGui/ImGuiHandler.h"
#include "EditorData.h"

#include "EditorWindows/ModelViewer.h"

//#include <Engine/Source/Utility/FileWatcher.h>
#include <External/Include/FileWatch/FileWatch.hpp>

#include <Engine/Source/Graphics/Camera.h>
#include <Editor/Source/EditorGraphics.h>
#include <Editor/Source/EditorWindows/LambdaWindow.h>

namespace KE
{
	class Graphics;
	class Window;
	class ShaderLoader;
	class TextureLoader;
	class ModelLoader;
	class GameObjectManager;
	class Timer;
	class SceneManager;
	struct LoggedVar;
}


// this macro should 
#define REGISTER_EDITOR_WINDOW(x, hidden) myWindowRegistry[#x] = {\
	[&](const std::any& aStartupData) -> EditorWindowBase*  { return CreateEditorWindow<x>(myWindowIndex++, #x, aStartupData); },\
	hidden \
	}

namespace KE_EDITOR
{
	typedef std::function<EditorWindowBase* (const std::any& aStartupData)> WindowCreationFunc;

	struct WindowRegistryEntry
	{
		WindowCreationFunc myCreationFunc;
		bool hidden = false;
	};

	class Editor
	{
		friend class ModelViewer;
		friend class ImGuiHandler;
		friend class EditorAssetBrowser;
		friend class EditorConsole;
		
	protected:
		KE::Graphics* myGraphics;
		KE::Window* myWindow;

		KE::Timer* myTimer;

		KE::ShaderLoader* myShaderLoader;
		KE::TextureLoader* myTextureLoader;
		KE::ModelLoader* myModelLoader;

		KE::Camera myCamera;

		unsigned int myWindowIndex = 0;
		std::vector<EditorWindowBase*> myWindows;

		EditorInspectionSystem myInspectionSystem;

		//filewatch::FileWatch<std::wstring> myFileWatcher;

		void HandleDragDropQueue();

		void SaveMeowFile();
		void LoadMeowFile();
		void ResetMeowFile();
		void SavePostProcessSettings();
		void LoadPostProcessSettings();
		void SaveKittyMeshFile();
		void LoadKittyMeshFile();
		void ClearNavmesh();
		void SaveGUIFile();
		void LoadGUIFile();
	public:
		KE::SceneManager* mySceneManager;
		EditorData myData;
		std::unordered_map<std::string, WindowRegistryEntry> myWindowRegistry;
		std::unordered_map<std::string, LambdaWindowInput> myLambdaWindowRegistry;

		Editor();
		~Editor();

		void Init(
			KE::Window* aWindow,
			KE::Timer* aTimer,

			KE::ShaderLoader* aShaderLoader,
			KE::TextureLoader* aTextureLoader,
			KE::ModelLoader* aModelLoader,
			KE::SceneManager* aGameObjectManager
		);
		static ImGuiDockNode* ExtendDockTabBar();
		void Update();

		void ExtractLogChildren(const KE::LoggedVar& log, const float aFrameTime);

		void RegisterLambdaWindow(const std::string& aName, LambdaWindowFunc aLambda);

		//EditorConsole* GetConsole() { return &myConsole; }

		template <typename T>
		T* CreateEditorWindow(unsigned int index, const char* aName, const std::any& aStartupData)
		{
			myWindows.push_back(new T(aStartupData));
			myWindows.back()->Init();
			myWindows.back()->SetID(index);
			myWindows.back()->SetName(aName);
			return (T*)myWindows.back();
		}

		template <typename T>
		T* GetEditorWindow()
		{
			for (auto& window : myWindows)
			{
				if (dynamic_cast<T*>(window))
				{
					return static_cast<T*>(window);
				}
			}

			return nullptr;
		}

		template <typename T>
		std::vector<T*> GetEditorWindowsOfType()
		{
			std::vector<T*> windows;

			for (auto& window : myWindows)
			{
				if (dynamic_cast<T*>(window))
				{
					windows.push_back(static_cast<T*>(window));
				}
			}

			return windows;
		}

		std::vector<EditorWindowBase*>& GetWindows() { return myWindows; }



		void BeginFrame();
		void EndFrame();

		void Shutdown();

		void ReceiveDragDrop(const std::string& aPath);

		bool IsEnabled() const;

		Vector2i GetViewportMousePos(int* anOutCamIndex = nullptr);

		KE::Box GetModelBoundsExtremes(const std::vector<const ModelBounds*>& aModelBoundsList, const Transform& aTransform);
		KE::GameObject* GetHoveredGameObject(
			const std::vector<KE::GameObject*>& aGameObjectList,
			const Vector2f& anEditorMousePos
		);
		static bool PointInsideTriangle(Vector3f p0, Vector3f p1, Vector3f p2, Vector3f point);
		static bool RayIntersectsModel(KE::MeshList* aMeshList, const Rayf& aRay, const Transform& aTransform);
		static bool RayIntersectsModel(KE::SkeletalMeshList* aMeshList, const Rayf& aRay, const Transform& aTransform);

		bool HighlightObject(KE::GameObject* aGameObject);	

		void ControlCamera(KE::Camera* aCamera);
	};

	//inline Editor* editorInstance;
}
#endif
