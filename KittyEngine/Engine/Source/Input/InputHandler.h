#pragma once
#include <bitset>
#include <optional>
#include <queue>
#include <Windows.h>

#define MAX_KEYS 256

namespace KE
{
	using Key = unsigned int;

	class InputHandler
	{
	public:
		struct RawDelta
		{
			int myX, myY;
		};

		std::optional<RawDelta> ReadRawDelta();
		void EnableRaw();
		void DisableRaw();
		bool RawEnabled() const;

	private:
		void OnRawDelta(int aDx, int aDy);
		std::vector<BYTE> myRawBuffer;
		std::queue<RawDelta> myRawDeltaBuffer;
		bool isRawMouseEnabled = false;

	public:
		InputHandler();
		~InputHandler();
		bool IsKeyPressed(int aKeyCode) const;
		bool IsKeyDown(int aKeyCode) const;
		bool IsKeyHeld(int aKeyCode) const;
		bool IsKeyReleased(int aKeyCode) const;
		bool IsAnyKeyDown() const;
		bool UpdateEvents(UINT aMsg, WPARAM aWParam, LPARAM aLParam);
		void Update();
		void SetCursorPosition(int aX, int aY);
		POINT GetMousePosition() const;
		POINT GetMouseDelta() const;
		short GetScrollDelta();
		bool IsMouseMoving() const;

		void HideCursor();
		void ShowCursor();

	private:
		void UpdateMousePosition();
		void LockCursorToWindow();
		void LockCursor(const HWND& aWindow);
		void UnlockCursor();

	private:
		bool isWindowActive = false;
		bool isCursorLocked = false;
		POINT myPreviousMousePos = {0, 0};
		POINT myCurrentMousePos = {0, 0};
		POINT myLiveMousePos = {0, 0};
		POINT myMouseDelta = {0, 0};
		short myScrollDelta = 0;
		std::bitset<MAX_KEYS> myPreviousKeyState;
		std::bitset<MAX_KEYS> myCurrentKeyState;
		std::bitset<MAX_KEYS> myLiveKeyState;
	};
}
