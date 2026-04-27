#pragma once
#include "Engine/Source/Audio/GlobalAudio.h"
#include "Window.h"

namespace KE {
	class SceneManager;
}
namespace AI {
	class AIManager;
}

namespace KE_EDITOR {
	class EditSounds : public EditorWindowBase {
		KE_EDITOR_FRIEND
	public:
		const char* GetWindowName() const override {
			return "Edit Sounds";
		}
		void Init() override;
		void Update() override;
		void Render() override;

	private:
		EditSounds(EditorWindowInput aStartupData = {})
			: EditorWindowBase(aStartupData) {}

		void Save();
		void Load();
		// const char* EnumParser(sound::SFX aType);
		// const char* EnumParser(sound::Music aType);
		// const char* EnumParser(sound::Ambient aType);
		AI::AIManager* AIManager = nullptr;
		int mySelectedType = -1;
	};
}  // namespace KE_EDITOR