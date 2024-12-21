#pragma once

namespace KE
{
	class GameObject;
}

namespace KE_EDITOR
{
	
	class SelectedObject : public EditorWindowBase
	{
	private:
		KE::GameObject* mySelectedObject = nullptr;
		bool myIsLocked = false;

	public:
		SelectedObject(EditorWindowInput aStartupData = {}) : EditorWindowBase(aStartupData) {}
		void Init() override;
		void Update() override;
		void Render() override;

		const char* GetWindowName() const override { return "Selected Object"; }
	};

}
