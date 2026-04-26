#include "stdafx.h"
#include "Gamepad.h"
#include <cassert>

#pragma comment(lib, "Xinput.lib")

namespace KE
{

	Gamepad::Gamepad()
	{
	}

	Gamepad::Gamepad(int aIndex)
	{
		myGamepadIndex = aIndex;

		myCurrentButtonStates.reset();
		myPreviousButtonStates.reset();
	}

	Gamepad::~Gamepad()
	{
		// Stop vibration
		Rumble(0.0f, 0.0f);
	}

	void Gamepad::SetRumble(float aLeftMotor, float aRightMotor, float aDuration, RumbleType aRumbleType)
	{
		myRumbleType = aRumbleType;
		myRumbleDuration = aDuration;
		myRumbleLeftMotor = aLeftMotor;
		myRumbleRightMotor = aRightMotor;
	}

	XINPUT_STATE Gamepad::GetState()
	{
		XINPUT_STATE GamepadState;
		ZeroMemory(&GamepadState, sizeof(XINPUT_STATE));

		XInputGetState(myGamepadIndex, &GamepadState);

		return GamepadState;
	}

	void Gamepad::Update()
	{
		myState = GetState();

		// Update button states
		UpdateButtonStates();

		myPreviousButtonStates = myCurrentButtonStates;
		myCurrentButtonStates = myLiveButtonStates;

		// Update rumble
		switch (myRumbleType)
		{
		case RumbleType::Stop:
		{
			Rumble(0.0f, 0.0f);
			myRumbleType = RumbleType::None;
			break;
		}
		case RumbleType::Timed:
		{
			myRumbleTimer += KE_GLOBAL::trueDeltaTime;

			if (myRumbleTimer >= myRumbleDuration)
			{
				myRumbleType = RumbleType::Stop;
				myRumbleTimer = 0.0f;
				Rumble(0.0f, 0.0f);
			}
			else
			{
				Rumble(myRumbleLeftMotor, myRumbleRightMotor);
			}
			break;
		}
		default:
			break;
		}
	}

	void Gamepad::UpdateButtonStates()
	{
		for (int i = 0; i < MAX_BUTTONS; i++)
		{
			myLiveButtonStates[i] = GetButtonState(i);
		}

		UpdateLeftStickX();
		UpdateLeftStickY();
		UpdateRightStickX();
		UpdateRightStickY();
		UpdateLeftTrigger();
		UpdateRightTrigger();
	}

	bool Gamepad::LStickInDeadzone()
	{
		// Obtain the X & Y axes of the stick
		short sX = myState.Gamepad.sThumbLX;
		short sY = myState.Gamepad.sThumbLY;

		// X axis is outside of deadzone
		if (sX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
			sX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			return false;

		// Y axis is outside of deadzone
		if (sY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE ||
			sY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
			return false;

		// One (or both axes) axis is inside of deadzone
		return true;
	}

	bool Gamepad::RStickInDeadzone()
	{
		// Obtain the X & Y axes of the stick
		short sX = myState.Gamepad.sThumbRX;
		short sY = myState.Gamepad.sThumbRY;

		// X axis is outside of deadzone
		if (sX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
			sX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			return false;

		// Y axis is outside of deadzone
		if (sY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE ||
			sY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
			return false;

		// One (or both axes) axis is inside of deadzone
		return true;
	}

	float Gamepad::UpdateLeftStickX()
	{
		myPrevThumbLX = myThumbLX;
		myThumbLX = myState.Gamepad.sThumbLX;

		// Return axis value, converted to a float
		return (static_cast<float>(myThumbLX) / 32768.0f);
	}

	float Gamepad::UpdateLeftStickY()
	{
		myPrevThumbLY = myThumbLY;
		myThumbLY = myState.Gamepad.sThumbLY;

		// Return axis value, converted to a float
		return (static_cast<float>(myThumbLY) / 32768.0f);
	}

	float Gamepad::UpdateRightStickX()
	{
		myPrevThumbRX = myThumbRX;
		myThumbRX = myState.Gamepad.sThumbRX;

		// Return axis value, converted to a float
		return (static_cast<float>(myThumbRX) / 32768.0f);
	}

	float Gamepad::UpdateRightStickY()
	{
		myPrevThumbRY = myThumbRY;
		myThumbRY = myState.Gamepad.sThumbRY;

		// Return axis value, converted to a float
		return (static_cast<float>(myThumbRY) / 32768.0f);
	}

	float Gamepad::UpdateLeftTrigger()
	{
		BYTE trigger = myState.Gamepad.bLeftTrigger;

		if (trigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
		{
			myPrevTriggerL = myTriggerL;
			myTriggerL = trigger;
			return myTriggerL / 255.0f;
		}

		return 0.0f;
	}

	float Gamepad::UpdateRightTrigger()
	{
		BYTE trigger = myState.Gamepad.bRightTrigger;

		if (trigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
		{
			myPrevTriggerR = myTriggerR;
			myTriggerR = trigger;
			return myTriggerR / 255.0f;
		}

		return 0.0f;
	}

	bool Gamepad::GetButtonState(int aButton)
	{
		if (myState.Gamepad.wButtons & XINPUT_BUTTONS[aButton])
		{
			return true;
		}

		return false;
	}

	bool Gamepad::IsButtonPressed(int aButton)
	{
		if (aButton < 0 || aButton >= MAX_BUTTONS)
		{
			return false;
		}

		return !myPreviousButtonStates[aButton] && myCurrentButtonStates[aButton];
	}

	bool Gamepad::IsButtonDown(int aButton)
	{
		if (aButton < 0 || aButton >= MAX_BUTTONS)
		{
			return false;
		}

		return myCurrentButtonStates[aButton];
	}

	bool Gamepad::IsButtonHeld(int aButton)
	{
		if (aButton < 0 || aButton >= MAX_BUTTONS)
		{
			return false;
		}

		return myPreviousButtonStates[aButton] && myCurrentButtonStates[aButton];
	}

	bool Gamepad::IsButtonReleased(int aButton)
	{
		if (aButton < 0 || aButton >= MAX_BUTTONS)
		{
			return false;
		}

		return myPreviousButtonStates[aButton] && !myCurrentButtonStates[aButton];
	}

	bool Gamepad::IsLeftStickToggled()
	{
		myLeftStickToggledUp = false;
		myLeftStickToggledDown = false;
		myLeftStickToggledLeft = false;
		myLeftStickToggledRight = false;

		float thumbLX = static_cast<float>(myThumbLX) / 32768.0f;
		float prevThumbLX = static_cast<float>(myPrevThumbLX) / 32768.0f;
		float thumbLY = static_cast<float>(myThumbLY) / 32768.0f;
		float prevThumbLY = static_cast<float>(myPrevThumbLY) / 32768.0f;

		// Up
		if (prevThumbLY < myThumbToggleLimit && thumbLY > myThumbToggleLimit)
		{
			myLeftStickToggledUp = true;
			//std::cout << "\nLeft stick toggled up";
			return true;
		}
		// Down
		else if (prevThumbLY > -myThumbToggleLimit && thumbLY < -myThumbToggleLimit)
		{
			myLeftStickToggledDown = true;
			//std::cout << "\nLeft stick toggled down";
			return true;
		}
		//Left
		else if (prevThumbLX > -myThumbToggleLimit && thumbLX < -myThumbToggleLimit)
		{
			myLeftStickToggledLeft = true;
			//std::cout << "\nLeft stick toggled left";
			return true;
		}
		// Right
		else if (prevThumbLX < myThumbToggleLimit && thumbLX > myThumbToggleLimit)
		{
			myLeftStickToggledRight = true;
			//std::cout << "\nLeft stick toggled right";
			return true;
		}

		return false;
	}

	void Gamepad::Rumble(float aLeftMotor, float aRightMotor)
	{
		// Vibration state
		XINPUT_VIBRATION VibrationState;

		// Zero memory
		ZeroMemory(&VibrationState, sizeof(XINPUT_VIBRATION));

		// Calculate vibration values
		int iLeftMotor = int(aLeftMotor * 65535.0f);
		int iRightMotor = int(aRightMotor * 65535.0f);

		// Set vibration values
		VibrationState.wLeftMotorSpeed = (WORD)iLeftMotor;
		VibrationState.wRightMotorSpeed = (WORD)iRightMotor;

		// Set the vibration state
		XInputSetState(myGamepadIndex, &VibrationState);
	}

	int Gamepad::GetIndex()
	{
		return myGamepadIndex;
	}

	bool Gamepad::IsConnected()
	{
		ZeroMemory(&myState, sizeof(XINPUT_STATE));

		DWORD result = XInputGetState(myGamepadIndex, &myState);

		if (result == ERROR_SUCCESS)
		{
			UpdateConnectionStatus(true);
			return true;
		}
		else
		{
			UpdateConnectionStatus(false);
			return false;
		}
	}

	void Gamepad::UpdateConnectionStatus(bool aConnected)
	{
		if (aConnected && myConnectionStatus != ConnectionStatus::Connected)
		{
			std::string text = "Controller " + std::to_string(myGamepadIndex) + " connected";
			KE_LOG(text.c_str());
			myConnectionStatus = ConnectionStatus::Connected;
		}
		else if (!aConnected && myConnectionStatus != ConnectionStatus::Disconnected)
		{
			std::string text = "Controller " + std::to_string(myGamepadIndex) + " disconnected";
			KE_LOG(text.c_str());
			myConnectionStatus = ConnectionStatus::Disconnected;
		}
	}

	XButtonIDs XButtons;

	XButtonIDs::XButtonIDs()
	{
		// These values are used to index the XINPUT_Buttons array,
		// accessing the matching XINPUT button value

		A = 0;
		B = 1;
		X = 2;
		Y = 3;

		DPadUp = 4;
		DPadDown = 5;
		DPadLeft = 6;
		DPadRight = 7;

		LShoulder = 8;
		RShoulder = 9;

		LThumbstick = 10;
		RThumbstick = 11;

		Start = 12;
		Back = 13;

		LTrigger = 14;
		RTrigger = 15;
		LStick = 16;
		RStick = 17;
	}
}