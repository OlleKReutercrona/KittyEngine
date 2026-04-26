#pragma once
#include "Engine/Source/Utility/EventSystem.h"

namespace KE
{
	class RaycastHandler;

	/// Class for handling inputs made from the user, sending them to
	///	the appropriate systems. (Player, UI, etc.)

	class UserInput final : ES::IObserver
	{
	public:
		UserInput();
		~UserInput() override;

		void Init();
		void Update();
		void SetIsMouseOnGUI(bool aIsMouseOnGUI);

		// IObserver
		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		void OnDestroy() override;

	private:
		bool isMouseOverGUI = false;
	};
}
