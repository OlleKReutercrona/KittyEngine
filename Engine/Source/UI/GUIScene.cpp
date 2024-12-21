#include "stdafx.h"
#include "GUIScene.h"
#include "Engine/Source/UI/GUIElement.h"

namespace KE
{
	GUIElement& GUIScene::CreateGUIElement(
		const Vector2f& aSize,
		const Vector2f& aOffset,
		const Vector2f& aScreenResolution,
		const bool aIsClickable,
		const eAlignType aAlignType,
		const eProgressionDirection aDirection)
	{
		float aPosX;
		float aPosY;

		GUIElement& element = myGUIElements.emplace_back();

		// Setup from where the element should be aligned
		element.myAlignType = aAlignType;

		if (element.myAlignType == eAlignType::BottomCenter)
		{
			aPosX = aScreenResolution.x / 2.0f + aOffset.x;
			aPosY = aScreenResolution.y + aOffset.y;
		}
		else if (element.myAlignType == eAlignType::Center)
		{
			aPosX = aScreenResolution.x / 2.0f + aOffset.x;
			aPosY = aScreenResolution.y / 2.0f + aOffset.y;
		}
		else if (element.myAlignType == eAlignType::TopCenter)
		{
			aPosX = aScreenResolution.x / 2.0f + aOffset.x;
			aPosY = aOffset.y;
		}
		else if (element.myAlignType == eAlignType::BottomLeft)
		{
			aPosX = aOffset.x;
			aPosY = aScreenResolution.y - aOffset.y;
		}

		element.myBox.myOffset.x = aOffset.x;
		element.myBox.myOffset.y = aOffset.y;
		element.myBox.myScreenPosition = { aPosX, aPosY };
		element.myBox.myWidth = aSize.x;
		element.myBox.myHeight = aSize.y;
		element.myBox.myOffsetResolutionFactor = { aOffset.x / aScreenResolution.x, aOffset.y / aScreenResolution.y };
		element.myBox.mySizeResolutionFactor = { aSize.x / aScreenResolution.x, aSize.y / aScreenResolution.y };
		element.myColour = { 1, 0, 0, 1 };
		//element.myType = aGUIElementType;
		//element.myEvent.myGUIElementType = aGUIElementType;
		element.isButton = aIsClickable;
		element.myProgressionDirection = aDirection;
		return element;
	}

	GUIElement& GUIScene::CreateGUIElement(
		const std::string& aName, 
		const std::string& aTexturePath, 
		const std::string& aEventName,
		//const eGUIElementType aGUIElementType, 
		const eAlignType aAlignType, 
		const eProgressionDirection aDirection, 
		const Vector2f& aOffsetResolutionFactor, 
		const Vector2f& aSizeResolutionFactor, 
		const Vector2i& aScreenResolution,
		const bool aIsButton)
	{
		float aPosX;
		float aPosY;

		GUIElement& element = myGUIElements.emplace_back();

		element.myParentScene = this;

		element.myName = aName;
		element.myTexturePath = aTexturePath;
		element.myEvent.myEventName = aEventName;

		if (element.shouldHideAtStart)
		{
			element.shouldDraw = false;
		}

		element.myBox.myOffset.x = aOffsetResolutionFactor.x * (float)aScreenResolution.x;
		element.myBox.myOffset.y = aOffsetResolutionFactor.y * (float)aScreenResolution.y;
		element.myBox.myWidth = aSizeResolutionFactor.x * (float)aScreenResolution.x;
		element.myBox.myHeight = aSizeResolutionFactor.y * (float)aScreenResolution.y;

		// Setup from where the element should be aligned
		element.myAlignType = aAlignType;

		if (element.myAlignType == eAlignType::BottomCenter)
		{
			aPosX = aScreenResolution.x / 2.0f + element.myBox.myOffset.x;
			aPosY = aScreenResolution.y + element.myBox.myOffset.y;
		}
		else if (element.myAlignType == eAlignType::Center)
		{
			aPosX = aScreenResolution.x / 2.0f + element.myBox.myOffset.x;
			aPosY = aScreenResolution.y / 2.0f + element.myBox.myOffset.y;
		}
		else if (element.myAlignType == eAlignType::TopCenter)
		{
			aPosX = aScreenResolution.x / 2.0f + element.myBox.myOffset.x;
			aPosY = element.myBox.myOffset.y;
		}
		else if (element.myAlignType == eAlignType::BottomLeft)
		{
			aPosX = element.myBox.myOffset.x;
			aPosY = aScreenResolution.y - element.myBox.myOffset.y;
		}

		element.myBox.myScreenPosition = { aPosX, aPosY };
		element.myBox.myOffsetResolutionFactor = aOffsetResolutionFactor;
		element.myBox.mySizeResolutionFactor = aSizeResolutionFactor;
		element.myColour = { 1, 0, 0, 1 };
		//element.myType = aGUIElementType;
		//element.myEvent.myGUIElementType = aGUIElementType;
		// TODO Fix this
		element.isButton = aIsButton;
		element.myProgressionDirection = aDirection;
		return element;
	}

	GUIElement& GUIScene::DuplicateGUIElement(const GUIElement& aElement, const Vector2f& aResolution)
	{
		std::string name = aElement.myName + "_Copy";
		return CreateGUIElement(
			{ aElement.myBox.myWidth, aElement.myBox.myHeight },
			aElement.myBox.myOffset,
			aResolution,
			//aElement.myType,
			aElement.isButton,
			aElement.myAlignType,
			aElement.myProgressionDirection);
	}

	GUITooltip& GUIScene::CreateTooltip(GUIElement& aElement,
	                                    const Vector2f& aSize,
	                                    const Vector2f& aOffsetFromParent)
	{
		float aPosX = aElement.myBox.myScreenPosition.x + aOffsetFromParent.x;
		float aPosY = aElement.myBox.myScreenPosition.y + aOffsetFromParent.y;

		GUITooltip& tooltip = aElement.AddTooltip();
		tooltip.myBox.myOffset.x = aOffsetFromParent.x;
		tooltip.myBox.myOffset.y = aOffsetFromParent.y;
		tooltip.myBox.myScreenPosition = { aPosX, aPosY };
		tooltip.myBox.myWidth = aSize.x;
		tooltip.myBox.myHeight = aSize.y;
		tooltip.myLinePoints[0] = { aPosX, aPosY };
		tooltip.myLinePoints[1] = { aPosX + aSize.x, aPosY };
		tooltip.myLinePoints[2] = { aPosX + aSize.x, aPosY - aSize.y };
		tooltip.myLinePoints[3] = { aPosX, aPosY - aSize.y };
		tooltip.myColour = { 1, 0, 0, 1 };
		tooltip.isInitialized = true;
		return tooltip;
	}

	void GUIScene::MoveElementUp(const size_t aIndex)
	{
		if (aIndex < myGUIElements.size() - 1)
		{
			std::swap(myGUIElements[aIndex], myGUIElements[aIndex + 1]);
		}
	}

	void GUIScene::MoveElementDown(const size_t aIndex)
	{
		if (aIndex > 0)
		{
			std::swap(myGUIElements[aIndex], myGUIElements[aIndex - 1]);
		}
	}

	void GUIScene::MoveElement(const size_t aIndex, const size_t aNewIndex)
	{
		if (aIndex < myGUIElements.size() && aNewIndex < myGUIElements.size() + 1)
		{
			if (aIndex >= aNewIndex)
			{
				std::rotate(myGUIElements.rend() - aIndex - 1, myGUIElements.rend() - aIndex, myGUIElements.rend() - aNewIndex);
			}
			else if (aIndex < aNewIndex)
			{
				std::rotate(myGUIElements.begin() + aIndex, myGUIElements.begin() + aIndex + 1, myGUIElements.begin() + aNewIndex);
			}
		}
	}

	void GUIScene::AddElement(GUIElement& aElement)
	{
		myGUIElements.push_back(aElement);
	}

	void GUIScene::RemoveElement(const size_t aIndex)
	{
		if (aIndex < myGUIElements.size())
		{
			myGUIElements.erase(myGUIElements.begin() + aIndex);
		}
	}

	//void GUIScene::Init(const eGUISceneType aType, const size_t aNumberOfElementsToReserve)
	//{
	//	myGUIElements.reserve(aNumberOfElementsToReserve);
	//	myType = aType;
	//}

	void GUIScene::Init(const std::string& aSceneName, const size_t aNumberOfElementsToReserve)
	{
		myGUIElements.reserve(aNumberOfElementsToReserve);
		myName = aSceneName;
	}

	void GUIScene::Init(const std::string& aSceneName)
	{
		myName = aSceneName;
		myGUIElements.reserve(64);
	}

	void GUIScene::UpdateGUISize(const int aWidth, const int aHeight)
	{
		const Vector2f resolution = { (float)aWidth, (float)aHeight };

		for (GUIElement& element : myGUIElements)
		{
			// Update element position
			element.UpdateSpriteScaleAndPosition(resolution, true);

			// Update tooltip position
			{
				GUITooltip& tooltip = element.myTooltip;
				float posX = element.myBox.myScreenPosition.x + tooltip.myBox.myOffset.x;
				float posY = element.myBox.myScreenPosition.y + tooltip.myBox.myOffset.y;

				tooltip.myLinePoints[0] = { posX, posY };
				tooltip.myLinePoints[1] = { posX + tooltip.myBox.myWidth, posY };
				tooltip.myLinePoints[2] = { posX + tooltip.myBox.myWidth, posY - tooltip.myBox.myHeight };
				tooltip.myLinePoints[3] = { posX, posY - tooltip.myBox.myHeight };

				tooltip.myBox.myScreenPosition = { posX, posY };

				if (tooltip.mySprite != nullptr)
				{
					tooltip.mySprite->myAttributes.myTransform.SetPosition(
						{ posX, posY - tooltip.myBox.myHeight, 0.0f }
					);
				}
			}
		}
	}
}
