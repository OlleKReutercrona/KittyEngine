#pragma once
#include "Window.h"

namespace KE
{
	class Camera;
	class RenderTarget;
}

namespace KE_EDITOR
{
	class Viewport : public EditorWindowBase
	{
	private:
		char myName[32] = "Viewport";
		int myRenderTargetIndex = 0;
		int myCameraIndex = 0;

	public:
		Viewport(EditorWindowInput aStartupData = {}) : EditorWindowBase(aStartupData) {}
		void Init() override;
		void Update() override;
		void Render() override;
		const char* GetWindowName() const override { return myName; } //todo: dynamically set this!
		void StyleBegin() override;
		void StyleEnd() override;

		void SetData(const int aRenderTargetIndex, const int aCameraIndex, const char* aName);

		void Serialize(void* aWorkingData) override;
		void Deserialize(void* aWorkingData) override;
	};
	
}
