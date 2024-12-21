#pragma once
#include "InputHandler.h"
#include "Engine/Source/Math/Vector2.h"
#include "Gamepad.h"

namespace KE
{
	struct InputEvent;

	class InputWrapper
	{
	public:
		InputWrapper();
		void Update();
		bool UpdateEvents(UINT aMsg, WPARAM aWParam, LPARAM aLParam);
		Vector2f GetMovement() const;
		bool GetIsEnabled() const;
		void SetEnabled(bool aValue);
		bool IsKeyPressed(int aKeyCode) const;
		bool IsKeyDown(int aKeyCode) const;
		bool IsKeyHeld(int aKeyCode) const;
		bool IsKeyReleased(int aKeyCode) const;
		bool IsAnyKeyDown() const;
		std::optional<InputHandler::RawDelta> ReadRawDelta();
		void EnableRaw();
		void DisableRaw();
		bool IsLMBPressed() const;
		bool IsLMBHeld() const;
		bool IsLMBReleased() const;
		bool IsRMBPressed() const;
		bool IsRMBHeld() const;
		bool IsRMBReleased() const;
		bool IsMMBPressed() const;
		bool IsMMBHeld() const;
		bool IsMMBReleased() const;
		Vector2f GetMousePosition() const;
		short GetScrollDelta();
		void UpdateControllerStatus();
		int GetKeyboardID(Key& aKey) const;
		Gamepad* GetGamepad(int aIndex);

	private:
		bool PressedKeyThisFrame(const std::vector<Key>& aMouseAndKeyboardKeys, Key& aOutKey) const;
		bool ReleasedKeyThisFrame(const std::vector<Key>& aMouseAndKeyboardKeys, Key& aOutKey) const;
		bool HeldKeyThisFrame(const std::vector<Key>& aMouseAndKeyboardKeys, Key& aOutKey) const;
		std::vector<InputEvent*> myInputEvents;
		bool isInputEnabled = true;

		// Input devices
		InputHandler myInputHandler;
		Gamepad myGamepads[4] = { Gamepad(0), Gamepad(1), Gamepad(2), Gamepad(3) };

		// Mouse & Keyboard
		std::vector<Key> myKeysUp;
		std::vector<Key> myKeysDown;
		std::vector<Key> myKeysLeft;
		std::vector<Key> myKeysRight;
		std::vector<Key> myKeysLeftClick;
		std::vector<Key> myKeysRightClick;
		std::vector<Key> myKeysMiddleClick;
		std::vector<Key> myKeysAction1;
		std::vector<Key> myKeysAction2;
		std::vector<Key> myKeysAction3;
		std::vector<Key> myKeysAction4;
		std::vector<Key> myKeysEsc;
		std::vector<Key> myKeysEnter;
		std::vector<Key> myKeysTab;
		std::vector<Key> myKeysShift;
		std::vector<Key> myKeysSpace;
		std::vector<Key> myKeysF1;
		std::vector<Key> myKeysF2;
		std::vector<Key> myKeysF3;
		std::vector<Key> myKeysF4;
		std::vector<Key> myKeysF5;
		std::vector<Key> myKeysF6;
	};
}
