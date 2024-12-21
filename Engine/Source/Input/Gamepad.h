#pragma once
#include "xinput.h"
#include <bitset>
#include <string>

namespace KE
{
	static const WORD XINPUT_BUTTONS[] = {
	  XINPUT_GAMEPAD_A,
	  XINPUT_GAMEPAD_B,
	  XINPUT_GAMEPAD_X,
	  XINPUT_GAMEPAD_Y,
	  XINPUT_GAMEPAD_DPAD_UP,
	  XINPUT_GAMEPAD_DPAD_DOWN,
	  XINPUT_GAMEPAD_DPAD_LEFT,
	  XINPUT_GAMEPAD_DPAD_RIGHT,
	  XINPUT_GAMEPAD_LEFT_SHOULDER,
	  XINPUT_GAMEPAD_RIGHT_SHOULDER,
	  XINPUT_GAMEPAD_LEFT_THUMB,
	  XINPUT_GAMEPAD_RIGHT_THUMB,
	  XINPUT_GAMEPAD_START,
	  XINPUT_GAMEPAD_BACK
	};

	enum class eButtons
	{
		A = 0,
		B = 1,
		X = 2,
		Y = 3,
		DPadUp = 4,
		DPadDown = 5,
		DPadLeft = 6,
		DPadRight = 7,
		LShoulder = 8,
		RShoulder = 9,
		LThumbstick = 10,
		RThumbstick = 11,
		Start = 12,
		Back = 13,
		LTrigger = 14,
		RTrigger = 15,
		LStick = 16,
		RStick = 17
	};

	inline std::string GetButtonName(eButtons aButton)
	{
		switch (aButton)
		{
		case eButtons::A:
			return "A";
		case eButtons::B:
			return "B";
		case eButtons::X:
			return "X";
		case eButtons::Y:
			return "Y";
		case eButtons::DPadUp:
			return "DPadUp";
		case eButtons::DPadDown:
			return "DPadDown";
		case eButtons::DPadLeft:
			return "DPadLeft";
		case eButtons::DPadRight:
			return "DPadRight";
		case eButtons::LShoulder:
			return "LShoulder";
		case eButtons::RShoulder:
			return "RShoulder";
		case eButtons::LThumbstick:
			return "LThumbstick";
		case eButtons::RThumbstick:
			return "RThumbstick";
		case eButtons::Start:
			return "Start";
		case eButtons::Back:
			return "Back";
		case eButtons::LTrigger:
			return "LTrigger";
		case eButtons::RTrigger:
			return "RTrigger";
		case eButtons::LStick:
			return "LStick";
		case eButtons::RStick:
			return "RStick";
		default:
			return "Unknown";
		}
	}

	struct XButtonIDs
	{
		XButtonIDs();

		unsigned int A, B, X, Y;
		unsigned int DPadUp, DPadDown, DPadLeft, DPadRight;
		unsigned int LShoulder, RShoulder;
		unsigned int LThumbstick, RThumbstick;
		unsigned int Start;
		unsigned int Back;
		unsigned int LTrigger, RTrigger;
		unsigned int LStick, RStick;
	};

	enum class ConnectionStatus
	{
		None,
		Connected,
		Disconnected
	};

	enum class RumbleType
	{
		None,
		Timed,
		Stop
	};

	class Gamepad
	{
	public:
		Gamepad();
		Gamepad(int aIndex);
		~Gamepad();

		void Update();
		void UpdateButtonStates();

		bool LStickInDeadzone();
		bool RStickInDeadzone();

		float UpdateLeftStickX();
		float UpdateLeftStickY();
		float UpdateRightStickX();
		float UpdateRightStickY();
		float GetLeftStickX() { return static_cast<float>(myThumbLX) / 32768.0f; }
		float GetLeftStickY() { return static_cast<float>(myThumbLY) / 32768.0f; };
		float GetRightStickX() { return static_cast<float>(myThumbRX) / 32768.0f; };
		float GetRightStickY() { return static_cast<float>(myThumbRY) / 32768.0f; };

		float UpdateLeftTrigger();
		float UpdateRightTrigger();
		inline float GetLeftTrigger() { return myTriggerL / 255.0f; }
		inline float GetRightTrigger() { return myTriggerR / 255.0f; }

		bool GetButtonState(int aButton);
		bool IsButtonPressed(int aButton);
		bool IsButtonDown(int aButton);
		bool IsButtonHeld(int aButton);
		bool IsButtonReleased(int aButton);

		bool IsLeftStickToggled();
		inline bool IsLeftStickToggledUp() { return myLeftStickToggledUp; }
		inline bool IsLeftStickToggledDown() { return myLeftStickToggledDown; }
		inline bool IsLeftStickToggledLeft() { return myLeftStickToggledLeft; }
		inline bool IsLeftStickToggledRight() { return myLeftStickToggledRight; }

		void SetRumble(float aLeftMotor, float aRightMotor, float aDuration, RumbleType aRumbleType = RumbleType::None);

		XINPUT_STATE GetState();
		int GetIndex();
		bool IsConnected();
		bool GetIsConnected() { return myConnectionStatus == ConnectionStatus::Connected; }

	private:
		// Vibrate the gamepad (0.0f cancel, 1,0f max)
		void Rumble(float aLeftMotor, float aRightMotor);
		void UpdateConnectionStatus(bool aConnected);

	private:
		XINPUT_STATE myState;
		int myGamepadIndex = -1;
		ConnectionStatus myConnectionStatus = ConnectionStatus::Disconnected;

		static const int MAX_BUTTONS = 14;
		std::bitset<MAX_BUTTONS> myPreviousButtonStates;
		std::bitset<MAX_BUTTONS> myCurrentButtonStates;
		std::bitset<MAX_BUTTONS> myLiveButtonStates;
		short myThumbLX = 0;
		short myThumbLY = 0;
		short myThumbRX = 0;
		short myThumbRY = 0;
		BYTE myTriggerL = 0;
		BYTE myTriggerR = 0;

		short myPrevThumbLX = 0;
		short myPrevThumbLY = 0;
		short myPrevThumbRX = 0;
		short myPrevThumbRY = 0;
		BYTE myPrevTriggerL = 0;
		BYTE myPrevTriggerR = 0;

		float myThumbToggleLimit = 0.5f;
		bool myLeftStickToggledUp = false;
		bool myLeftStickToggledDown = false;
		bool myLeftStickToggledLeft = false;
		bool myLeftStickToggledRight = false;

		RumbleType myRumbleType = RumbleType::None;
		float myRumbleTimer = 0.0f;
		float myRumbleDuration = 0.0f;
		float myRumbleLeftMotor = 0.0f;
		float myRumbleRightMotor = 0.0f;
	};

	extern XButtonIDs XButtons;
}