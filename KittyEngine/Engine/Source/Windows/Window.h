#pragma once
#include "KittyEngineWin.h"
//#include "Engine/Source/Input/Keyboard.h"
//#include "Engine/Source/Input/Mouse.h"
#include "Engine/Source/Graphics/Graphics.h"
#include "Engine/Source/Math/Vector2.h"

#include <optional>
#include <memory>
#include <vector>

#include "Engine/Source/Input/InputWrapper.h"
#include "Engine/Source/Utility/Event.h"

#ifndef KE_NOEDITOR
namespace KE_EDITOR
{
	class Editor;
}
#endif

namespace KE
{
	struct ResolutionEvent : ES::Event
	{
		ResolutionEvent() = default;
		~ResolutionEvent() override = default;

		int myWidth = 0;
		int myHeight = 0;
		bool myFullscreen = false;
	};

	class Window : ES::IObserver
	{
	private:
		// Singleton manages registration/cleanup of Window class
		class WindowClass
		{
		public:
			static const wchar_t* GetName();
			static HINSTANCE GetInstance();

		private:
			WindowClass();
			~WindowClass();
			WindowClass(const WindowClass&) = delete;
			WindowClass& operator=(const WindowClass&) = delete;
			static constexpr const wchar_t* WND_CLASS_NAME = L"KittyEngine";
			static WindowClass myWndClass;
			HINSTANCE myHInst;
		};

	public:
		Window(int aWidth, int aHeight, const wchar_t* aName);
		~Window();
		Window(const Window&) = delete;
		Window& operator=(const WindowClass&) = delete;
		void EnableCursor();
		void DisableCursor();
		bool IsCursorEnabled() const;
		static std::optional<int> ProcessMessages();
		Graphics& GetGraphics() const;
		InputWrapper& GetInputWrapper();

		void AddToWindowName(const wchar_t* aExtention);

		inline Vector2i GetWindowSize() { return Vector2i(myWidth, myHeight); };
		inline Vector2i GetWindowPosition() const { return myWindowPosition; }

		void SetWindowDims(int aX, int aY, int aWidth, int aHeight, bool aFullscreen);
		void SetRenderResolution(int aWidth, int aHeight);
		void SetFullscreen(bool aState, int aWidth = -1, int aHeight = -1);
		inline bool GetFullscreen() const { return isFullscreen; }

	private:
		void ConfineCursor() const;
		void FreeCursor();
		void HideCursor();
		void ShowCursor();
		void EnableImGuiMouse();
		void DisableImGuiMouse();
		static LRESULT WINAPI HandleMsgSetup(HWND aHWnd, UINT aUMsg, WPARAM aWParam, LPARAM aLParam);
		static LRESULT WINAPI HandleMsgThunk(HWND aHWnd, UINT aUMsg, WPARAM aWParam, LPARAM aLParam);
		LRESULT HandleMsg(HWND aHWnd, UINT aUMsg, WPARAM aWParam, LPARAM aLParam);

	public:
		// IObserver
		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		void OnDestroy() override;

		InputWrapper myInputWrapper;
		HWND myHWnd;

#ifndef KE_NOEDITOR
		KE_EDITOR::Editor* myEditor;
#endif

	private:
		bool isCursorEnabled = true;
		bool isFullscreen = false;
		int myWidth;
		int myHeight;

		int myWindowedWidth;
		int myWindowedHeight;

		const std::wstring myStartName;

		Vector2i myWindowPosition;

		std::unique_ptr<Graphics> myGraphics;

		ResolutionEvent* myResolutionEvent = nullptr;
	};
}
