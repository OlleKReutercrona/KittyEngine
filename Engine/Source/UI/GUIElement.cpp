#include "stdafx.h"
#include "GUIElement.h"
#include "GUIScene.h"
#include "Engine/Source/Audio/GlobalAudio.h"

namespace KE
{
	GUIElement::GUIElement()
	{
		mySpriteBatch.myRenderLayer = eRenderLayers::UI;
	}

	GUIElement::~GUIElement()
	{
		OnDestroy();
	}

	void GUIElement::InitObserver()
	{
		shouldReceiveEvents = true;
		OnInit();
	}

	GUITooltip& GUIElement::AddTooltip()
	{
		return myTooltip;
	}

	void GUIElement::Update(float aDeltaTime)
	{
		//if (mySprite == nullptr)
		//{
		//	return;
		//}
		if (isTimerActive)
		{
			myCooldownTimer += aDeltaTime;

			SetFillFactor(myCooldownTimer / myTotalCooldown);

			if (myCooldownTimer >= myTotalCooldown)
			{
				isTimerActive = false;
				myCooldownTimer = 0.0f;
				SetFillFactor(1.0f);
			}
		}
	}

	void GUIElement::SetFillFactor(const float aFactor)
	{
		myFillFactor = aFactor;
		mySpriteBatch.myInstances[0].SetFillFactor(myFillFactor, myProgressionDirection);
	}

	void GUIElement::Activate()
	{
		if (!myParentScene->IsActive()) { return; }

		myState = eGUIElementState::Pressed;
		myColour = { 0, 1, 0, 1 };

		//myEvent.myGUIElementType = myType;
		//myEvent.myEventName = myName;
		ES::EventSystem::GetInstance().SendEvent(myEvent);
		KE::GlobalAudio::PlaySFX(sound::SFX::MenuSelect);

		if (!isTimerActive)
		{
			isTimerActive = true;
		}
	}

	void GUIElement::Click()
	{
		if (!myParentScene->IsActive()) { return; }
		//myEvent.myGUIElementType = myType;
		//myEvent.myEventName = myName;
		ES::EventSystem::GetInstance().SendEvent(myEvent);

		KE::GlobalAudio::PlaySFX(sound::SFX::MenuSelect);
	}

	void GUIElement::Highlight()
	{
		if (myState == eGUIElementState::Pressed)
		{
			return;
		}
		myState = eGUIElementState::Hovered;

		if (mySecondaryTexture != myDisplayTexture)
		{
			mySpriteBatch.myData.myTexture = mySecondaryTexture;
		}

		myColour = { 1, 1, 0, 1 };

		myTooltip.Show();

		if (isButton) {
			KE::GlobalAudio::PlaySFX(sound::SFX::MenuToggle);
		}
	}

	void GUIElement::Reset()
	{
		myState = eGUIElementState::Idle;

		if (mySecondaryTexture != myDisplayTexture)
		{
			mySpriteBatch.myData.myTexture = myDisplayTexture;
		}

		myColour = { 1, 0, 0, 1 };

		myTooltip.Hide();
	}

	void GUIElement::UpdateSpriteScaleAndPosition(const Vector2f& aResolution, const bool aUpdateResolution)
	{
		float posX = 0.0f;
		float posY = 0.0f;

		if (aUpdateResolution)
		{
			myBox.myOffset = myBox.myOffsetResolutionFactor * aResolution;
			myBox.myWidth = myBox.mySizeResolutionFactor.x * aResolution.x;
			myBox.myHeight = myBox.mySizeResolutionFactor.y * aResolution.y;
		}

		myBox.myOffsetResolutionFactor = { myBox.myOffset.x / aResolution.x, myBox.myOffset.y / aResolution.y };
		myBox.mySizeResolutionFactor = { myBox.myWidth / aResolution.x, myBox.myHeight / aResolution.y };

		// Convert to switch case
		switch (myAlignType)
		{
		case eAlignType::BottomLeft:
		{
			posX = myBox.myOffset.x;
			posY = aResolution.y - myBox.myOffset.y;
			break;
		}
		case eAlignType::BottomCenter:
		{
			posX = aResolution.x / 2.0f + myBox.myOffset.x;
			posY = aResolution.y - myBox.myOffset.y;
			break;
		}
		case eAlignType::BottomRight:
		{
			posX = aResolution.x - myBox.myOffset.x;
			posY = aResolution.y - myBox.myOffset.y;
			break;
		}
		case eAlignType::Center:
		{
			posX = aResolution.x / 2.0f + myBox.myOffset.x;
			posY = aResolution.y / 2.0f - myBox.myOffset.y;
			break;
		}
		case eAlignType::CenterLeft:
		{
			posX = myBox.myOffset.x;
			posY = aResolution.y / 2.0f - myBox.myOffset.y;
			break;
		}
		case eAlignType::CenterRight:
		{
			posX = aResolution.x - myBox.myOffset.x;
			posY = aResolution.y / 2.0f - myBox.myOffset.y;
			break;
		}
		case eAlignType::TopLeft:
		{
			posX = myBox.myOffset.x;
			posY = myBox.myOffset.y;
			break;
		}
		case eAlignType::TopCenter:
		{
			posX = aResolution.x / 2.0f + myBox.myOffset.x;
			posY = myBox.myOffset.y;
			break;
		}
		case eAlignType::TopRight:
		{
			posX = aResolution.x - myBox.myOffset.x;
			posY = myBox.myOffset.y;
			break;
		}
		case eAlignType::Fullscreen:
		{
			posX = 0.0f;
			posY = aResolution.y - myBox.myOffset.y;
			myBox.myWidth = aResolution.x;
			myBox.myHeight = aResolution.y;
			break;
		}
		case eAlignType::Count:
			break;
		default:;
		}

		myBox.myScreenPosition = { posX, posY };

		/*if (mySprite != nullptr)
		{
		}*/
		if (!mySpriteBatch.myInstances.empty())
		{
			mySpriteBatch.myInstances[0].myAttributes.myTransform.SetPosition({ posX, posY - myBox.myHeight, 0.0f });
			mySpriteBatch.myInstances[0].myAttributes.myTransform.SetScale({ myBox.myWidth, myBox.myHeight, 1.0f });
		}

		//mySprite->myAttributes.myTransform.SetScale({ myBox.myWidth, myBox.myHeight, 1.0f });

		//Vector2f pos = myBox.myScreenPosition;
		//mySprite->myAttributes.myTransform.SetPosition({ pos.x, pos.y - myBox.myHeight, 0.0f });
	}

	void GUIElement::OnReceiveEvent(ES::Event& aEvent)
	{
		if (!shouldReceiveEvents)
		{
			return;
		}
	}

	void GUIElement::OnInit()
	{
		
	}

	void GUIElement::OnDestroy()
	{

	}
}
