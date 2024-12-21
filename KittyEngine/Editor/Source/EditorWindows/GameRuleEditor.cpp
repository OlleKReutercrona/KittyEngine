#include "stdafx.h"
#include "GameRuleEditor.h"
#include "Editor/Source/Editor.h"

#ifndef KITTYENGINE_NO_EDITOR

void KE_EDITOR::GameRuleEditor::Init()
{
	Load();
}

void KE_EDITOR::GameRuleEditor::Update()
{
}

void KE_EDITOR::GameRuleEditor::Render()
{
	ImGui::Text("Game rule editor");
	ImGui::Spacing();
	{
		ImGui::Text("PlayerMovementSpeed: ");
		ImGui::SameLine();
		ImGui::InputFloat("PlayerSpeed", &myCurrentSelectedGameRuleSet->PlayerMovementSpeed);
	}
	{
		ImGui::Text("PlayerThrowingDistance: ");
		ImGui::SameLine();
		ImGui::InputFloat("PlayerThrowDist", &myCurrentSelectedGameRuleSet->PlayerThrowDistance);
	}
	{
		ImGui::Checkbox("ItemSpawn", &myCurrentSelectedGameRuleSet->ItemSpawn);
	}
	if (ImGui::Button("Save"))
	{
		Save();
	}
}

void KE_EDITOR::GameRuleEditor::Save()
{
	P8::GameRules::GetInstance()->ChangeGameRules(myCurrentSelectedGameRuleSet);
}

void KE_EDITOR::GameRuleEditor::Load()
{
	myCurrentSelectedGameRuleSet = P8::GameRules::GetInstance()->GetGameRules();
}

#endif // KITTYENGINE_NO_EDITOR