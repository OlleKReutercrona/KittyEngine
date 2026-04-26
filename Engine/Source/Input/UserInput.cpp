#include "stdafx.h"
#include "UserInput.h"

#include "InputEvents.h"
#include "InputWrapper.h"

namespace KE
{
	UserInput::UserInput()
	{
		/*	std::cout << "\nUserInput created";*/
	}

	UserInput::~UserInput()
	{
		OnDestroy();
	}

	void UserInput::Init()
	{
		OnInit();
	}

	void UserInput::Update() { }

	void UserInput::SetIsMouseOnGUI(const bool aIsMouseOnGUI)
	{
		isMouseOverGUI = aIsMouseOnGUI;
	}

	void UserInput::OnReceiveEvent(ES::Event& aEvent)
	{
		const InputEvent* inputActionEvent = dynamic_cast<InputEvent*>(&aEvent);

		if (inputActionEvent != nullptr)
		{
			if (isMouseOverGUI)
			{
				GUIEvent uiEvent;
				uiEvent.myInputType = inputActionEvent->myInputType;
				uiEvent.myInteractionType = inputActionEvent->myInteractionType;
				uiEvent.myKeyBindings = inputActionEvent->myKeyBindings;
				uiEvent.myMousePosition = inputActionEvent->myMousePosition;
				//uiEvent.myTriggerKeyName = inputActionEvent->myTriggerKeyName;
				uiEvent.interactions = inputActionEvent->interactions;


				ES::EventSystem::GetInstance().SendEvent(uiEvent);
			}
			else
			{
				PlayerEvent playerEvent;
				playerEvent.myInputType = inputActionEvent->myInputType;
				playerEvent.myInteractionType = inputActionEvent->myInteractionType;
				playerEvent.myKeyBindings = inputActionEvent->myKeyBindings;
				playerEvent.myMousePosition = inputActionEvent->myMousePosition;
				//playerEvent.myTriggerKeyName = inputActionEvent->myTriggerKeyName;
				playerEvent.interactions = inputActionEvent->interactions;

				//std::cout << "\nInputEvent received";
				//std::cout << "\nNumber of interactions: " << inputActionEvent->interactions.size();
				//for (const Interaction& interaction : inputActionEvent->interactions)
				//{
				//	std::cout << "\nTrigger name: " << interaction.myTriggerKeyName;
				//}

				if (playerEvent.myInputType == eInputType::LeftClick ||
					playerEvent.myInputType == eInputType::RightClick)
				{
					// TODO Raycast on this position:
					playerEvent.myMousePosition;
				}

				ES::EventSystem::GetInstance().SendEvent(playerEvent);
			}
		}

		const MouseMoveEvent* mouseMoveEvent = dynamic_cast<MouseMoveEvent*>(&aEvent);

		if (mouseMoveEvent != nullptr)
		{
			if (isMouseOverGUI)
			{
				GUIEvent uiEvent;
				uiEvent.myInputType = eInputType::None;
				uiEvent.myInteractionType = eInteractionType::Hovered;
				//uiEvent.myKeyBindings;
				uiEvent.myMousePosition = mouseMoveEvent->myMousePosition;
				//uiEvent.myTriggerKeyName;

				ES::EventSystem::GetInstance().SendEvent(uiEvent);
			}
			else
			{
				// TODO Add hover highlight above items and enemies
				// TODO Raycast on this position:
				mouseMoveEvent->myMousePosition;
				// TODO Player needs to rotate when moving the mouse as well
			}
		}
	}

	void UserInput::OnInit()
	{
		ES::EventSystem::GetInstance().Attach<InputEvent>(this);
		ES::EventSystem::GetInstance().Attach<MouseMoveEvent>(this);
	}

	void UserInput::OnDestroy()
	{
		ES::EventSystem::GetInstance().Detach<InputEvent>(this);
		ES::EventSystem::GetInstance().Detach<MouseMoveEvent>(this);
	}
}
