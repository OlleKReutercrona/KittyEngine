#include "stdafx.h"
#ifndef KITTYENGINE_NO_EDITOR

#include "DeferredView.h"

#include "Editor/Source/EditorUtils.h"
#include "Editor/Source/ImGui/ImGuiHandler.h"
#include "Graphics/Graphics.h"
#include "imgui/imgui.h"

void KE_EDITOR::DeferredView::Init()
{
	myBuffer = &KE_GLOBAL::blackboard.Get<KE::Graphics>("graphics")->GetGBuffer();
	//myBuffer = &KE_GLOBAL::blackboard.Get<KE::Graphics>("graphics")->GetSecondaryGBuffer();
	mySSAO = &KE_GLOBAL::blackboard.Get<KE::Graphics>("graphics")->GetSSAO();
}

void KE_EDITOR::DeferredView::Update()
{
}

void KE_EDITOR::DeferredView::Render()
{
	ID3D11ShaderResourceView* const* shaderResourceViews = myBuffer->GetShaderResourceViews();
	ID3D11ShaderResourceView* const* depthView = myBuffer->GetDepthShaderResourceView();
	ID3D11ShaderResourceView* SSAOView = mySSAO->GetSSAOSRV();
	ID3D11ShaderResourceView* BlurView = mySSAO->GetBlurSRV();
	ID3D11ShaderResourceView* noise = mySSAO->GetNoiseRSV();

	ImGui::Columns(2);
	ImGui::Text("#0 World Position");
	ImGui::Image(shaderResourceViews[0], ImVec2(512, 288));
	ImGui::Text("#2 Albedo");
	ImGui::Image(shaderResourceViews[1], ImVec2(512, 288));
	ImGui::Text("#4 Normal");
	ImGui::Image(shaderResourceViews[2], ImVec2(512, 288));
	ImGui::Text("#6 SSAO");
	ImGui::Image(SSAOView, ImVec2(512, 288));
	ImGui::Text("#6 SSAO - Noise");
	ImGui::Image(noise, ImVec2(512, 288));

	ImGui::NextColumn();
	ImGui::Text("#1 Material");
	ImGui::Image(shaderResourceViews[3], ImVec2(512, 288));
	ImGui::Text("#3 Effects");
	ImGui::Image(shaderResourceViews[4], ImVec2(512, 288));
	ImGui::Text("#5 Ambient Occlusion");
	ImGui::Image(shaderResourceViews[5], ImVec2(512, 288));
	ImGui::Text("#7 Depth");
	ImGui::Image(depthView[0], ImVec2(512, 288));
	ImGui::Text("#8 SSAO - Blur");
	ImGui::Image(BlurView, ImVec2(512, 288));
}
#endif