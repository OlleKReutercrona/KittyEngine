#include "stdafx.h"
#ifndef KITTYENGINE_NO_EDITOR
#include "Window.h"
#include "GameObjectList.h"

#include "Editor/Source/Editor.h"
#include "SceneManagement/SceneManager.h"

void KE_EDITOR::GameObjectList::Init()
{
}

void KE_EDITOR::GameObjectList::Update()
{
}

void KE_EDITOR::GameObjectList::Render()
{
	const std::vector<KE::GameObject*>& _gameObjectList = KE_GLOBAL::editor->mySceneManager->GetCurrentScene()->GetGameObjectManager().GetGameObjects();
	const std::vector<KE::GameObject*>& _newGameObjectList = KE_GLOBAL::editor->mySceneManager->GetCurrentScene()->GetGameObjectManager().GetNewGameObjects();

	const std::vector<KE::GameObject*>& gameObjectList = _gameObjectList.size() > 0 ? _gameObjectList : _newGameObjectList;

	//if (ImGui::Button("Recache"))
	//{
	//	CreateGameObjectCache(gameObjectList);
	//}
	//ImGui::SameLine();

	static char inputBuffer[32] = "";
	static bool cacheGenerated = false;
	if (GameObjectCacheDirty(gameObjectList))
	{
		CreateGameObjectCache(gameObjectList);
	}

	static std::string filterStr = "";

	float textHeight = ImGui::GetTextLineHeightWithSpacing();

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(0.12f, 0.12f, 0.12f, 1.0f).Value);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	if (ImGui::BeginChild("Object List Child", {0,0}, ImGuiChildFlags_Border))
	{
		if (ImGui::BeginChild("Filter Child", ImVec2(0, 20)))
		{
			if (ImGui::InputText("Filter", inputBuffer, 32))
			{
				filterStr = inputBuffer;
				std::transform(filterStr.begin(), filterStr.end(), filterStr.begin(), ::tolower);
				CalculateGameObjectFilter(gameObjectList, filterStr);
			}
		}
		ImGui::EndChild();
		for (int i = 0; i < gameObjectList.size(); ++i)
		{
			KE::GameObject* gameObject = gameObjectList[i];
			if (gameObject->GetParent() != nullptr) { continue; }
			DisplayListEntry(gameObject);
		}

		if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
		{
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				KE_GLOBAL::editor->myData.gameObjectData.DeselectAll();
			}
			else if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("ObjectListPopup");
			}

		}

		if (ImGui::BeginPopup("ObjectListPopup"))
		{
			if (ImGui::MenuItem("Create Empty"))
			{
				int id = KE_GLOBAL::editor->mySceneManager->GetCurrentScene()->GetGameObjectManager().GenerateUniqueID();
				std::string name = "test item";
				KE_GLOBAL::editor->mySceneManager->GetCurrentScene()->GetGameObjectManager().CreateGameObject(id, &name);
			}
			ImGui::EndPopup();
		}

	}
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}

bool KE_EDITOR::GameObjectList::GameObjectCacheDirty(const std::vector<KE::GameObject*>& aGameObjectList)
{
	//return false;
	if (/*gameObjectDataCache.size()*/ lastGameObjectCount != aGameObjectList.size())
	{
		lastGameObjectCount = aGameObjectList.size();	
		return true;
	}
	return false;
}

void KE_EDITOR::GameObjectList::CreateGameObjectCache(const std::vector<KE::GameObject*>& aGameObjectList)
{
	gameObjectDataCache.clear();

	for (size_t i = 0; i < aGameObjectList.size(); i++)
	{
		GameObjectDataCache newCache;
		newCache.assembledName = FormatString("(%i) : %s", aGameObjectList[i]->myID, aGameObjectList[i]->GetName().c_str());
		newCache.objectName = aGameObjectList[i]->GetName();
		newCache.searchName = newCache.assembledName;
		std::transform(newCache.searchName.begin(), newCache.searchName.end(), newCache.searchName.begin(), ::tolower);


		gameObjectDataCache[aGameObjectList[i]->myID] = newCache;
	}
}

void KE_EDITOR::GameObjectList::CalculateGameObjectFilter(const std::vector<KE::GameObject*>& aGameObjectList, const std::string& aNameFilter)
{
	for (size_t i = 0; i < aGameObjectList.size(); i++)
	{
		//if (aGameObjectList[i]->myParent != nullptr) { continue; } //skip children

		if (aNameFilter.size() > 0)
		{
			size_t found = gameObjectDataCache[aGameObjectList[i]->myID].searchName.find(aNameFilter);
			if (found != std::string::npos)
			{
				gameObjectDataCache[aGameObjectList[i]->myID].matchesFilter = true;
				gameObjectDataCache[aGameObjectList[i]->myID].filterMatchCharacterStart = (int)found;
				gameObjectDataCache[aGameObjectList[i]->myID].filterMatchCharacterEnd = (int)found + (int)aNameFilter.size();
			}
			else
			{
				gameObjectDataCache[aGameObjectList[i]->myID].matchesFilter = false;
				gameObjectDataCache[aGameObjectList[i]->myID].filterMatchCharacterStart = 0;
				gameObjectDataCache[aGameObjectList[i]->myID].filterMatchCharacterEnd = 0;
			}

			gameObjectDataCache[aGameObjectList[i]->myID].childrenMatchesFilter = false;
			for (KE::GameObject* child : aGameObjectList[i]->GetChildren())
			{
				if (gameObjectDataCache[child->myID].searchName.find(aNameFilter) != std::string::npos)
				{
					gameObjectDataCache[aGameObjectList[i]->myID].childrenMatchesFilter = true;
				}
			}
		}
		else
		{
			gameObjectDataCache[aGameObjectList[i]->myID].matchesFilter = true;
			gameObjectDataCache[aGameObjectList[i]->myID].childrenMatchesFilter = true;
		}
	}
}

void KE_EDITOR::GameObjectList::DisplayListEntry(KE::GameObject* aGameObject)
{
	int id = aGameObject->myID;
	GameObjectDataCache& cache = gameObjectDataCache[id];
	bool selected = KE_GLOBAL::editor->myData.gameObjectData.IsGameObjectSelected(id);
	bool active = aGameObject->IsActive();

	if (!(cache.matchesFilter || cache.childrenMatchesFilter))
	{
		return;
	}

	if (cache.assembledName == "")
	{
		return;
	}

	ImGuiTreeNodeFlags flags;
	flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
	if (selected)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
	}
	if (aGameObject->GetChildren().size() == 0)
	{
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	bool open = false;
	if (cache.matchesFilter && active)
	{
		open = ImGui::TreeNodeEx(cache.assembledName.c_str(), flags);
	}
	else
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1));
		open = ImGui::TreeNodeEx(cache.assembledName.c_str(), flags);
		ImGui::PopStyleColor();
	}

	//check if node was clicked, but not on the arrow
	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		if (!ImGui::GetIO().KeyCtrl)
		{
			KE_GLOBAL::editor->myData.gameObjectData.DeselectAll();
		}
		KE_GLOBAL::editor->myData.gameObjectData.SelectObject(id);
	}

	//if the node is hovered, display its name over any other text
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip(cache.assembledName.c_str());
	}

	if (open)
	{
		for (KE::GameObject* child : aGameObject->GetChildren())
		{
			DisplayListEntry(child);
		}

		ImGui::TreePop();
	}
}
#endif