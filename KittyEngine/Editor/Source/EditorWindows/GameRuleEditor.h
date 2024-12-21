#pragma once
#include "Window.h"
#include "Project/Source/GameRules/GameRules.h"

#ifndef KITTYENGINE_NO_EDITOR
namespace KE_EDITOR
{
	class GameRuleEditor : EditorWindowBase
	{
		KE_EDITOR_FRIEND
	public:

		const char* GetWindowName() const override { return "Edit GameRules"; }

		void Init() override;
		void Update() override;
		void Render() override;

	private:
		GameRuleEditor(EditorWindowInput aStartupData = {}) : EditorWindowBase(aStartupData) {}

		void Save();
		void Load();

		P8::GameRuleSet* myCurrentSelectedGameRuleSet;
	};
}
#endif // KITTYENGINE_NO_EDITOR