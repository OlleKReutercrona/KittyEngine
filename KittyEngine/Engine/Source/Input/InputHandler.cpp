#include "stdafx.h"
#include "InputHandler.h"
#include <string>
#include <cassert>

//#ifdef KITTYENGINE_SHIP //graphics, for resolution changes
//#include "Graphics/Graphics.h"
//#endif

#include "Utility/DebugTimeLogger.h"
#include "Utility/Logging.h"

#ifndef KITTYENGINE_NO_EDITOR
#include <Editor/Source/Editor.h>
#endif // 


namespace KE
{
	std::optional<InputHandler::RawDelta> InputHandler::ReadRawDelta()
	{
		if (myRawDeltaBuffer.empty())
		{
			return std::nullopt;
		}
		const RawDelta rawDelta = myRawDeltaBuffer.front();
		myRawDeltaBuffer.pop();
		return rawDelta;
	}

	void InputHandler::EnableRaw()
	{
		isRawMouseEnabled = true;
	}

	void InputHandler::DisableRaw()
	{
		isRawMouseEnabled = false;
	}

	bool InputHandler::RawEnabled() const
	{
		return isRawMouseEnabled;
	}

	void InputHandler::OnRawDelta(int aDx, int aDy)
	{
		myRawDeltaBuffer.push({ aDx, aDy });
	}

	InputHandler::InputHandler()
	{
		KE_GLOBAL::blackboard.Register("inputHandler", this);

		//HideCursor();
	}

	InputHandler::~InputHandler()
	{
	}

	bool InputHandler::IsKeyPressed(const int aKeyCode) const
	{
		assert(aKeyCode >= 0);
		assert(aKeyCode < MAX_KEYS);

		return !myPreviousKeyState[aKeyCode] && myCurrentKeyState[aKeyCode];
	}

	bool InputHandler::IsKeyDown(const int aKeyCode) const
	{
		assert(aKeyCode >= 0);
		assert(aKeyCode < MAX_KEYS);

		return myCurrentKeyState[aKeyCode];
	}

	bool InputHandler::IsKeyHeld(const int aKeyCode) const
	{
		assert(aKeyCode >= 0);
		assert(aKeyCode < MAX_KEYS);

		return myPreviousKeyState[aKeyCode] && myCurrentKeyState[aKeyCode];
	}

	bool InputHandler::IsKeyReleased(const int aKeyCode) const
	{
		assert(aKeyCode >= 0);
		assert(aKeyCode < MAX_KEYS);

		return myPreviousKeyState[aKeyCode] && !myCurrentKeyState[aKeyCode];
	}

	bool InputHandler::IsAnyKeyDown() const
	{
		return myCurrentKeyState.count() > 0;
	}

	bool InputHandler::UpdateEvents(const UINT aMsg, const WPARAM aWParam, const LPARAM aLParam)
	{
		switch (aMsg)
		{
			// Window
		case WM_ACTIVATE:
		{
			switch (aWParam)
			{
			case WA_ACTIVE:
				isWindowActive = true;
				break;
			case WA_CLICKACTIVE:
				isWindowActive = true;
				UpdateMousePosition();
				break;
			case WA_INACTIVE:
				isWindowActive = false;
				UnlockCursor();
				break;
			default:;
			}
			break;
		}
		// Keyboard
		case WM_SYSKEYDOWN:
		{
			myLiveKeyState[aWParam] = true;
			break;
		}
		case WM_SYSKEYUP:
		{
			myLiveKeyState[aWParam] = false;
			break;
		}
		case WM_KEYDOWN:
		{
			myLiveKeyState[aWParam] = true;
			break;
		}
		case WM_KEYUP:
		{
			myLiveKeyState[aWParam] = false;
			break;
		}
		// Mouse
		case WM_LBUTTONDOWN:
		{
			//LockCursorToWindow();
			myLiveKeyState[VK_LBUTTON] = true;
			break;
		}
		case WM_LBUTTONUP:
		{
			myLiveKeyState[VK_LBUTTON] = false;
			break;
		}
		case WM_MBUTTONDOWN:
		{
			myLiveKeyState[VK_MBUTTON] = true;
			break;
		}
		case WM_MBUTTONUP:
		{
			myLiveKeyState[VK_MBUTTON] = false;
			break;
		}
		case WM_RBUTTONDOWN:
		{
			myLiveKeyState[VK_RBUTTON] = true;
			break;
		}
		case WM_RBUTTONUP:
		{
			myLiveKeyState[VK_RBUTTON] = false;
			break;
		}
		case WM_XBUTTONDOWN:
		{
			const int xButton = GET_XBUTTON_WPARAM(aWParam);
			if (xButton == 1)
			{
				myLiveKeyState[VK_XBUTTON1] = true;
			}
			else
			{
				myLiveKeyState[VK_XBUTTON2] = true;
			}
			break;
		}
		case WM_XBUTTONUP:
		{
			const int xButton = GET_XBUTTON_WPARAM(aWParam);
			if (xButton == 1)
			{
				myLiveKeyState[VK_XBUTTON1] = false;
			}
			else
			{
				myLiveKeyState[VK_XBUTTON2] = false;
			}
			break;
		}
		case WM_MOUSEWHEEL:
		{
			myScrollDelta = GET_WHEEL_DELTA_WPARAM(aWParam);
			break;
		}
		case WM_MOUSEMOVE:
		{
			if (!isRawMouseEnabled)
			{
				UpdateMousePosition();
			}
			break;
		}
		case WM_INPUT:
		{
			{
				if (!isRawMouseEnabled)
				{
					break;
				}
				UINT size = {};
				// First get the size of the input data
				if (GetRawInputData(
					reinterpret_cast<HRAWINPUT>(aLParam),
					RID_INPUT,
					nullptr,
					&size,
					sizeof(RAWINPUTHEADER)) == -1)
				{
					// Bail msg processing if error
					break;
				}
				myRawBuffer.resize(size);
				// Read in the input data
				if (GetRawInputData(
					reinterpret_cast<HRAWINPUT>(aLParam),
					RID_INPUT,
					myRawBuffer.data(),
					&size,
					sizeof(RAWINPUTHEADER)) != size)
				{
					// Bail msg processing if error
					break;
				}
				// Process the raw input data
				auto& rawInput = reinterpret_cast<const RAWINPUT&>(*myRawBuffer.data());
				if (rawInput.header.dwType == RIM_TYPEMOUSE &&
					(rawInput.data.mouse.lLastX != 0 || rawInput.data.mouse.lLastY != 0))
				{
					OnRawDelta(rawInput.data.mouse.lLastX, rawInput.data.mouse.lLastY);
				}
				break;
			}
		}
		default:
			return false;
		}
		return true;
	}

	void InputHandler::Update()
	{
		myPreviousKeyState = myCurrentKeyState;
		myCurrentKeyState = myLiveKeyState;

		myPreviousMousePos = myCurrentMousePos;
		myCurrentMousePos = myLiveMousePos;
	}

	void InputHandler::SetCursorPosition(const int aX, const int aY)
	{
		SetCursorPos(aX, aY);
	}

	POINT InputHandler::GetMousePosition() const
	{
#ifndef KITTYENGINE_NO_EDITOR

		KE_EDITOR::Editor* editor = KE_GLOBAL::editor;
		if (editor && editor->IsEnabled())
		{
			POINT point;

			int cam = -1;
			Vector2i editorPos = editor->GetViewportMousePos(&cam);
			point.x = (LONG)editorPos.x;
			point.y = (LONG)editorPos.y;

			//if (point.x < 0)
			//{
			//	while (::ShowCursor(TRUE) < 0)
			//	{
			//		;
			//	}
			//}
			//else if(cam == 0)
			//{
			//
			//	while (::ShowCursor(FALSE) >= 0)
			//	{
			//		;
			//	}
			//}
			return point;
		}


#endif // !KITTYENGINE_NO_EDITOR

		return myCurrentMousePos;
	}

	POINT InputHandler::GetMouseDelta() const
	{
		return myMouseDelta;
	}

	short InputHandler::GetScrollDelta()
	{
		const short scrollDelta = myScrollDelta;
		myScrollDelta = 0;
		return scrollDelta;
	}

	bool InputHandler::IsMouseMoving() const
	{
		return myPreviousMousePos.x != myCurrentMousePos.x || myPreviousMousePos.y != myCurrentMousePos.y;
	}

	void InputHandler::HideCursor()
	{
		while (::ShowCursor(FALSE) >= 0)
		{
			;
		}
	}

	void InputHandler::ShowCursor()
	{
		while (::ShowCursor(TRUE) < 0)
		{
			;
		}
	}

	void InputHandler::UpdateMousePosition()
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(GetForegroundWindow(), &point);

		RECT rect;
		const HWND hWnd = GetForegroundWindow();
		if (GetClientRect(hWnd, &rect))
		{
			const int height = rect.bottom - rect.top;
			myLiveMousePos.x = point.x;
			myLiveMousePos.y = point.y;
		}
		else
		{
			myLiveMousePos = point;
		}

		myMouseDelta = { myLiveMousePos.x - myPreviousMousePos.x, myLiveMousePos.y - myPreviousMousePos.y };
	}

	void InputHandler::LockCursorToWindow()
	{
		if (isCursorLocked)
		{
			return;
		}
		RECT rect;
		const HWND hWnd = GetForegroundWindow();
		if (GetWindowRect(hWnd, &rect))
		{
			const int width = rect.right - rect.left;
			const int height = rect.bottom - rect.top;

			if (myCurrentMousePos.x >= 0 && myCurrentMousePos.x <= width &&
				myCurrentMousePos.y >= 0 && myCurrentMousePos.y <= height)
			{
				LockCursor(hWnd);
			}
		}
	}

	void InputHandler::LockCursor(const HWND& aWindow)
	{
		RECT rect;
		GetClientRect(aWindow, &rect);

		POINT upperLeft;
		upperLeft.x = rect.left;
		upperLeft.y = rect.top;

		POINT lowerRight;
		lowerRight.x = rect.right;
		lowerRight.y = rect.bottom;

		MapWindowPoints(aWindow, nullptr, &upperLeft, 1);
		MapWindowPoints(aWindow, nullptr, &lowerRight, 1);

		rect.left = upperLeft.x;
		rect.top = upperLeft.y;

		rect.right = lowerRight.x;
		rect.bottom = lowerRight.y;

		ClipCursor(&rect);
		isCursorLocked = true;
	}

	void InputHandler::UnlockCursor()
	{
		ClipCursor(nullptr);
		isCursorLocked = false;
	}
}
