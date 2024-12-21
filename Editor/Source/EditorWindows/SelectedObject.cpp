#include "stdafx.h"
#ifndef KITTYENGINE_NO_EDITOR
#include "Window.h"
#include "SelectedObject.h"

#include "Editor/Source/Editor.h"
#include "Editor/Source/EditorInterface.h"
#include "SceneManagement/SceneManager.h"

void KE_EDITOR::SelectedObject::Init()
{
}

void KE_EDITOR::SelectedObject::Update()
{
	if (myIsLocked) { return; }
	const int selectedID = KE_GLOBAL::editor->myData.gameObjectData.lastSelectedGameObject;
	if (selectedID != INT_MIN)
	{
		mySelectedObject = KE_GLOBAL::editor->mySceneManager->GetCurrentScene()->GetGameObjectManager().GetGameObject(selectedID);
	}
	else
	{
		mySelectedObject = nullptr;
	}
}

void KE_EDITOR::SelectedObject::Render()
{
	if (mySelectedObject == nullptr) { return; }
	
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImColor(0.12f, 0.12f, 0.12f, 1.0f).Value);
	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
	ImGui::BeginChild("Selected Object Child", ImVec2(0, 0), ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_Border);
	ImGui::Text(mySelectedObject->GetName().c_str());
	ImGui::SameLine();
	ImGui::Text("( Layer: %s )", KE::Collision::GetLayerAsString(mySelectedObject->myLayer).c_str());
	ImGui::SameLine(ImGui::GetWindowWidth() - 65.0f);
	/*ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1.0f, 1.0f));
	ImGui::Checkbox("Lock", &myIsLocked);
	ImGui::PopStyleVar();*/
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();

	ImGuiHandler::DisplayTransform(&mySelectedObject->myTransform, eSeparatorAbove);

	for (auto& component : mySelectedObject->GetComponentsRaw())
	{
		ImGuiHandler::ComponentDisplayDispatch(component);
	}

}
#endif