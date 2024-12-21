#include "stdafx.h"
#include "InputWrapper.h"

#include "Input.h"
#include "Utility/EventSystem.h"


namespace KE
{
	InputWrapper::InputWrapper()
	{
		// Keyboard
		myKeysUp.push_back(VK_UP);
		myKeysUp.push_back('W');
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::Up, myKeysUp));

		myKeysDown.push_back(VK_DOWN);
		myKeysDown.push_back('S');
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::Down, myKeysDown));

		myKeysLeft.push_back(VK_LEFT);
		myKeysLeft.push_back('A');
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::Left, myKeysLeft));

		myKeysRight.push_back(VK_RIGHT);
		myKeysRight.push_back('D');
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::Right, myKeysRight));

		myKeysAction1.push_back(VK_NUMPAD1);
		myKeysAction1.push_back('J');
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::Action1, myKeysAction1));

		myKeysAction2.push_back(VK_NUMPAD2);
		myKeysAction2.push_back('K');
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::Action2, myKeysAction2));

		myKeysAction3.push_back(VK_NUMPAD3);
		myKeysAction3.push_back('L');
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::Action3, myKeysAction3));

		//myKeysAction4.push_back('L');
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::Action4, myKeysAction4));

		myKeysEsc.push_back(VK_ESCAPE);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::Esc, myKeysEsc));

		myKeysEnter.push_back(VK_RETURN);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::Enter, myKeysEnter));

		myKeysTab.push_back(VK_TAB);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::Tab, myKeysTab));

		//myKeysShift.push_back(VK_NUMPAD1);
		myKeysShift.push_back(VK_SHIFT);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::Shift, myKeysShift));

		myKeysSpace.push_back(VK_NUMPAD0);
		myKeysSpace.push_back(VK_SPACE);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::Space, myKeysSpace));

		myKeysF1.push_back(VK_F1);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::F1, myKeysF1));

		myKeysF2.push_back(VK_F2);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::F2, myKeysF2));

		myKeysF3.push_back(VK_F3);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::F3, myKeysF3));

		myKeysF4.push_back(VK_F4);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::F4, myKeysF4));

		myKeysF5.push_back(VK_F5);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::F5, myKeysF5));

		myKeysF6.push_back(VK_F6);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::F6, myKeysF6));

		// Mouse
		myKeysLeftClick.push_back(VK_LBUTTON);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::LeftClick, myKeysLeftClick));

		myKeysRightClick.push_back(VK_RBUTTON);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::RightClick, myKeysRightClick));

		myKeysMiddleClick.push_back(VK_MBUTTON);
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::MiddleClick, myKeysMiddleClick));

		// Gamepads
		UpdateControllerStatus();

		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxA, XButtons.A));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxB, XButtons.B));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxX, XButtons.X));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxY, XButtons.Y));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxDPadUp, XButtons.DPadUp));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxDPadDown, XButtons.DPadDown));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxDPadLeft, XButtons.DPadLeft));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxDPadRight, XButtons.DPadRight));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxLShoulder, XButtons.LShoulder));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxRShoulder, XButtons.RShoulder));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxLThumbstick, XButtons.LThumbstick));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxRThumbstick, XButtons.RThumbstick));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxStart, XButtons.Start));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxBack, XButtons.Back));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxLeftTrigger, XButtons.LTrigger));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxRightTrigger, XButtons.RTrigger));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxLeftStick, XButtons.LStick));
		myInputEvents.push_back(ES::EventSystem::GetInstance().CreateNewEvent<InputEvent>(eInputType::XboxRightStick, XButtons.RStick));
	}

	void InputWrapper::Update()
	{
		if (!isInputEnabled)
		{
			return;
		}

		myInputHandler.Update();

		//if (myUpdateControllerTimer > myUpdateControllerInterval)
		//{
		//	UpdateControllerStatus();
		//	myUpdateControllerTimer = 0.0f;
		//	//std::cout << "\nChecking controller status";
		//}
		//else
		//{
		//	myUpdateControllerTimer += KE_GLOBAL::deltaTime;
		//}

		for (int i = 0; i < 4; ++i)
		{
			if (myGamepads[i].GetIsConnected())
			{
				myGamepads[i].Update();
			}
		}

		bool pressedKey = false;

		for (InputEvent* action : myInputEvents)
		{
			action->interactions.clear();

			Key key;
			bool shouldQueue = false;

			// Keyboard and mouse
			if (action->myKeyBindings.size() > 0)
			{
				for (int i = 0; i < action->myKeyBindings.size(); i++)
				{
					std::vector<Key> tempKeys = { action->myKeyBindings[i] };

					if (PressedKeyThisFrame(tempKeys, key))
					{
						action->myInteractionType = eInteractionType::Pressed;
						action->myMousePosition = myInputHandler.GetMousePosition();

						action->interactions.push_back({ eInteractionType::Pressed, key, GetKeyDesc(key) });
						action->interactions.back().myInputType = action->myInputType;
						action->interactions.back().myControllerID = GetKeyboardID(key);

						pressedKey = true;
						shouldQueue = true;
					}
					else if (ReleasedKeyThisFrame(tempKeys, key))
					{
						action->myInteractionType = eInteractionType::Released;
						action->myMousePosition = myInputHandler.GetMousePosition();

						action->interactions.push_back({ eInteractionType::Released, key, GetKeyDesc(key) });
						action->interactions.back().myInputType = action->myInputType;
						action->interactions.back().myControllerID = GetKeyboardID(key);

						shouldQueue = true;
					}
					else if (HeldKeyThisFrame(tempKeys, key))
					{
						action->myInteractionType = eInteractionType::Held;
						action->myMousePosition = myInputHandler.GetMousePosition();

						action->interactions.push_back({ eInteractionType::Held, key,GetKeyDesc(key) });
						action->interactions.back().myInputType = action->myInputType;
						action->interactions.back().myControllerID = GetKeyboardID(key);

						shouldQueue = true;
					}
				}
			}
			// Gamepad
			else
			{
				for (int i = 0; i < 4; ++i)
				{
					if (myGamepads[i].GetIsConnected())
					{
						if (myGamepads[i].IsButtonPressed(action->myXboxButton))
						{
							action->myInteractionType = eInteractionType::Pressed;
							action->myXboxButton = action->myXboxButton;
							action->interactions.push_back({ eInteractionType::Pressed, action->myXboxButton, GetButtonName((eButtons)action->myXboxButton) });
							action->interactions.back().myInputType = action->myInputType;
							action->interactions.back().myControllerID = myGamepads[i].GetIndex();

							shouldQueue = true;
						}
						else if (myGamepads[i].IsButtonReleased(action->myXboxButton))
						{
							action->myInteractionType = eInteractionType::Released;
							action->myXboxButton = action->myXboxButton;
							action->interactions.push_back({ eInteractionType::Released, action->myXboxButton, GetButtonName((eButtons)action->myXboxButton) });
							action->interactions.back().myInputType = action->myInputType;
							action->interactions.back().myControllerID = myGamepads[i].GetIndex();

							shouldQueue = true;
						}
						else if (myGamepads[i].IsButtonHeld(action->myXboxButton))
						{
							action->myInteractionType = eInteractionType::Held;
							action->myXboxButton = action->myXboxButton;
							action->interactions.push_back({ eInteractionType::Held, action->myXboxButton, GetButtonName((eButtons)action->myXboxButton) });
							action->interactions.back().myInputType = action->myInputType;
							action->interactions.back().myControllerID = myGamepads[i].GetIndex();

							shouldQueue = true;
						}
						else if (action->myInputType == eInputType::XboxLeftStick)
						{
							if (myGamepads[i].IsLeftStickToggled())
							{
								action->myInteractionType = eInteractionType::Pressed;
								action->myXboxButton = XButtons.LStick;
								action->interactions.push_back({ eInteractionType::Pressed, XButtons.LStick, GetButtonName((eButtons)action->myXboxButton) });
								action->interactions.back().myStick = { myGamepads[i].GetLeftStickX(), myGamepads[i].GetLeftStickY() };
								action->interactions.back().myControllerID = myGamepads[i].GetIndex();
								action->interactions.back().myInputType = action->myInputType;

								if (myGamepads[i].IsLeftStickToggledUp())
								{
									action->interactions.back().myInputType = eInputType::XboxLeftTriggerToggledUp;
								}
								else if (myGamepads[i].IsLeftStickToggledDown())
								{
									action->interactions.back().myInputType = eInputType::XboxLeftTriggerToggledDown;
								}
								else if (myGamepads[i].IsLeftStickToggledLeft())
								{
									action->interactions.back().myInputType = eInputType::XboxLeftTriggerToggledLeft;
								}
								else if (myGamepads[i].IsLeftStickToggledRight())
								{
									action->interactions.back().myInputType = eInputType::XboxLeftTriggerToggledRight;
								}

								shouldQueue = true;
							}
							if (!myGamepads[i].LStickInDeadzone())
							{
								action->myInteractionType = eInteractionType::Held;
								action->myXboxButton = XButtons.LStick;
								action->interactions.push_back({ eInteractionType::Held, XButtons.LStick, GetButtonName((eButtons)action->myXboxButton) });
								action->interactions.back().myStick = { myGamepads[i].GetLeftStickX(), myGamepads[i].GetLeftStickY() };
								action->interactions.back().myControllerID = myGamepads[i].GetIndex();
								action->interactions.back().myInputType = action->myInputType;

								shouldQueue = true;
							}
						}
						else if (action->myInputType == eInputType::XboxRightStick)
						{

							if (!myGamepads[i].RStickInDeadzone())
							{
								action->myInteractionType = eInteractionType::Held;
								action->myXboxButton = XButtons.RStick;
								action->interactions.push_back({ eInteractionType::Held, XButtons.RStick, GetButtonName((eButtons)action->myXboxButton) });
								action->interactions.back().myStick = { myGamepads[i].GetRightStickX(), myGamepads[i].GetRightStickY() };
								action->interactions.back().myControllerID = myGamepads[i].GetIndex();
								action->interactions.back().myInputType = action->myInputType;

								shouldQueue = true;
							}
						}
						else if (action->myInputType == eInputType::XboxLeftTrigger)
						{
							if (myGamepads[i].GetLeftTrigger() > 0.0f)
							{
								action->myInteractionType = eInteractionType::Held;
								action->myXboxButton = XButtons.LTrigger;
								action->interactions.push_back({ eInteractionType::Held, XButtons.LTrigger, GetButtonName((eButtons)action->myXboxButton) });
								action->interactions.back().myTrigger = myGamepads[i].GetLeftTrigger();
								action->interactions.back().myControllerID = myGamepads[i].GetIndex();
								action->interactions.back().myInputType = action->myInputType;

								shouldQueue = true;
							}
						}
						else if (action->myInputType == eInputType::XboxRightTrigger)
						{
							if (myGamepads[i].GetRightTrigger() > 0.0f)
							{
								action->myInteractionType = eInteractionType::Held;
								action->myXboxButton = XButtons.RTrigger;
								action->interactions.push_back({ eInteractionType::Held, XButtons.RTrigger, GetButtonName((eButtons)action->myXboxButton) });
								action->interactions.back().myTrigger = myGamepads[i].GetRightTrigger();
								action->interactions.back().myControllerID = myGamepads[i].GetIndex();
								action->interactions.back().myInputType = action->myInputType;

								shouldQueue = true;
							}
						}
					}
				}
			}

			if (shouldQueue)
			{
				ES::EventSystem::GetInstance().QueueEvent(action);
			}
		}

		if (pressedKey)
		{
			InputEvent anyEvent(eInputType::Any, {}, eInteractionType::Pressed);
			ES::EventSystem::GetInstance().SendEvent(anyEvent);
		}

		if (myInputHandler.IsMouseMoving())
		{
			const POINT mousePosition = myInputHandler.GetMousePosition();
			MouseMoveEvent mouseMoveEvent;
			mouseMoveEvent.myMousePosition = mousePosition;
			ES::EventSystem::GetInstance().SendEvent(mouseMoveEvent);
		}
	}

	bool InputWrapper::UpdateEvents(const UINT aMsg, const WPARAM aWParam, const LPARAM aLParam)
	{
		return myInputHandler.UpdateEvents(aMsg, aWParam, aLParam);
	}

	Vector2f InputWrapper::GetMovement() const
	{
		Vector2f movement = { 0, 0 };

		for (const Key key : myKeysLeft)
		{
			if (myInputHandler.IsKeyDown(key))
			{
				movement.x = -1;
			}
		}

		for (const Key key : myKeysRight)
		{
			if (myInputHandler.IsKeyDown(key))
			{
				movement.x = 1;
			}
		}

		for (const Key key : myKeysUp)
		{
			if (myInputHandler.IsKeyDown(key))
			{
				movement.y = 1;
			}
		}

		for (const Key key : myKeysDown)
		{
			if (myInputHandler.IsKeyDown(key))
			{
				movement.y = -1;
			}
		}

		return movement;
	}

	bool InputWrapper::PressedKeyThisFrame(const std::vector<Key>& aMouseAndKeyboardKeys, Key& aOutKey) const
	{
		bool pressedKey = false;

		for (const Key key : aMouseAndKeyboardKeys)
		{
			if (myInputHandler.IsKeyPressed(key))
			{
				pressedKey = true;
				aOutKey = key;
			}
		}

		return pressedKey;
	}

	bool InputWrapper::ReleasedKeyThisFrame(const std::vector<Key>& aMouseAndKeyboardKeys, Key& aOutKey) const
	{
		bool releasedKey = false;

		for (const Key key : aMouseAndKeyboardKeys)
		{
			if (myInputHandler.IsKeyReleased(key))
			{
				releasedKey = true;
				aOutKey = key;
			}
		}

		return releasedKey;
	}

	bool InputWrapper::HeldKeyThisFrame(const std::vector<Key>& aMouseAndKeyboardKeys, Key& aOutKey) const
	{
		bool heldKey = false;

		for (const Key key : aMouseAndKeyboardKeys)
		{
			if (myInputHandler.IsKeyHeld(key))
			{
				heldKey = true;
				aOutKey = key;
			}
		}

		return heldKey;
	}

	bool InputWrapper::GetIsEnabled() const
	{
		return isInputEnabled;
	}

	void InputWrapper::SetEnabled(const bool aValue)
	{
		isInputEnabled = aValue;
	}

	bool InputWrapper::IsKeyPressed(const int aKeyCode) const
	{
		return myInputHandler.IsKeyPressed(aKeyCode);
	}

	bool InputWrapper::IsKeyDown(const int aKeyCode) const
	{
		return myInputHandler.IsKeyDown(aKeyCode);
	}

	bool InputWrapper::IsKeyHeld(const int aKeyCode) const
	{
		return myInputHandler.IsKeyHeld(aKeyCode);
	}

	bool InputWrapper::IsKeyReleased(const int aKeyCode) const
	{
		return myInputHandler.IsKeyReleased(aKeyCode);
	}

	bool InputWrapper::IsAnyKeyDown() const
	{
		return myInputHandler.IsAnyKeyDown();
	}

	std::optional<InputHandler::RawDelta> InputWrapper::ReadRawDelta()
	{
		return myInputHandler.ReadRawDelta();
	}

	void InputWrapper::EnableRaw()
	{
		myInputHandler.EnableRaw();
	}

	void InputWrapper::DisableRaw()
	{
		myInputHandler.DisableRaw();
	}

	bool InputWrapper::IsLMBPressed() const
	{
		return myInputHandler.IsKeyPressed(VK_LBUTTON);
	}

	bool InputWrapper::IsLMBHeld() const
	{
		return myInputHandler.IsKeyHeld(VK_LBUTTON);
	}

	bool InputWrapper::IsLMBReleased() const
	{
		return myInputHandler.IsKeyReleased(VK_LBUTTON);
	}

	bool InputWrapper::IsRMBPressed() const
	{
		return myInputHandler.IsKeyPressed(VK_RBUTTON);
	}

	bool InputWrapper::IsRMBHeld() const
	{
		return myInputHandler.IsKeyHeld(VK_RBUTTON);
	}

	bool InputWrapper::IsRMBReleased() const
	{
		return myInputHandler.IsKeyReleased(VK_RBUTTON);
	}

	bool InputWrapper::IsMMBPressed() const
	{
		return myInputHandler.IsKeyPressed(VK_MBUTTON);
	}

	bool InputWrapper::IsMMBHeld() const
	{
		return myInputHandler.IsKeyHeld(VK_MBUTTON);
	}

	bool InputWrapper::IsMMBReleased() const
	{
		return myInputHandler.IsKeyReleased(VK_MBUTTON);
	}

	Vector2f InputWrapper::GetMousePosition() const
	{
		POINT point = myInputHandler.GetMousePosition();
		return { (float)point.x, (float)point.y };
	}

	short InputWrapper::GetScrollDelta()
	{
		return myInputHandler.GetScrollDelta();
	}

	void InputWrapper::UpdateControllerStatus()
	{
		for (int i = 0; i < 4; ++i)
		{
			myGamepads[i].IsConnected();
		}
	}

	int InputWrapper::GetKeyboardID(Key& aKey) const
	{
		if (aKey == 'W' ||
			aKey == 'A' ||
			aKey == 'S' ||
			aKey == 'D' ||
			aKey == VK_SPACE ||
			aKey == 'J' ||
			aKey == 'K' ||
			aKey == 'L')
		{
			return 4;
		}
		else if (aKey == VK_UP ||
			aKey == VK_DOWN ||
			aKey == VK_LEFT ||
			aKey == VK_RIGHT ||
			aKey == VK_NUMPAD0 ||
			aKey == VK_NUMPAD1 ||
			aKey == VK_NUMPAD2 ||
			aKey == VK_NUMPAD3)
		{
			return 5;
		}

		return -1;
	}
	Gamepad* InputWrapper::GetGamepad(int aIndex)
	{
		if (aIndex < 0 || aIndex > 3)
		{
			return nullptr;
		}
		return &myGamepads[aIndex];
	}
}
