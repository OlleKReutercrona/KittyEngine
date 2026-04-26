#pragma once

#ifndef KITTYENGINE_NO_EDITOR

namespace KE
{
	class GBuffer;
	class SSAO;
}

namespace KE_EDITOR
{
	class DeferredView : public EditorWindowBase
	{
		KE_EDITOR_FRIEND
	private:
		KE::GBuffer* myBuffer;
		KE::SSAO* mySSAO;
	public:
		DeferredView(EditorWindowInput aStartupData = {}) : EditorWindowBase(aStartupData) {}

		const char* GetWindowName() const override { return "Deferred View"; }
		void Init() override;
		void Update() override;
		void Render() override;
	};
}

#endif