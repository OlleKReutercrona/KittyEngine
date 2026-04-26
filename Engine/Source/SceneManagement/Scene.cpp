#include "stdafx.h"
#include "Scene.h"
#pragma message("MEOW")
#include "Engine/Source/Windows/Window.h"
#include "Engine/Source/Graphics/Camera.h"
#include "Engine/Source/Graphics/DebugRenderer.h"
#include "Engine/Source/Utility/DebugTimeLogger.h"

#ifndef KITTYENGINE_NO_EDITOR
#include <Editor/Source/Editor.h>
#endif



KE::Scene::Scene(const int aSceneID, const std::string aSceneName, const int aBuildIndex) :
	sceneID(aSceneID),
	sceneName(aSceneName),
	buildIndex(aBuildIndex)
{
	myDrawDebugFlags |= (int)SceneDrawFlags::eDrawGameObject;

#ifdef RETAIL
	myDrawDebugFlags |= (int)SceneDrawFlags::eDrawNavmesh;
	myDrawDebugFlags |= (int)SceneDrawFlags::eDrawPathfinding;
	myDrawDebugFlags |= (int)SceneDrawFlags::eDrawPhysX
#endif // RETAIL
}

KE::Scene::~Scene()
{}

void KE::Scene::ToggleDrawDebugFlag(const SceneDrawFlags aFlag)
{
	myDrawDebugFlags = myDrawDebugFlags ^ (int)aFlag;
}

bool KE::Scene::GetDrawFlag(const SceneDrawFlags aFlag)
{
	return (myDrawDebugFlags & ((int)aFlag));
}


void KE::Scene::Init(Window* aWindow, PrefabHandler* aPFHandler)
{
	myWindow = aWindow;

	myCollisionHandler.Init();
	gameObjectManager.Init(this);
	myPrefabHandler = aPFHandler;


	Camera* mainCamera = aWindow->GetGraphics().GetCameraManager().GetCamera(KE_MAIN_CAMERA_INDEX);
	myRaycastHandler.Init(*mainCamera, myCollisionHandler);

	if (myNavmeshPath.size() != 0)
	{
		bool initSuccess = myNavmesh.Init(&myWindow->GetGraphics(), myNavmeshPath);

		if (!initSuccess)
		{
			KE_ERROR("Scene failed to load Navmesh for file: %s", myNavmeshPath.c_str());
			return;
		}

		myPathfinder.Init(myNavmesh);
	}
}

void KE::Scene::RegisterNavmesh(std::string& aObjFilepath)
{
	myNavmeshPath = aObjFilepath;
}

void KE::Scene::AddChildParentPair(const int aParent, const int aChild)
{
	myToBeParents.emplace(std::pair(aChild, aParent));
}

void KE::Scene::AssignAdoptedChildren()
{
	for (auto& pair : myToBeParents)
	{
		auto child = gameObjectManager.GetGameObject(pair.first);
		auto parent = gameObjectManager.GetGameObject(pair.second);

		gameObjectManager.AddChild(*parent, *child);
	}

	myToBeParents.clear();
}

void KE::Scene::Activate()
{
	gameObjectManager.RegisterToBlackboard();
	myCollisionHandler.RegisterToBlackboard();
	myCollisionHandler.AddNavmesh(myNavmesh);

	gameObjectManager.PreCalculateWorldTransforms();
}

void KE::Scene::Deactivate()
{
	gameObjectManager.Unload();
	myCollisionHandler.Unload();
}

void KE::Scene::UpdateHierarchy()
{
	gameObjectManager.UpdateHierarchy();
}

void KE::Scene::Update()
{
	if (GetAsyncKeyState(VK_F10) == SHORT_MIN + 1)
	{
		ToggleDrawDebugFlag(SceneDrawFlags::eDrawPhysX);
		ToggleDrawDebugFlag(SceneDrawFlags::eDrawNavmesh);
	}

	gameObjectManager.Awake();

	KE::DebugTimeLogger::BeginLogVar("Collision");
	myCollisionHandler.Update();
	KE::DebugTimeLogger::EndLogVar("Collision");

	gameObjectManager.EarlyUpdate();

	KE::DebugTimeLogger::BeginLogVar("GameObject Update");
	gameObjectManager.Update();
	KE::DebugTimeLogger::EndLogVar("GameObject Update");

	gameObjectManager.LateUpdate();

	myRaycastHandler.AssignCamera(
		*myWindow->GetGraphics().GetCameraManager().GetHighlightedCamera()
	);

}

void KE::Scene::DebugDraw(KE::DebugRenderer& aDrawer)
{
	KE::DebugTimeLogger::BeginLogVar("Draw GOs");
	if ((myDrawDebugFlags & (int)SceneDrawFlags::eDrawGameObject)) gameObjectManager.DebugDraw(aDrawer);
	KE::DebugTimeLogger::EndLogVar("Draw GOs");

	KE::DebugTimeLogger::BeginLogVar("Draw PhysX");
	if((myDrawDebugFlags & (int)SceneDrawFlags::eDrawPhysX)) myCollisionHandler.DebugDraw(aDrawer);
	KE::DebugTimeLogger::EndLogVar("Draw PhysX");

	KE::DebugTimeLogger::BeginLogVar("Draw Navmesh");
	if ((myDrawDebugFlags & (int)SceneDrawFlags::eDrawNavmesh)) myNavmesh.DebugRender(&myWindow->GetGraphics());
	if ((myDrawDebugFlags & (int)SceneDrawFlags::eDrawPathfinding)) myPathfinder.DebugRender(&myWindow->GetGraphics());
	KE::DebugTimeLogger::EndLogVar("Draw Navmesh");
}


KE::GameObject* KE::Scene::AddGameObject(const int anID, const std::string& aName, const Transform& aTransform, const bool isStatic)
{
	return gameObjectManager.CreateGameObject(anID, &aName, aTransform, isStatic);
}
