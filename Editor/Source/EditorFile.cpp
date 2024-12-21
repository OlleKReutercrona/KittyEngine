#include "stdafx.h"
#include "ImGui/ImGuiHandler.h"
#include "Editor.h"
#include "EditorGraphics.h"

#include "EditorFile.h"

KE_EDITOR::EditorFile::EditorFile(const EditorFile& aFile)
{
	myType = aFile.myType;
	myPath = aFile.myPath;
	myName = aFile.myName;
	myIconPath = aFile.myIconPath;
}

KE_EDITOR::EditorFileInteraction KE_EDITOR::EditorFile::Display(KE::TextureLoader* aTextureLoader) const
{
	return BeginDisplayBox(aTextureLoader);
}

KE_EDITOR::EditorFileInteraction KE_EDITOR::EditorFile::BeginDisplayBox(KE::TextureLoader* aTextureLoader) const
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

	KE_EDITOR::EditorFileInteraction result = EditorFileInteraction::eNone;

	if (ImGui::BeginChild(myPath.c_str(), ImVec2(64.0f, 64.0f) + ImVec2(0.0f, 40.0f), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 0.2f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.2f, 0.2f, 0.2f));

		result = DisplayIcon(aTextureLoader);
		DisplayName();

		ImGui::PopStyleColor(3);
	}
	ImGui::EndChild();

	ImGui::PopStyleVar(3);

	return result;
}

KE_EDITOR::EditorFileInteraction KE_EDITOR::EditorFile::DisplayIcon(KE::TextureLoader* aTextureLoader) const
{
	const KE::Texture* iconTex = aTextureLoader->GetTextureFromPath(myIconPath);
	if (!iconTex)  { return EditorFileInteraction::eNone; }

	if (iconTex->myMetadata.myTextureFormat != KE::TextureFormat::Texture2D)
	{
		aTextureLoader->ProjectCubemap(myIconPath, KE::CubemapProjectionType::Equirectangular);
		KE::RenderTarget* rt = KE_GLOBAL::blackboard.Get<KE::Graphics>()->GetRenderTarget(9);

		ImGui::Image(rt->GetShaderResourceView(), {64.0f, 64.0f});
	}



	ImGui::ImageButton(myPath.c_str(), iconTex->myShaderResourceView.Get(), ImVec2(64.0f, 64.0f));

	//

	if (ImGui::BeginDragDropSource())
	{
		if (myType == EditorFileType::eTexture)
		{
			char buffer[256];
			strcpy_s(buffer, myIconPath.c_str());

			if (!ImGui::SetDragDropPayload("texturePath", buffer, sizeof(buffer)))
			{
				ImGui::BeginTooltip();
				ImGui::ImageButton(iconTex->myShaderResourceView.Get(), ImVec2(64.0f, 64.0f));
				ImGui::SameLine();
				ImGui::Text("Texture: %s", myPath.c_str());
				ImGui::EndTooltip();
			}
		}
		else if (myType == EditorFileType::eModel)
		{
			char buffer[256];
			strcpy_s(buffer, myPath.c_str());

			if (!ImGui::SetDragDropPayload("modelPath", buffer, sizeof(buffer)))
			{
				ImGui::BeginTooltip();

				auto* graphics = KE_GLOBAL::blackboard.Get<KE::Graphics>("graphics");
				KE::RenderTarget* rt;
				RenderModelThumbnail(graphics, myPath, false, &rt);
				ImGui::Image(rt->GetShaderResourceView(), ImVec2(256.0f, 256.0f));

				ImGui::Text("Model: %s", myPath.c_str());
				ImGui::EndTooltip();
			}
		}

		ImGui::EndDragDropSource();
	}

	if (ImGui::IsItemHovered())
	{
		if (ImGui::IsMouseDoubleClicked(0)) { return EditorFileInteraction::eLeftDoubleClick; }
		if (ImGui::IsMouseClicked(0)) { return EditorFileInteraction::eLeftClick; }
		if (ImGui::IsMouseDoubleClicked(1)) { return EditorFileInteraction::eRightDoubleClick; }
		if (ImGui::IsMouseClicked(1)) { return EditorFileInteraction::eRightClick; }

		return EditorFileInteraction::eHovered;
	}
	return EditorFileInteraction::eNone;
		
}

void KE_EDITOR::EditorFile::DisplayName() const
{
	ImGui::TextWrapped(myName.c_str());
}