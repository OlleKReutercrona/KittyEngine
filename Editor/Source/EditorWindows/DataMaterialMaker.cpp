#include "stdafx.h"

#include <format>
#ifndef KITTYENGINE_NO_EDITOR

#include "DeferredView.h"

#include "Editor/Source/EditorUtils.h"
#include "Editor/Source/ImGui/ImGuiHandler.h"
#include "Graphics/Graphics.h"
#include "imgui/imgui.h"
#include "DataMaterialMaker.h"

void KE_EDITOR::MaterialEditor::Init()
{
}

void KE_EDITOR::MaterialEditor::Update()
{
}

void KE_EDITOR::MaterialEditor::Render()
{
	bool changed = false;

	ImGui::InputText("Material Name", materialParameters.materialName, 64);
	changed = ImGui::ColorEdit3("Albedo", &materialParameters.albedo.x) ? true : changed;	
	changed = ImGui::SliderFloat("Metallic", &materialParameters.metallic, 0.0f, 1.0f) ? true : changed;
	changed = ImGui::SliderFloat("Roughness", &materialParameters.roughness, 0.0f, 1.0f) ? true : changed;
	//changed = ImGui::SliderFloat("Ambient Occlusion", &materialParameters.ao, 0.0f, 1.0f) ? true : changed;
	changed = ImGui::SliderFloat("Emission", &materialParameters.emission, 0.0f, 1.0f) ? true : changed;

	if (ImGui::Button("Create Material") || changed )
	{
		KE::DataMaterial newMaterial;
		newMaterial.myName = materialParameters.materialName;
		newMaterial.albedo = materialParameters.albedo;
		newMaterial.metallic = materialParameters.metallic;
		newMaterial.roughness = materialParameters.roughness;
		newMaterial.emission = materialParameters.emission;

		const std::string& intermediatePath = "intermediateMaterial.catMat";
		auto& texLoader = KE_GLOBAL::blackboard.Get<KE::Graphics>()->GetTextureLoader();
		texLoader.SaveDataMaterial(intermediatePath, newMaterial);
		madeMaterial = texLoader.LoadDataMaterial(intermediatePath, true);

		std::string thumbnailPath = std::format("Data/InternalAssets/MaterialThumbnails/{}.dds", materialParameters.materialName);

		texLoader.RenderThumbnail(madeMaterial, thumbnailPath);
		thumbnailTexture = texLoader.GetTextureFromPath(thumbnailPath, false, true);

		//{
		//	KE::Texture* thumbnailTex = nullptr;
		//	std::string thumbnailPath = std::format(
		//		"Data/InternalAssets/MaterialThumbnails/{}.dds",
		//		rrToUse.myMaterial->myName
		//	);
		//	editor->myTextureLoader->RenderThumbnail(rrToUse.myMaterial, thumbnailPath);
		//	thumbnailTex = editor->myTextureLoader->GetTextureFromPath(thumbnailPath, false, true);
		//	if (thumbnailTex)
		//	{
		//		ImGui::Image(thumbnailTex->myShaderResourceView.Get(), ImVec2(64, 64));
		//	}
		//}
	}

	if (madeMaterial)
	{
		ImGuiHandler::DisplayMaterial(madeMaterial);
		ImGui::Image(thumbnailTexture->myShaderResourceView.Get(), { 512, 512 });
	}
}
#endif