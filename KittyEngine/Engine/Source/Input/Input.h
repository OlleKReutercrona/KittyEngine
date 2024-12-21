#pragma once
#include <unordered_map>
#include <string>
#include <Windows.h>

#include "Engine/Source/Utility/Event.h"

namespace KE
{
	using Key = unsigned int;

	enum
	{
		KEY_MOV_FW = 'W',
		KEY_MOV_BW = 'S',
		KEY_MOV_LF = 'A',
		KEY_MOV_RT = 'D',
		KEY_MOV_UP = 'E',
		KEY_MOV_DN = 'Q',
		KEY_ABT_1 = '1',
		KEY_ABT_2 = '2',
		KEY_ABT_3 = '3',
		KEY_ABT_4 = '4',
		KEY_MOU_LB = 0x01,
		KEY_MOU_RB = 0x02,
	};

	inline std::string GetKeyDesc(const unsigned int aVirtualKey)
	{
		static const std::unordered_map<unsigned int, std::string> KEY_MAP = {
			{1, "Left Mouse Button"},
			{2, "Right Mouse Button"},
			{3, "Middle Mouse Button"},
			{4, "X1 Mouse Button"},
			{5, "X2 Mouse Button"},
			{8, "Backspace"},
			{9, "Tab"},
			{13, "Enter"},
			{16, "Shift"},
			{17, "Ctrl"},
			{18, "Alt"},
			{19, "Pause/Break"},
			{20, "Caps Lock"},
			{27, "Esc"},
			{32, "Space"},
			{33, "Page Up"},
			{34, "Page Down"},
			{37, "Left Arrow Key"},
			{38, "Up Arrow Key"},
			{39, "Right Arrow Key"},
			{40, "Down Arrow Key"},
			// ... Add more keys as needed ...
			{65, "A"},
			{66, "B"},
			{67, "C"},
			{68, "D"},
			{69, "E"},
			{70, "F"},
			{71, "G"},
			{72, "H"},
			{73, "I"},
			{74, "J"},
			{75, "K"},
			{76, "L"},
			{77, "M"},
			{78, "N"},
			{79, "O"},
			{80, "P"},
			{81, "Q"},
			{82, "R"},
			{83, "S"},
			{84, "T"},
			{85, "U"},
			{86, "V"},
			{87, "W"},
			{88, "X"},
			{89, "Y"},
			{90, "Z"},
			{91, "Left Windows Key"},
			{92, "Right Windows Key"},
			{93, "Applications Key"},
			{96, "Numpad 0"},
			{97, "Numpad 1"},
			{98, "Numpad 2"},
			{99, "Numpad 3"},
			{100, "Numpad 4"},
			{101, "Numpad 5"},
			{102, "Numpad 6"},
			{103, "Numpad 7"},
			{104, "Numpad 8"},
			{105, "Numpad 9"},
			{106, "Multiply"},
			{107, "Add"},
			{109, "Subtract"},
			{110, "Decimal Point"},
			{111, "Divide"},
			// ... Add more keys as needed ...
			{112, "F1"},
			{113, "F2"},
			{114, "F3"},
			{115, "F4"},
			{116, "F5"},
			{117, "F6"},
			{118, "F7"},
			{119, "F8"},
			{120, "F9"},
			{121, "F10"},
			{122, "F11"},
			{123, "F12"},
			// ... Add more function keys as needed ...
			{144, "Num Lock"},
			{145, "Scroll Lock"},
			{186, ";"},
			{187, "="},
			{188, ","},
			{189, "-"},
			{190, "."},
			{191, "/"},
			{192, "`"},
			// ... Add more special characters as needed ...
			{219, "["},
			{220, "\\"},
			{221, "]"},
			{222, "'"},
			{255, "Delete"} // Virtual key code for Delete key
		};

		const auto it = KEY_MAP.find(aVirtualKey);
		if (it != KEY_MAP.end())
		{
			return it->second;
		}
		else
		{
			return "Unknown Key";
		}
	}

	enum class eInputType
	{
		None,
		Up,
		Down,
		Left,
		Right,
		LeftClick,
		RightClick,
		MiddleClick,
		Action1,
		Action2,
		Action3,
		Action4,
		Esc,
		Enter,
		Tab,
		Shift,
		Space,
		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		StartWave,
		Any,
		XboxA,
		XboxB,
		XboxX,
		XboxY,
		XboxDPadUp,
		XboxDPadDown,
		XboxDPadLeft,
		XboxDPadRight,
		XboxLShoulder,
		XboxRShoulder,
		XboxLThumbstick,
		XboxRThumbstick,
		XboxStart,
		XboxBack,
		XboxLeftTrigger,
		XboxRightTrigger,
		XboxLeftStick,
		XboxRightStick,
		XboxLeftTriggerToggledUp,
		XboxLeftTriggerToggledDown,
		XboxLeftTriggerToggledLeft,
		XboxLeftTriggerToggledRight,
	};

	enum class eInteractionType
	{
		None,
		Pressed,
		Released,
		Held,
		Hovered
	};

	struct Interaction
	{
		Interaction() = default;
		
		Interaction(const eInteractionType aInteractionType, const Key aKey, const std::string& aTriggerName)
		{
			myInteractionType = aInteractionType;
			myKey = aKey;
			myTriggerKeyName = aTriggerName;
		}

		Interaction(const eInteractionType aInteractionType, const int aXboxButton, const std::string& aTriggerName)
		{
			myInteractionType = aInteractionType;
			myXboxButton = aXboxButton;
			myTriggerKeyName = aTriggerName;
		}

		std::string myTriggerKeyName = "Not set";
		eInteractionType myInteractionType = eInteractionType::None;
		eInputType myInputType = eInputType::None;
		Key myKey;
		Key myXboxButton;
		int myControllerID = -1;
		Vector2f myStick = { 0.0f, 0.0f };
		float myTrigger = 0.0f;
	};

	struct InputEvent : ES::Event
	{
		InputEvent() = default;

		InputEvent(const eInputType aInputType, const std::vector<Key>& aMouseAndKeyboardKeys, const eInteractionType aInterationType = eInteractionType::None)
		{
			myInputType = aInputType;
			myInteractionType = aInterationType;
			myKeyBindings = aMouseAndKeyboardKeys;
		}

		InputEvent(const eInputType aInputType, const Key aXboxButton, const eInteractionType aInterationType = eInteractionType::None)
		{
			myInputType = aInputType;
			myInteractionType = aInterationType;
			myXboxButton = aXboxButton;
		}

		std::vector<Interaction> interactions;

		eInputType myInputType = eInputType::None;
		eInteractionType myInteractionType = eInteractionType::None;
		std::vector<Key> myKeyBindings = {};
		POINT myMousePosition = {0, 0};
		//std::string myTriggerKeyName = "Not set";
		Key myXboxButton;
		//int myControllerID = -1;
		//Vector2f myStick = { 0.0f, 0.0f };
		//float myTrigger = 0.0f;
	};

	struct MouseMoveEvent : ES::Event
	{
		MouseMoveEvent() = default;

		MouseMoveEvent(const POINT& aMousePosition)
		{
			myMousePosition = aMousePosition;
		}

		POINT myMousePosition = {0, 0};
	};
}
