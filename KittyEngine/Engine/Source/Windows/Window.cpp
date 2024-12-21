#include "stdafx.h"
#include "Window.h"
//#include "Project/resource.h"

#include <External/Include/imgui/imgui_impl_win32.h>

#include <cassert>
#include <sstream>

#include <string>
#include <Lmcons.h>

#include "UI/GUIEvents.h"
#include "Utility/EventSystem.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#ifndef KE_NOEDITOR
#include <Editor/Source/Editor.h>
#endif
#include <Project\Source\GameEvents\GameEvents.h>

#define KE_FULLSCREENSTYLE WS_POPUP | WS_VISIBLE
#define KE_WINDOWEDSTYLE WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_SIZEBOX | WS_VISIBLE

namespace KE
{
	// Window Class
	Window::WindowClass Window::WindowClass::myWndClass;

	const wchar_t* Window::WindowClass::GetName()
	{
		return WND_CLASS_NAME;
	}

	HINSTANCE Window::WindowClass::GetInstance()
	{
		return myWndClass.myHInst;
	}

	Window::WindowClass::WindowClass() : myHInst(GetModuleHandle(nullptr))
	{
		WNDCLASSEX wc = { 0 };
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = HandleMsgSetup;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = GetInstance();
		wc.hIcon = static_cast<HICON>(LoadImage(myWndClass.myHInst, L"ID", IMAGE_ICON, 32, 32, 0));
		wc.hCursor = nullptr;
		wc.hbrBackground = nullptr;
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = GetName();
		wc.hIconSm = static_cast<HICON>(LoadImage(myWndClass.myHInst, L"ID", IMAGE_ICON, 16, 16, 0));
		RegisterClassEx(&wc);
	}

	Window::WindowClass::~WindowClass()
	{
		UnregisterClass(WND_CLASS_NAME, GetInstance());
	}

	Window::Window(const int aWidth, const int aHeight, const wchar_t* aName)
		: myStartName(aName)
	{
		myWidth = aWidth;
		myHeight = aHeight;
		myWindowedWidth = aWidth;
		myWindowedHeight = aHeight;

		OnInit();

		// Calculate window size based on desired client region size
		RECT wr = {};
		wr.left = 0;
		wr.right = aWidth + wr.left;
		wr.top = 0;
		wr.bottom = aHeight + wr.top;
		AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

		myHWnd = CreateWindow(
			WindowClass::GetName(),
			aName,
			WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU | WS_SIZEBOX,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			wr.right - wr.left,
			wr.bottom - wr.top,
			nullptr,
			nullptr,
			WindowClass::GetInstance(),
			this
		);
		// Check error
		assert(myHWnd && "hWnd is null");

		// Show window
		ShowWindow(myHWnd, SW_SHOWDEFAULT);

		//focus window
		SetForegroundWindow(myHWnd);

		RECT clientRect;
		GetClientRect(myHWnd, &clientRect);
		myWidth = clientRect.right - clientRect.left;
		myHeight = clientRect.bottom - clientRect.top;

		// Create graphics object
		myGraphics = std::make_unique<Graphics>(myHWnd, myWidth, myHeight);

		myGraphics->Init();

		// Initialise ImGui Win32 Impl
		//ImGui_ImplWin32_Init(hWnd);

		// Register mouse raw input device
		RAWINPUTDEVICE rid = {};
		rid.usUsagePage = 0x01; // Mouse page
		rid.usUsage = 0x02; // Mouse usage
		rid.dwFlags = 0;
		rid.hwndTarget = nullptr;
		RegisterRawInputDevices(&rid, 1, sizeof(rid));
		//MoveWindow(hWnd, 1920, 0, aWidth, aHeight, TRUE);
		ShowWindow(myHWnd, SW_SHOWDEFAULT);

		// Event
		myResolutionEvent = ES::EventSystem::GetInstance().CreateNewEvent<ResolutionEvent>();

#ifdef KITTYENGINE_NO_EDITOR
		SetFullscreen(true);
#endif
	}

	Window::~Window()
	{
		//ImGui_ImplWin32_Shutdown();
		OnDestroy();

		ES::EventSystem::GetInstance().HandleCleanup();
		DestroyWindow(myHWnd);
	}

	void Window::EnableCursor()
	{
		isCursorEnabled = true;
		ShowCursor();
		EnableImGuiMouse();
		FreeCursor();
	}

	void Window::DisableCursor()
	{
		isCursorEnabled = false;
		HideCursor();
		DisableImGuiMouse();
		ConfineCursor();
	}

	bool Window::IsCursorEnabled() const
	{
		return isCursorEnabled;
	}

	std::optional<int> Window::ProcessMessages()
	{
		MSG msg = {};
		// While queue has messages, remove and dispatch them
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// Check for quit because PeekMessage does not signal this via return
			if (msg.message == WM_QUIT)
			{
				// wParam here is the value passed to PostQuitMessage
				return static_cast<int>(msg.wParam);
			}
			TranslateMessage(&msg); // Posts auxilliary WM_CHAR messages from key msg
			DispatchMessage(&msg);
		}

		// Return empty optional
		return {};
	}

	Graphics& Window::GetGraphics() const
	{
		assert(myGraphics && "pGraphics is nullptr");
		return *myGraphics;
	}

	InputWrapper& Window::GetInputWrapper()
	{
		return myInputWrapper;
	}

	void Window::AddToWindowName(const wchar_t* aExtention)
	{
		std::wstring newName(myStartName + L" - " + aExtention);

		SetWindowTextW(myHWnd, newName.c_str());
	}

	void Window::SetWindowDims(int aX, int aY, int aWidth, int aHeight, bool aFullscreen)
	{
		aFullscreen;
		aWidth = aWidth == -1 ? myWidth : aWidth;
		aHeight = aHeight == -1 ? myHeight : aHeight;

		RECT wr = {};
		wr.left = aX;
		wr.right = aWidth + wr.left;
		wr.top = aY;
		wr.bottom = aHeight + wr.top;

		if (aFullscreen)
		{
			SetWindowLong(myHWnd, GWL_STYLE, KE_FULLSCREENSTYLE);
		}
		else
		{
			SetWindowLong(myHWnd, GWL_STYLE, KE_WINDOWEDSTYLE);
			AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);
		}

		aWidth = wr.right - wr.left;
		aHeight = wr.bottom - wr.top;

		//std::cout << aWidth << ", " << aHeight << std::endl;

		//adjust window size to account for window borders
		myWidth = aWidth;
		myHeight = aHeight;
		myWindowPosition = { aX, aY };
		SetWindowPos(myHWnd, nullptr, aX, aY, aWidth, aHeight, SWP_NOZORDER);
	}

	void Window::SetRenderResolution(const int aWidth, const int aHeight)
	{
		std::cout << "Window::SetRenderResolution CALLED with: " << aWidth << ", " << aHeight << ", isFullscreen " << (isFullscreen ? "true" : "false") << std::endl;

		myWidth = aWidth;
		myHeight = aHeight;
		myWindowedWidth = aWidth;
		myWindowedHeight = aHeight;
		myGraphics->SetRenderResolution(aWidth, aHeight);
		myResolutionEvent->myWidth = myWidth;
		myResolutionEvent->myHeight = myHeight;
		myResolutionEvent->myFullscreen = isFullscreen;
		ES::EventSystem::GetInstance().SendEvent(*myResolutionEvent);
	}

	void Window::SetFullscreen(bool aState, int aWidth, int aHeight)
	{
		std::cout << "Window::SetFullscreen CALLED with: " << (aState ? "true" : "false") << std::endl;

		/*if (isFullscreen == aState) { return; }*/
		isFullscreen = aState;

#ifdef KITTYENGINE_SHIP
		myGraphics->SetFullscreenStatus(aState);
#endif
		//get the screen that the window is on
		HMONITOR hmon = MonitorFromWindow(myHWnd, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = { sizeof(mi) };
		GetMonitorInfo(hmon, &mi);

		int x = mi.rcMonitor.left;
		int y = mi.rcMonitor.top;
		int width = mi.rcMonitor.right - mi.rcMonitor.left;
		int height = mi.rcMonitor.bottom - mi.rcMonitor.top;

		if (aState) //set fullscreen
		{
			myWindowedWidth = myWidth;
			myWindowedHeight = myHeight;

			myWidth = width;
			myHeight = height;

#ifndef KITTYENGINE_SHIP
			myGraphics->SetRenderResolution(width, height);
			SetWindowDims(x, y, myWidth, myHeight, true);
#else
			SetWindowDims(x, y, myWidth, myHeight, true);
			//myGraphics->SetRenderResolution(width, height);
#endif

		}
		else
		{
			//myWidth = myWindowedWidth;
			//myHeight = myWindowedHeight;

			myWidth = aWidth > 0 ? aWidth : myGraphics->GetRenderWidth();
			myHeight = aHeight > 0 ? aHeight : myGraphics->GetRenderHeight();

			SetWindowDims(x, y, myWidth, myHeight, false);
			//myGraphics->SetRenderResolution(myWidth, myHeight);
		}
	}

	void Window::ConfineCursor() const
	{
		RECT rect;
		GetClientRect(myHWnd, &rect);
		MapWindowPoints(myHWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
		ClipCursor(&rect);
	}

	void Window::FreeCursor()
	{
		ClipCursor(nullptr);
	}

	void Window::HideCursor()
	{
		while (::ShowCursor(FALSE) >= 0)
		{
			;
		}
	}

	void Window::ShowCursor()
	{
		while (::ShowCursor(TRUE) < 0)
		{
			;
		}
	}

	void Window::EnableImGuiMouse()
	{
		//ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
	}

	void Window::DisableImGuiMouse()
	{
		//ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
	}

	LRESULT WINAPI Window::HandleMsgSetup(HWND aHWnd, UINT aUMsg, WPARAM aWParam, LPARAM aLParam)
	{
		// Use create parameter passed in from CreateWindow() to store window class pointer at WinAPI
		if (aUMsg == WM_NCCREATE)
		{
			// Extract ptr to window class from creation data
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(aLParam);
			Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
			// Set WinAPI-managed user data to store ptr to window class
			SetWindowLongPtr(aHWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
			// Sett kessage proc to normal (non-setup) handler now that setup is finished
			SetWindowLongPtr(aHWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
			// Forward message to window class handler
			return pWnd->HandleMsg(aHWnd, aUMsg, aWParam, aLParam);
		}
		// If we get a message before the WM_NCCREATE message, handle with default handler
		return DefWindowProc(aHWnd, aUMsg, aWParam, aLParam);
	}

	LRESULT WINAPI Window::HandleMsgThunk(HWND aHWnd, UINT aUMsg, WPARAM aWParam, LPARAM aLParam)
	{
		// Retrieve ptr to window class
		Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(aHWnd, GWLP_USERDATA));
		// Forward message to window class handler
		return pWnd->HandleMsg(aHWnd, aUMsg, aWParam, aLParam);
	}

	LRESULT Window::HandleMsg(HWND aHWnd, UINT aUMsg, WPARAM aWParam, LPARAM aLParam)
	{
		myInputWrapper.UpdateEvents(aUMsg, aWParam, aLParam);
#ifndef KITTYENGINE_NO_EDITOR

		if (ImGui_ImplWin32_WndProcHandler(aHWnd, aUMsg, aWParam, aLParam))
		{
			//return true;
		}

		//more compact imgui event stifling:
		if ((aUMsg >= WM_KEYDOWN && aUMsg <= WM_SYSKEYUP && ImGui::GetIO().WantCaptureKeyboard) ||
			(aUMsg >= WM_MOUSEMOVE && aUMsg <= WM_MOUSEWHEEL && ImGui::GetIO().WantCaptureMouse))
		{
			//return DefWindowProc(aHWnd, aUMsg, aWParam, aLParam); //maybe change this to just skipping the switch?
		}

#endif // NO_IMGUI
		//
		//
		switch (aUMsg)
		{
			// We don't want the DefProc to handle this message because
			// we want our destructor to destroy the window, so return 0 instead of break

		//case WM_SETCURSOR:
		//{
		//	if (LOWORD(aLParam) == HTCLIENT)
		//	{
		//		SetCursor(LoadCursorFromFile(L"cursor.cur"));
		//		return TRUE;
		//	}
		//	break;
		//}

		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}
		// Clear keystates when window loses focus to prevent input getting stuck
		case WM_KILLFOCUS:
		{
			//myInputWrapper.GetKeyboard().ClearKeyStates();
			break;
		}
		case WM_SIZE:
		{
			if (myGraphics)
			{
				std::cout << "WM_SIZE CALLED with: " << LOWORD(aLParam) << ", " << HIWORD(aLParam) << ", " << (isFullscreen ? "true" : "false") << std::endl;

				myGraphics->SetRenderResolution(LOWORD(aLParam), HIWORD(aLParam));
				myWidth = LOWORD(aLParam);
				myHeight = HIWORD(aLParam);
				myResolutionEvent->myWidth = LOWORD(aLParam);
				myResolutionEvent->myHeight = HIWORD(aLParam);
				myResolutionEvent->myFullscreen = isFullscreen;
				ES::EventSystem::GetInstance().SendEvent(*myResolutionEvent);

			}
			break;
		}
		case WM_ACTIVATE:
		{
			// Confine/free cursor on window to foreground/background if cursor disabled
			if (!isCursorEnabled)
			{
				if (aWParam & WA_ACTIVE)
				{
					ConfineCursor();
					HideCursor();
				}
				else
				{
					FreeCursor();
					ShowCursor();
				}
			}
			break;
		}
		case WM_DROPFILES:
		{
#ifndef  KITTYENGINE_NO_EDITOR
			//myEditor->ReceiveDragDrop();
			//for every file, run ReceiveDragDrop on the an std::string of the file path


			std::string filePath;
			HDROP hDrop = (HDROP)aWParam;
			UINT fileCount = DragQueryFileA(hDrop, 0xFFFFFFFF, NULL, 0);
			for (UINT i = 0; i < fileCount; i++)
			{
				UINT filePathLength = DragQueryFileA(hDrop, i, NULL, 0);
				filePath.resize(filePathLength);
				DragQueryFileA(hDrop, i, filePath.data(), filePathLength + 1);

				myEditor->ReceiveDragDrop(filePath);
			}

			DragFinish(hDrop);
#endif // ! KITTYENGINE_NOEDITOR
			break;
		}
		case WM_SYSKEYDOWN:
		{
			if (aWParam == VK_RETURN)
			{
				if (aLParam & 0x60000000)
				{
					SetFullscreen(!GetFullscreen());
				}
			}
			break;
		}
		case WM_DEVICECHANGE:
		{
			myInputWrapper.UpdateControllerStatus();
			break;
		}
		}
		return DefWindowProc(aHWnd, aUMsg, aWParam, aLParam);
	}

	void Window::OnReceiveEvent(ES::Event& aEvent)
	{
		if (GUIResolutionEvent* resEvent = dynamic_cast<GUIResolutionEvent*>(&aEvent))
		{
			std::cout << "Window GUIResolutionEvent Received with: " << 
				resEvent->myWidth <<
				", " <<
				resEvent->myHeight <<
				", " <<
				(resEvent->myFullscreen ? "true" : "false") <<
				", Window isFullscreen = " <<
				(isFullscreen ? "true" : "false") <<
				std::endl;

#ifndef KITTYENGINE_SHIP
			SetRenderResolution(resEvent->myWidth, resEvent->myHeight);
			SetFullscreen(resEvent->myFullscreen);
#else
			//myGraphics->SetFullscreenStatus(resEvent->myFullscreen);
			myWidth = resEvent->myWidth;
			myHeight = resEvent->myHeight;

			if (resEvent->myFullscreen && !isFullscreen)
			{

				int graphicsW = myGraphics->GetRenderWidth();
				int graphicsH = myGraphics->GetRenderHeight();

				SetFullscreen(resEvent->myFullscreen);
				SetRenderResolution(graphicsW, graphicsH);

			}
			else
			{
				SetFullscreen(resEvent->myFullscreen, resEvent->myWidth, resEvent->myHeight);
				SetRenderResolution(resEvent->myWidth, resEvent->myHeight);
			}
#endif
		}

		if (P8::GamepadRumbleEvent* rumbleEvent = dynamic_cast<P8::GamepadRumbleEvent*>(&aEvent))
		{
			if (Gamepad* gamepad = myInputWrapper.GetGamepad(rumbleEvent->gamepadIndex))
			{
				gamepad->SetRumble(rumbleEvent->leftMotor, rumbleEvent->rightMotor, rumbleEvent->duration, rumbleEvent->rumbleType);
			}
		}
	}

	void Window::OnInit()
	{
		ES::EventSystem::GetInstance().Attach<GUIResolutionEvent>(this);
		ES::EventSystem::GetInstance().Attach<P8::GamepadRumbleEvent>(this);
	}

	void Window::OnDestroy()
	{
		ES::EventSystem::GetInstance().Detach<GUIResolutionEvent>(this);
		ES::EventSystem::GetInstance().Detach<P8::GamepadRumbleEvent>(this);
	}
}
