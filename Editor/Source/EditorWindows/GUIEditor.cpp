#include "stdafx.h"
#ifndef KITTYENGINE_NO_EDITOR
#include "GUIEditor.h"

#include <regex>

#include "Editor/Source/Editor.h"
#include <External/Include/imgui/imgui.h>

#include <Engine/Source/UI/GUIHandler.h>

#include "Input/InputEvents.h"

#include "Engine/Source/Graphics/Graphics.h"


namespace KE_EDITOR
{

	GUIEditor::~GUIEditor()
	{
		GUIEditor::OnDestroy();
	}

	void GUIEditor::Init()
	{
		myGUIHandler = KE_GLOBAL::blackboard.Get<KE::GUIHandler>("GUIHandler");

		//LoadGUIFromFile();

		OnInit();
	}

	void GUIEditor::Update()
	{
	}

	bool GUIEditor::DisplayDebugOptions()
	{
		ImGui::SeparatorText("Debug");
		ImGui::Checkbox("Draw anchor", &shouldDrawAnchor);

		ImGui::SeparatorText("Grid");
		ImGui::Checkbox("Draw grid", &myGUIHandler->drawGUIGrid);
		ImGui::Checkbox("On: Square grid | Off: Rectangle grid", &myGUIHandler->gridSquareCells);

		if (!myGUIHandler->gridSquareCells)
		{
			ImGui::SliderInt("Grid size X", &myGUIHandler->myGridSizeX, 2, 60);
			ImGui::SliderInt("Grid size Y", &myGUIHandler->myGridSizeY, 2, 60);
		}
		else
		{
			ImGui::SliderInt("Grid size", &myGUIHandler->myGridSizeX, 2, 60);
		}

		if (!shouldDrawAnchor)
		{
			return true;
		}
		return false;
	}

	void GUIEditor::DisplaySelectedGuiElement()
	{
		// Edit GUIElement
		// Setup temp variables for monitoring changes
		// TODO Can crash if the selected element is deleted

		if (mySelectedGUIElement == nullptr)
		{
			return;
		}

		float fill = mySelectedGUIElement->GetFillFactor();
		float width = mySelectedGUIElement->myBox.myWidth;
		float height = mySelectedGUIElement->myBox.myHeight;
		float offsetX = mySelectedGUIElement->myBox.myOffset.x;
		float offsetY = mySelectedGUIElement->myBox.myOffset.y;
		KE::eAlignType alignType = mySelectedGUIElement->myAlignType;
		KE::eProgressionDirection progressionDirection = mySelectedGUIElement->myProgressionDirection;

		if (ImGui::Button("Rename"))
		{
			ImGui::OpenPopup("ChangeNamePopup");
			lockSelect = true;
		}

		// Check if the context menu is open
		if (ImGui::BeginPopup("ChangeNamePopup"))
		{
			// Display an input text field to allow the user to change the name
			static char newNameBuffer[256];
			strcpy_s(newNameBuffer, mySelectedGUIElement->myName.c_str());

			if (ImGui::InputText("New Name", newNameBuffer, IM_ARRAYSIZE(newNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				// Update the element's name when Enter is pressed
				std::string newName = newNameBuffer;
				RenameNotUniqueElementName(mySelectedGUIScene, newName);
				mySelectedGUIElement->myName = newName;
				ImGui::CloseCurrentPopup();
			}

			lockSelect = false;
			ImGui::EndPopup();
		}

		ImGui::SameLine();
		if (ImGui::Button("Delete"))
		{
			mySelectedGUIScene->RemoveElement(mySelectedElementIndex);
			mySelectedGUIElement = nullptr;
			mySelectedElementIndex = -1;
			return;
		}

		ImGui::SameLine();
		if (ImGui::Button("Duplicate"))
		{
			Vector2f resolution = { (float)myGUIHandler->myGraphics->GetRenderSize().x, (float)myGUIHandler->myGraphics->GetRenderSize().y };
			KE::GUIElement* duplicateElement = &mySelectedGUIScene->DuplicateGUIElement(*mySelectedGUIElement, resolution);

			std::string elementName = mySelectedGUIElement->myName + "_Copy";
			RenameNotUniqueElementName(mySelectedGUIScene, elementName);
			duplicateElement->myName = elementName;

			myGUIHandler->AssignSpriteToGUIElement(
				*duplicateElement,
				myGUIHandler->myGraphics,
				{ mySelectedGUIElement->myBox.myWidth, mySelectedGUIElement->myBox.myHeight },
				mySelectedGUIElement->mySpriteBatch.myData.myTexture->myMetadata.myFilePath);


			duplicateElement->UpdateSpriteScaleAndPosition({ resolution.x, resolution.y }, false);

			if (mySelectedGUIElement->hasText)
			{
				duplicateElement->hasText = true;
				myGUIHandler->SetGUIText(duplicateElement, mySelectedGUIElement->myText);
			}

			mySelectedGUIElement = duplicateElement;
			mySelectedElementIndex = static_cast<int>(mySelectedGUIScene->GetGUIElements().size() - 1);
		}

		ImGui::SeparatorText("Edit");
		ImGui::DragFloat("Width", &mySelectedGUIElement->myBox.myWidth);
		ImGui::DragFloat("Height", &mySelectedGUIElement->myBox.myHeight);
		ImGui::DragFloat2("Offset", &mySelectedGUIElement->myBox.myOffset.x);
		ImGuiHandler::DisplayTexture(&mySelectedGUIElement->myDisplayTexture, { 64, 64 });
		ImGuiHandler::DisplayTexture(&mySelectedGUIElement->mySecondaryTexture, { 64, 64 });

		mySelectedGUIElement->Reset();
		
		ImGui::Checkbox("Hide at start", &mySelectedGUIElement->shouldHideAtStart);
		ImGui::Checkbox("Show", &mySelectedGUIElement->shouldDraw);

		ImGui::Text("Event Name: %s", mySelectedGUIElement->myEvent.myEventName.c_str());

		ImGui::SameLine();

		if (ImGui::Button("Clear"))
		{
			myGUIHandler->UnregisterAssignedEvent(mySelectedGUIElement);
		}

		if (ImGui::InputTextWithHint("##eventName", "Event Name", eventBuffer, 64, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			//mySelectedGUIElement->myEvent.myEventName = eventBuffer;

			if (mySelectedGUIElement->myEvent.myEventName != "")
			{
				myGUIHandler->UnregisterAssignedEvent(mySelectedGUIElement);
			}

			myGUIHandler->RegisterAssignedEvent(eventBuffer, mySelectedGUIElement);
			memset(eventBuffer, 0, 64);
		}


		std::vector<std::string> events;
		for (const auto& pair : myGUIHandler->myEventMap)
		{
			events.push_back(pair.first);
		}

		static int currentItem = 0;  // This will store the index of the currently selected item

		// Create a dropdown menu
		if (ImGui::BeginCombo("Select Event", events[currentItem].c_str()))
		{
			for (int i = 0; i < events.size(); i++)
			{
				bool isSelected = (currentItem == i);
				if (ImGui::Selectable(events[i].c_str(), isSelected))
				{
					currentItem = i;  // Update the current item index when an item is selected
					myGUIHandler->RegisterAssignedEvent(events[i], mySelectedGUIElement);
				}

				// Set the initial focus when opening the combo (scrolling to the current item)
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}




		//ImGuiHandler::DisplayEnumSlider("Type", &mySelectedGUIElement->myType, KE::eGUIElementType::Count);
		ImGuiHandler::DisplayEnumSlider("AlignType", &mySelectedGUIElement->myAlignType, KE::eAlignType::Count);
		ImGuiHandler::DisplayEnumSlider("ProgressionDirection", &mySelectedGUIElement->myProgressionDirection, KE::eProgressionDirection::Count);
		ImGui::Checkbox("Button", &mySelectedGUIElement->isButton);

		ImGui::DragFloat("Fill Factor", &fill, 0.005f, 0.0f, 1.0f);

		if (fill != mySelectedGUIElement->GetFillFactor() ||
			progressionDirection != mySelectedGUIElement->myProgressionDirection)
		{
			mySelectedGUIElement->SetFillFactor(fill);
		}

		if (width != mySelectedGUIElement->myBox.myWidth ||
			height != mySelectedGUIElement->myBox.myHeight ||
			offsetX != mySelectedGUIElement->myBox.myOffset.x ||
			offsetY != mySelectedGUIElement->myBox.myOffset.y ||
			alignType != mySelectedGUIElement->myAlignType)
		{
			const Vector2i resolution = myGUIHandler->myGraphics->GetRenderSize();
			mySelectedGUIElement->UpdateSpriteScaleAndPosition({ (float)resolution.x ,(float)resolution.y }, false);

			myGUIHandler->SetGUIText(mySelectedGUIElement, mySelectedGUIElement->myText);
			//myGUIHandler->UpdateGUIText(mySelectedGUIElement);
		}

		// Draw GUIElement anchor point
		Vector2f resolution = { (float)myGUIHandler->myGraphics->GetRenderSize().x, (float)myGUIHandler->myGraphics->GetRenderSize().y };
		Vector2f position = { 0.f, 0.f };

		switch (mySelectedGUIElement->myAlignType)
		{
		case KE::eAlignType::BottomLeft:
			position = { 0.0f, resolution.y };
			break;
		case KE::eAlignType::BottomCenter:
			position = { resolution.x / 2.0f, resolution.y };
			break;
		case KE::eAlignType::BottomRight:
			position = { resolution.x, resolution.y };
			break;
		case KE::eAlignType::Center:
			position = { resolution.x / 2.0f, resolution.y / 2.0f };
			break;
		case KE::eAlignType::CenterLeft:
			position = { 0.0f, resolution.y / 2.0f };
			break;
		case KE::eAlignType::CenterRight:
			position = { resolution.x, resolution.y / 2.0f };
			break;
		case KE::eAlignType::TopLeft:
			position = { 0.0f, 0.0f };
			break;
		case KE::eAlignType::TopCenter:
			position = { resolution.x / 2.0f, 0.0f };
			break;
		case KE::eAlignType::TopRight:
			position = { resolution.x, 0.0f };
			break;
		case KE::eAlignType::Fullscreen:
			position = { 0.0f, 0.0f };
			break;
		case KE::eAlignType::Count:
			break;
		default:;
		}

		ImGui::Checkbox("Has text", &mySelectedGUIElement->hasText);

		if (mySelectedGUIElement->hasText)
		{
			std::string text = mySelectedGUIElement->myText;

			std::string displayText = mySelectedGUIElement->myText.length() > 0 ? mySelectedGUIElement->myText : "Text";
			if (ImGui::InputTextWithHint("##newText", displayText.c_str(), textBuffer, 64, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				text = textBuffer;
				mySelectedGUIElement->myText = text;
				memset(textBuffer, 0, 64);
			}

			ImGui::ColorEdit4("Text Colour", &mySelectedGUIElement->myTextColour.x);

			mySelectedGUIElement->mySpriteBatch.myTextStyling.text.horizontalAlign = KE::TextAlign::Center;
			ImGuiHandler::EditTextStyling(mySelectedGUIElement->mySpriteBatch.myTextStyling);

			myGUIHandler->SetGUIText(mySelectedGUIElement, text);
		}

		KE::DebugRenderer* debugRenderer = KE_GLOBAL::blackboard.Get<KE::DebugRenderer>("debugRenderer");

		// Anchor point
		{
			const Vector2f left = { position.x - 10.0f, position.y };
			const Vector2f leftUp = { position.x - 5.0f, position.y + 5.0f };
			const Vector2f leftDown = { position.x - 5.0f, position.y - 5.0f };
			const Vector2f right = { position.x + 10.0f, position.y };
			const Vector2f rightUp = { position.x + 5.0f, position.y + 5.0f };
			const Vector2f rightDown = { position.x + 5.0f, position.y - 5.0f };
			const Vector2f up = { position.x, position.y + 10.0f };
			const Vector2f down = { position.x, position.y - 10.0f };

			debugRenderer->RenderScreenSpaceLine(left, right, Vector4f(1, 0, 0, 0.5f));
			debugRenderer->RenderScreenSpaceLine(up, down, Vector4f(1, 0, 0, 0.5f));
			debugRenderer->RenderScreenSpaceLine(leftUp, rightDown, Vector4f(1, 0, 0, 0.5f));
			debugRenderer->RenderScreenSpaceLine(leftDown, rightUp, Vector4f(1, 0, 0, 0.5f));
			debugRenderer->RenderScreenSpaceLine(position, mySelectedGUIElement->myBox.myScreenPosition, Vector4f(0, 1, 0, 0.5f));
		}

		// Draw selected GUIElement highlight box
		{
			constexpr float offset = 5.0f;

			const Vector2f leftUp = { mySelectedGUIElement->myBox.myScreenPosition.x - offset, mySelectedGUIElement->myBox.myScreenPosition.y - mySelectedGUIElement->myBox.myHeight - offset };
			const Vector2f rightUp = { mySelectedGUIElement->myBox.myScreenPosition.x + mySelectedGUIElement->myBox.myWidth + offset, mySelectedGUIElement->myBox.myScreenPosition.y - mySelectedGUIElement->myBox.myHeight - offset };
			const Vector2f rightDown = { mySelectedGUIElement->myBox.myScreenPosition.x + mySelectedGUIElement->myBox.myWidth + offset, mySelectedGUIElement->myBox.myScreenPosition.y + offset };
			const Vector2f leftDown = { mySelectedGUIElement->myBox.myScreenPosition.x - offset, mySelectedGUIElement->myBox.myScreenPosition.y + offset };
			debugRenderer->RenderScreenSpaceLine(leftUp, rightUp, Vector4f(1, 1, 1, 1.0f));
			debugRenderer->RenderScreenSpaceLine(rightUp, rightDown, Vector4f(1, 1, 1, 1.0f));
			debugRenderer->RenderScreenSpaceLine(rightDown, leftDown, Vector4f(1, 1, 1, 1.0f));
			debugRenderer->RenderScreenSpaceLine(leftDown, leftUp, Vector4f(1, 1, 1, 1.0f));
		}
	}

	void GUIEditor::RenameNotUniqueElementName(KE::GUIScene* aGuiScene, std::string& aOutElementName)
	{
		// Check if name is unique
		int numberOfSameNames = 0;
		for (auto& element : aGuiScene->GetGUIElements())
		{
			std::string existingNameWithoutNumber = std::regex_replace(element.myName, std::regex("_[0-9]"), "");
			std::string newNameWithoutNumber = std::regex_replace(aOutElementName, std::regex("_[0-9]"), "");
			if (existingNameWithoutNumber == newNameWithoutNumber)
			{
				numberOfSameNames++;
			}
		}
		// If not unique, append number to name
		if (numberOfSameNames > 0)
		{
			aOutElementName += "_" + std::to_string(numberOfSameNames);
		}
	}

	void GUIEditor::RenameNotUniqueSceneName(KE::GUIHandler* aGuiHandler, std::string& aOutElementName)
	{
		// Check if name is unique
		int numberOfSameNames = 0;
		for (auto& scene : aGuiHandler->myGUISceneMap)
		{
			std::string existingNameWithoutNumber = std::regex_replace(scene.first, std::regex("_[0-9]"), "");
			std::string newNameWithoutNumber = std::regex_replace(aOutElementName, std::regex("_[0-9]"), "");
			if (existingNameWithoutNumber == newNameWithoutNumber)
			{
				numberOfSameNames++;
			}
		}
		// If not unique, append number to name
		if (numberOfSameNames > 0)
		{
			aOutElementName += "_" + std::to_string(numberOfSameNames);
		}
	}

	bool GUIEditor::DisplayGuiSceneElements(KE::GUIScene* aGuiScene)
	{
		if (ImGui::Button("Rename"))
		{
			lockSelect = true;
			ImGui::OpenPopup("ChangeNamePopup");
		}

		// Check if the context menu is open
		if (ImGui::BeginPopup("ChangeNamePopup"))
		{
			// Display an input text field to allow the user to change the name
			static char newNameBuffer[256];
			strcpy_s(newNameBuffer, mySelectedGUIScene->myName.c_str());

			if (ImGui::InputText("New Name", newNameBuffer, IM_ARRAYSIZE(newNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				// Update the element's name when Enter is pressed
				std::string newName = newNameBuffer;
				auto sceneToRename = myGUIHandler->myGUISceneMap.extract(mySelectedGUIScene->myName);
				RenameNotUniqueSceneName(myGUIHandler, newName);
				sceneToRename.key() = newName;
				myGUIHandler->myGUISceneMap.insert(std::move(sceneToRename));
				mySelectedGUIScene->myName = newName;
				ImGui::CloseCurrentPopup();
			}

			lockSelect = false;
			ImGui::EndPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete"))
		{
			if (mySelectedGUIScene != nullptr)
			{
				myGUIHandler->PopGUIScene(mySelectedGUIScene->GetSceneName());
				myGUIHandler->RemoveGUIScene(mySelectedGUIScene->GetSceneName());
				mySelectedGUIScene = nullptr;
				mySelectedGUIElement = nullptr;
				mySelectedElementIndex = -1;
				return false;
			}
		}

		ImGui::SeparatorText("Elements");

		ImGui::PushID(aGuiScene->GetSceneName().c_str());

		if (ImGui::InputTextWithHint("##newGuiElement", "New GUIElement", elementInputBuffer, 64, ImGuiInputTextFlags_EnterReturnsTrue))
		{
			std::string elementName = elementInputBuffer;
			memset(elementInputBuffer, 0, 64);

			Vector2f size = { 100.f, 100.f };
			Vector2f offset = { 0.f, 0.f };
			Vector2f resolution = { (float)myGUIHandler->myGraphics->GetRenderSize().x, (float)myGUIHandler->myGraphics->GetRenderSize().y };

			// Create new GUIElement
			aGuiScene->CreateGUIElement(
				size,
				offset,
				resolution,
				//KE::eGUIElementType::Decoration,
				false,
				KE::eAlignType::Center
			);

			RenameNotUniqueElementName(aGuiScene, elementName);

			aGuiScene->GetGUIElements().back().myName = elementName;

			myGUIHandler->AssignSpriteToGUIElement(
				aGuiScene->GetGUIElements().back(),
				myGUIHandler->myGraphics,
				{ 100.f, 100.f },
				"Data\\EngineAssets\\KEDefault_c.dds");

			mySelectedGUIElement = &aGuiScene->GetGUIElements().back();
			mySelectedElementIndex = static_cast<int>(aGuiScene->GetGUIElements().size() - 1);
		}

		auto& elements = aGuiScene->GetGUIElements();
		static int draggedElementIndex = -1;

		GUIDragDropPayload guiPayload;
		guiPayload.sourceScene = aGuiScene;


		for (int i = 0; i < elements.size(); i++)
		{
			ImGui::PushID(i);

			guiPayload.elementIndex = i;

			if (i < elements.size())
			{
				if (ImGui::Selectable((elements[i].myName).c_str(), mySelectedGUIElement == &elements[i]))
				{
					mySelectedGUIElement = &elements[i];
					mySelectedElementIndex = i;
				}
			}

			/*	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					draggedElementIndex = i;
				}*/

			if (ImGui::BeginDragDropSource())
			{
				ImGui::SetDragDropPayload("DRAGGABLE_ELEMENT", &guiPayload, sizeof(guiPayload));
				ImGui::Text("%s", elements[i].myName.c_str());
				ImGui::EndDragDropSource();
				std::cout << "\nDragging element: " << elements[i].myName;
			}

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAGGABLE_ELEMENT"))
				{
					auto* received = (GUIDragDropPayload*)payload->Data;
					int payloadIndex = received->elementIndex;

					if (received->sourceScene != aGuiScene)
					{
						myGUIHandler->MoveElementBetweenScenes(received->sourceScene->GetSceneName(), aGuiScene->GetSceneName(), payloadIndex);
					}
					else
					{
						aGuiScene->MoveElement(payloadIndex, i);
					}
				}
				ImGui::EndDragDropTarget();
			}

			ImGui::PopID();
		}

		ImGui::PopID();

		ImGui::Separator();

		return true;
	}

	void GUIEditor::LoadGUIFromFile()
	{
		myGUIHandler->LoadGUIFromFile();
	}

	void GUIEditor::Render()
	{
		if (myGUIHandler != nullptr)
		{
			ImGui::Text("Save");
			if (ImGui::Button("Save GUI"))
			{
				if (!myGUIHandler->myGUISceneMap.empty())
				{
					KE::GUIFile guiFile(myGUIHandler->myGUISceneMap);
					guiFile.Save("Data/GUI/GUI.prrmao");
				}
				else
				{
					KE_ERROR("Whoops! No GUIScenes to save!");
				}
			}

			if (ImGui::Button("Load GUI"))
			{
				LoadGUIFromFile();
			}

			DisplayDebugOptions();

			ImGui::Dummy({ 0, 40 });
			ImGui::SeparatorText("Scenes");

			if (ImGui::InputTextWithHint("##newGuiScene", "New GUIScene", sceneInputBuffer, 64, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::string sceneName = sceneInputBuffer;
				memset(sceneInputBuffer, 0, 64);

				RenameNotUniqueSceneName(myGUIHandler, sceneName);

				// Create new GUIScene
				myGUIHandler->CreateGUIScene(sceneName);
				mySelectedGUIScene = myGUIHandler->GetGUIScene(sceneName);
				myGUIHandler->PushGUIScene(sceneName);
			}

			const bool hasSelectedGUIElement = mySelectedGUIElement != nullptr;

			for (auto& scenePair : myGUIHandler->myGUISceneMap)
			{
				ImGui::PushID(scenePair.first.c_str());
				bool sceneDisplayed = IsSceneActive(&scenePair.second);

				if (ImGui::Checkbox("##sceneActive", &sceneDisplayed))
				{
					SetSceneActive(&scenePair.second, sceneDisplayed);
				}

				ImGui::SameLine();

				bool treeNodeOpen = ImGui::TreeNode(scenePair.first.c_str());

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAGGABLE_ELEMENT"))
					{
						auto* received = (GUIDragDropPayload*)payload->Data;
						int payloadIndex = received->elementIndex;

						myGUIHandler->MoveElementBetweenScenes(received->sourceScene->GetSceneName(), scenePair.first, payloadIndex);
					}
					ImGui::EndDragDropTarget();
				}

				if (treeNodeOpen)
				{
					mySelectedGUIScene = &scenePair.second;
					DisplayGuiSceneElements(&scenePair.second);
					if (mySelectedGUIScene == nullptr)
					{
						ImGui::TreePop();
						ImGui::PopID();
						return;
					}
					ImGui::TreePop();
				}

				ImGui::PopID();
			}

			if (hasSelectedGUIElement)
			{
				ImGui::Dummy({ 0, 40 });
				ImGui::SeparatorText("Selected Element");
				DisplaySelectedGuiElement();
			}
		}
	}

	bool GUIEditor::IsSceneActive(const KE::GUIScene* aScene) const
	{
		for (auto& scene : myGUIHandler->myActiveGUIScenes)
		{
			if (scene == aScene)
			{
				return true;
			}
		}
		return false;
	}

	void GUIEditor::SetSceneActive(const KE::GUIScene* aScene, const bool aActive) const
	{
		if (aActive)
		{
			myGUIHandler->PushGUIScene(aScene->GetSceneName());
		}
		else
		{
			myGUIHandler->PopGUIScene(aScene->GetSceneName());
		}
	}

	void GUIEditor::OnReceiveEvent(ES::Event& aEvent)
	{
		if (const KE::InputEvent* event = dynamic_cast<KE::InputEvent*>(&aEvent))
		{
			const Vector2f mousePosition = {
				static_cast<float>(event->myMousePosition.x), static_cast<float>(event->myMousePosition.y)
			};

			/// -----------------------------------------------------------------------------------------------
			///	Pressed
			if ((event->myInputType == KE::eInputType::LeftClick ||
				event->myInputType == KE::eInputType::RightClick) &&
				event->myInteractionType == KE::eInteractionType::Pressed)
			{
				if (!lockSelect)
				{
					// If mouse is over GUI
					if (myGUIHandler->IsMouseOverGUI(mousePosition))
					{
						KE::GUIElement* element = myGUIHandler->GetGUIElementFromMousePosition(mousePosition);

						if (element != nullptr)
						{
							mySelectedGUIElement = element;
							mySelectedGUIScene = element->myParentScene;
						}
					}
				}

			}
		}
	}

	void GUIEditor::OnInit()
	{
		ES::EventSystem::GetInstance().Attach<KE::InputEvent>(this);
	}

	void GUIEditor::OnDestroy()
	{
		ES::EventSystem::GetInstance().Detach<KE::InputEvent>(this);
	}
}

#endif