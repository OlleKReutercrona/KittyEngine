#include "stdafx.h"
#include "Editor/Source/ImGui/ImGuiHandler.h"
#include "Window.h"

namespace KE_EDITOR
{
	bool EditorWindowBase::Begin()
	{
		StyleBegin();
		bool open = true;
		if (myDockNodeID != 0) { ImGui::SetNextWindowDockID(myDockNodeID); myDockNodeID = 0; }

		ImGuiWindowClass windowClass;
		windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoCloseButton;
		ImGui::SetNextWindowClass(&windowClass);

		myStatus = ImGui::Begin(FormatString("%s##%i", GetWindowName(), GetID()), &open, myWindowFlags) ? WindowStatus::Open : WindowStatus::Hidden;

		if (!open) { myStatus = WindowStatus::Closed; }

		return myStatus == WindowStatus::Open;
	}

	void EditorWindowBase::End()
	{
		ImGui::End();
		StyleEnd();
	}

	ImGuiID EditorWindowBase::GetDockNodeID()
	{
		return ImGui::GetWindowDockID();
	}
}
