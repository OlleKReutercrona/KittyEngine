#pragma once
#include <vector>
#include <array>
#include <string>
#include <unordered_map>

struct PerformanceData
{
	float deltaTime   = 0.0f;
	float lowestTime  = FLT_MAX;
	float highestTime = FLT_MIN;

	float Fps()			{ return 1.0f / deltaTime;	 }
	float LowestFps()	{ return 1.0f / highestTime; }
	float HighestFps()	{ return 1.0f / lowestTime;	 }

	void Register(float aTimeDelta) 
	{ 
		deltaTime = aTimeDelta;
		lowestTime = aTimeDelta < lowestTime ? aTimeDelta : lowestTime;
		highestTime = aTimeDelta > highestTime ? aTimeDelta : highestTime;
	}
};

struct DragAndDropData
{
	std::vector<std::string> files;
};

struct GameObjectData
{
	std::unordered_map<int, bool> selectedGameObjects;
	int lastSelectedGameObject = INT_MIN;
	
	bool IsGameObjectSelected(int anId)
	{
		return selectedGameObjects.find(anId) != selectedGameObjects.end();
	}
	
	bool SelectObject(int anId)
	{
		if (IsGameObjectSelected(anId))
			return false;

		selectedGameObjects[anId] = true;
		lastSelectedGameObject = anId;
		return true;
	}

	bool DeselectObject(int anId)
	{
		if (!IsGameObjectSelected(anId))
			return false;

		selectedGameObjects.erase(anId);
		return true;
	}

	void DeselectAll()
	{
		selectedGameObjects.clear();
		lastSelectedGameObject = INT_MIN;
	}
};

struct ActiveData
{
	char editorState = 0; //0 = active, 1 = fullscreen editor, 2 = fullscreen game
};

struct InputData
{
	//bool clickIntercepted = false;
};

enum class DebugRenderLevel : char
{
	eNone = 0,
	eBasic = 1,
	eFull = 2
};

struct DebugRenderData
{
	DebugRenderLevel renderNavmesh = DebugRenderLevel::eNone;
	DebugRenderLevel renderSkeleton = DebugRenderLevel::eNone;
};

struct EditorData
{
	PerformanceData performance;
	DragAndDropData dragDropData;
	GameObjectData gameObjectData;
	ActiveData activeData;
	InputData inputData;
	DebugRenderData debugRenderData;

	bool FullscreenToggleQueued = false;
};