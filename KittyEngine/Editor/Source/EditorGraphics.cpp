#include "stdafx.h"

#ifndef KITTYENGINE_NO_EDITOR

#include "EditorGraphics.h"

#include <wincodec.h>

#include "ImGui/ImGuiHandler.h"

#include <External/Include/dxtex/DirectXTex.h>

#include "Utility/Global.h"

namespace KE_EDITOR
{
#pragma region ModelThumbnail

	bool SaveImage(const std::string& aFilePath, KE::Graphics* aGraphics, const int size, KE::RenderTarget& rt)
	{
		std::string fileNameBase = GetFileNameFromPath(aFilePath);
		std::string finalName = "Data/InternalAssets/ModelThumbnails/" + fileNameBase + ".dds";

		return aGraphics->GetTextureLoader().SaveTextureToFile(
			finalName,
			rt.GetShaderResourceView(),
			size,
			size
		);
		
	}

	bool RenderModelThumbnail(KE::Graphics* aGraphics, const std::string& aFilePath, bool aSave, KE::RenderTarget** aOutRenderTarget)
	{

		auto& modelLoader = aGraphics->GetModelLoader();

		if (modelLoader.IsModelSkeletal(aFilePath))
		{
			return false; // don't render thumbnails for skeletal models
		}

		KE::MeshList* ml = &modelLoader.Load(aFilePath);

		if (ml->myMeshes.size() == 0)
		{
			//KE_ERROR("Thumbnail Generation failed for empty mesh %s.", aFilePath.c_str());
			return false;
		}

		static float rotationTimer = 0;
		rotationTimer += KE_GLOBAL::deltaTime;

		static float s = 1.2f;
		static Vector2f oDim = { 16.0f, 9.0f };
		static Vector2f pRot = { -20.0f, 0.0f };
		
		//set up a transform for the mesh list
		Transform transform;

		static KE::ModelData modelData;
		modelData.myTransform = &transform.GetMatrix();
		modelData.myMeshList = ml;
		if (modelData.myRenderResources.size() == 0)
		{
			auto& wd = modelData.myRenderResources.emplace_back();
			wd.myVertexShader = aGraphics->GetShaderLoader().GetVertexShader(SHADER_LOAD_PATH "Model_Tooltip_VS.cso");
			wd.myPixelShader = aGraphics->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH "Model_Tooltip_PS.cso");
		}
		if (modelData.myMeshList->myMaterialNames.size() > 0)
		{
			modelData.myRenderResources.back().myMaterial = aGraphics->GetTextureLoader().GetMaterialFromPath(modelData.myMeshList->myMaterialNames[0]);
		}
		else
		{
			modelData.myRenderResources.back().myMaterial = aGraphics->GetTextureLoader().GetDefaultMaterial();
		}


		Vector3f average = modelData.myMeshList->myMeshes[0].myBounds.min + modelData.myMeshList->myMeshes[0].myBounds.max;
		average /= 2.0f;

		Vector3f dims = modelData.myMeshList->myMeshes[0].myBounds.max - modelData.myMeshList->myMeshes[0].myBounds.min;

		float rotOff = 0.0f;

		bool deeperThanWide = dims.x < dims.z;
		if (deeperThanWide)
		{
			rotOff = -90.0f;
		}
		bool tallerThanHorizontal = dims.y > dims.x && dims.y > dims.z;

		transform.SetPosition(average * -1.0f + Vector3f(0.0f, 0.0f, 0.0f));
		transform.RotateLocal({ pRot.y * KE::DegToRadImmediate, 0.0f, 0.0f });
		transform.RotateWorld({ 0.0f, (pRot.x + rotOff) * KE::DegToRadImmediate, 0.0f });
		if (!aSave)
		{
			transform.RotateWorld({ 0.0f, rotationTimer, 0.0f });
		}

		//ImGui::DragFloat2("Ortho Size", &oDim.x);
		//ImGui::DragFloat2("rot", &pRot.x);
		//
		//
		//auto mov = ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle);
		//pRot.x += mov.x * -0.1f;
		//pRot.y += mov.y * -0.1f;
		//ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
		//
		//
		//
		//ImGui::DragFloat("s", &s, 0.01f, 0.01f, 100.0f);

		constexpr float aspect = 9.0f / 9.0f;

		if (tallerThanHorizontal)
		{
			oDim.y = (dims.y) * s;
			oDim.x = oDim.y * aspect;
		}
		else //make sure the model fits horizontally instead
		{
			float largestDim = deeperThanWide ? dims.z : dims.x;
			oDim.y = largestDim * s;
			oDim.x = oDim.y * aspect;
		}

		

		if (oDim.x <= 0.01f) { oDim.x = 0.01f; }
		if (oDim.y <= 0.01f) { oDim.y = 0.01f; }

		KE::Camera cam;
		cam.SetOrthographic(oDim.x, oDim.y, -1000.0f, 1000.0f);

		//ImGuiHandler::DisplayModelData(&modelData);

		constexpr int size = 512;
		

		aGraphics->SetViewport(size, size);

		float clear[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		static KE::RenderTarget rt;
		static bool rtInit = false;
		if (!rtInit)
		{
			rt.Init(aGraphics->GetDevice().Get(), aGraphics->GetContext().Get(), size, size);
			rtInit = true;
		}

		rt.Clear(clear);
		aGraphics->SetRenderTarget(&rt);

		if (aOutRenderTarget)
		{
			*aOutRenderTarget = &rt;
		}

		aGraphics->SetRasterizerState(KE::eRasterizerStates::NoCulling);

		aGraphics->SetView(cam.GetViewMatrix());
		aGraphics->SetProjection(cam.GetProjectionMatrix());
		//aGraphics->RenderModel(modelData);
		aGraphics->GetDefaultRenderer()->RenderModel({ nullptr, cam.GetViewMatrix(), cam.GetProjectionMatrix() }, modelData);

		//if (!aSave)
		//{
		//	ImGui::Image(
		//		rt.GetShaderResourceView(),
		//		ImVec2(256, 256)
		//	);
		//}

		//ImGui::Image(
		//	rt.GetShaderResourceView(),
		//	ImVec2(64.0f, 64.0f)
		//);

		aGraphics->SetRasterizerState(KE::eRasterizerStates::BackfaceCulling);

		// Configure viewport
		{
			D3D11_VIEWPORT vp = {};
			vp.Width = static_cast<FLOAT>(aGraphics->GetRenderWidth());
			vp.Height = static_cast<FLOAT>(aGraphics->GetRenderHeight());
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0.0f;
			vp.TopLeftY = 0.0f;
			aGraphics->GetContext()->RSSetViewports(1u, &vp);
		}

		if (aSave)
		{
			return SaveImage(modelData.myMeshList->myFilePath, aGraphics, size, rt);
		}

		return false;
	}

#pragma endregion

#pragma region EditorIcons

	EditorIcon::EditorIcon(KE::Graphics* aGraphics, const std::string& aPath)
	{
		myTexture = aGraphics->GetTextureLoader().GetTextureFromPath(aPath);
	}

	ID3D11ShaderResourceView* EditorIcon::GetSRV() const
	{
		return myTexture->myShaderResourceView.Get();
	}
#pragma endregion


	
	//
#pragma region KittyVFX

#pragma region VFXCurveEdit
	
	VFXCurveEdit::VFXCurveEdit(std::array<KE::VFXCurveDataSet, (size_t)KE::VFXAttributeTypes::Count>* aCurveData) : curveData(aCurveData)
	{
		mMax = ImVec2(1.f, 1.f);
		mMin = ImVec2(0.f, 0.f);
	}
	size_t VFXCurveEdit::GetCurveCount()
	{
		return (size_t)KE::VFXAttributeTypes::Count;
	}

	bool VFXCurveEdit::IsVisible(size_t curveIndex)
	{
		return (*curveData)[curveIndex].visible;
	}
	size_t VFXCurveEdit::GetPointCount(size_t curveIndex)
	{
		return (*curveData)[curveIndex].myData.size();
	}

	uint32_t VFXCurveEdit::GetCurveColor(size_t curveIndex)
	{
		//uint32_t cols[] = { 0xFF0000FF, 0xFF00FF00, 0xFFFF0000 };
		//return cols[curveIndex];

		return CURVE_COLORS[(int)curveData->at(curveIndex).myType];
	}

	ImVec2* VFXCurveEdit::GetPoints(size_t curveIndex)
	{
		return (ImVec2*)(*curveData)[curveIndex].myData.data();
	}

	ImCurveEdit::CurveType VFXCurveEdit::GetCurveType(size_t curveIndex) const 
	{ 
		return (ImCurveEdit::CurveType)curveData->at(curveIndex).myCurveProfile; 
	}

	int VFXCurveEdit::EditPoint(size_t curveIndex, int pointIndex, ImVec2 value)
	{
		(*curveData)[curveIndex].myData[pointIndex] = Vector2f(value.x, value.y);
		SortValues(curveIndex);
		for (size_t i = 0; i < GetPointCount(curveIndex); i++)
		{
			if ((*curveData)[curveIndex].myData[i].x == value.x)
				return (int)i;
		}
		return pointIndex;
	}

	void VFXCurveEdit::CreateCurve(KE::VFXAttributeTypes aType)
	{
		int index = (int)aType;
		auto& curve = curveData->at(index);
		curve = KE::VFXCurveDataSet();
		curve.myType = aType;
		curve.myMinValue = KE::VFXCurveDefaults[index].x;
		curve.myMaxValue = KE::VFXCurveDefaults[index].y;

		float d = KE::VFXCurveDefaults[index].z;

		AddPoint(index, ImVec2(0.f, d));
		AddPoint(index, ImVec2(1.f, d));
	}

	void VFXCurveEdit::RemoveCurve(KE::VFXAttributeTypes curveIndex)
	{
		curveData->at((int)curveIndex) = KE::VFXCurveDataSet();
	}

	void VFXCurveEdit::AddPoint(size_t curveIndex, ImVec2 value)
	{
		(*curveData)[curveIndex].myData.push_back({ value.x, value.y });
		SortValues(curveIndex);
	}

	bool VFXCurveEdit::IsPointVisible(size_t aCurveIndex, size_t aPointIndex)
	{
		//if (curveData->at(aCurveIndex).myType == KE::VFXAttributeTypes::PARTICLE_BURST)
		//{
		//	if (aPointIndex == 0 || aPointIndex == curveData->at(aCurveIndex).myData.size() - 1)
		//	{
		//		return false; // don't show the first and last point of the burst curve
		//	}
		//}

		return true;
	}

	void VFXCurveEdit::SortValues(size_t curveIndex)
	{
		auto b = (*curveData)[curveIndex].myData.begin();
		auto e = (*curveData)[curveIndex].myData.end();
		std::sort(b, e, [](Vector2f a, Vector2f b) { return a.x < b.x; });
	}
	
#pragma endregion

#pragma region VFXSequenceInterface
	void VFXSequenceInterface::DisplayParticleEmitter(int anIndex, ImDrawList* aDrawList, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect)
	{
		KE::VFXTimeStamp& stamp = mySequence->myTimestamps[anIndex];
		KE::ParticleEmitter& emitter = mySequence->myParticleEmitters[stamp.myEffectIndex].myEmitter;
		
		myCurveEdits[anIndex].mMin = ImVec2(0.0f, 0.f);
		myCurveEdits[anIndex].mMax = ImVec2((float)mySequence->myDuration, 1.f);

		
		aDrawList->AddText(ImVec2(legendRect.Min.x + 30, legendRect.Min.y), 0xFFFFFFFF,"Bursts");

		ImGui::SetCursorScreenPos(rc.Min);
		for (KE::VFXCurveDataSet& cd : *myCurveEdits[anIndex].curveData)
		{
			if (!cd.IsValid()) { continue; }
			cd.myData[0].x = 0.0f;
			cd.myData.back().x = (float)stamp.myEndpoint;
		}

		ImCurveEdit::Edit(
			myCurveEdits[anIndex],
			rc.Max - rc.Min,
			ImGui::GetID(FormatString("o%i n%i", anIndex, 0)),
			&myCurveEdits[anIndex].editData,
			&clippingRect,
			NULL
		);

		for (KE::VFXCurveDataSet& cd : *myCurveEdits[anIndex].curveData)
		{
			for (Vector2f& dd : cd.myData)
			{
				dd.y = 0.5f;
			}
		}
	}

	void VFXSequenceInterface::DisplayCurveEditMenu(int anIndex, int& curveToEdit)
	{
		if (ImGui::BeginPopup(KE_EDITOR::FormatString("curveEditMenu%i", anIndex)))
		{
			int currentCurve = (int)myCurveEdits[anIndex].curveData->at(curveToEdit).myCurveProfile;

			ImVec2 orgCurPos = ImGui::GetCursorPos();
			for (int i = 0; i < (int)KE::VFXCurveProfiles::Count; i++)
			{
				if (ImGui::RadioButton(KE::VFXCurveProfileNames[i], currentCurve == i))
				{
					myCurveEdits[anIndex].curveData->at(curveToEdit).myCurveProfile = (KE::VFXCurveProfiles)i;
				}
			}
			ImVec2 realCurPos = ImGui::GetCursorPos();

			ImGui::SetCursorPos(orgCurPos + ImVec2(128.0f, 0.0f));
			if (ImGui::DragFloat("Min", &myCurveEdits[anIndex].curveData->at(curveToEdit).myMinValue, 0.01f)) {}
			ImGui::SetCursorPos(orgCurPos + ImVec2(128.0f, 20.0f));
			if (ImGui::DragFloat("Max", &myCurveEdits[anIndex].curveData->at(curveToEdit).myMaxValue, 0.01f)) {}

			ImGui::SetCursorPos(realCurPos);
			ImGui::NewLine();
			ImGui::Separator();
			if (ImGui::MenuItem("Delete Curve"))
			{
				myCurveEdits[anIndex].RemoveCurve((KE::VFXAttributeTypes)curveToEdit);
			}
			ImGui::EndPopup();
		}
	}

	void VFXSequenceInterface::DisplayVFXMeshInstance(int anIndex, ImDrawList* aDrawList, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect)
	{
		KE::VFXTimeStamp& stamp = mySequence->myTimestamps[anIndex];

		//aDrawList->AddRectFilled(clippingRect.Min, clippingRect.Max, IM_COL32_BLACK);

		myCurveEdits[anIndex].mMin = ImVec2(0.0f, 0.f);
		myCurveEdits[anIndex].mMax = ImVec2((float)mySequence->myDuration, 1.f);
		//aDrawList->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);

		static int curveToEdit = -1;
		int visibleCurveIndex = 0;
		for (int i = 0; i < (int)KE::VFXAttributeTypes::Count; i++)
		{
			if (!myCurveEdits[anIndex].curveData->at(i).IsValid()) { continue; }
			ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + visibleCurveIndex * 14.f);
			ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (visibleCurveIndex + 1) * 14.f);
			aDrawList->AddText(pta, myCurveEdits[anIndex].IsVisible(i) ? 0xFFFFFFFF : 0x80FFFFFF, myCurveEdits[anIndex].curveData->at(i).GetName());
			if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()))
			{
				if (ImGui::IsMouseClicked(0) && ImGui::IsWindowFocused())
				{
					myCurveEdits[anIndex].curveData->at(i).visible = !myCurveEdits[anIndex].curveData->at(i).visible;
				}
				else if (ImGui::IsMouseClicked(1) && ImGui::IsWindowFocused())
				{
					ImGui::OpenPopup(KE_EDITOR::FormatString("curveEditMenu%i", anIndex));
					curveToEdit = i;
				}

			}

			visibleCurveIndex++;
		}

		DisplayCurveEditMenu(anIndex, curveToEdit);


		ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + visibleCurveIndex * 14.f);
		ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (visibleCurveIndex + 1) * 14.f);
		aDrawList->AddText(pta, 0x80FFFFFF, "[Add Attribute]");
		if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0) && ImGui::IsWindowFocused())
		{
			ImGui::OpenPopup(KE_EDITOR::FormatString("addAttribute%i", anIndex));
		}

		KE::VFXAttributeTypes typeToAdd = KE::VFXAttributeTypes::Count;
		if (ImGui::BeginPopup(KE_EDITOR::FormatString("addAttribute%i", anIndex)))
		{
			for (KE::VFXAttributeTypes type = (KE::VFXAttributeTypes)0; type < KE::VFXAttributeTypes::Count; type = (KE::VFXAttributeTypes)((int)type + 1))
			{
				if (ImGui::Selectable(KE::VFXAttributeTypeNames[(int)type]))
				{
					typeToAdd = type;
				}
			}

			ImGui::EndPopup();
		}
		if (typeToAdd != KE::VFXAttributeTypes::Count)
		{
			myCurveEdits[anIndex].CreateCurve(typeToAdd);
		}

		//aDrawList->PopClipRect();

		ImGui::SetCursorScreenPos(rc.Min);
		for (KE::VFXCurveDataSet& cd : *myCurveEdits[anIndex].curveData)
		{
			if (!cd.IsValid()) { continue; }
			cd.myData[0].x = 0.0f;
			cd.myData.back().x = 1.0f;

			cd.myData[0].y = ImClamp(cd.myData[0].y, 0.0f, 1.0f);
			cd.myData.back().y = ImClamp(cd.myData.back().y, 0.0f, 1.0f);
		}

		for (KE::VFXCurveDataSet& cd : *myCurveEdits[anIndex].curveData)
		{
			for (Vector2f& dd : cd.myData)
			{
				dd.x = (float)stamp.myStartpoint + ((float)stamp.myEndpoint - (float)stamp.myStartpoint) * dd.x;
			}
		}

		ImCurveEdit::Edit(
			myCurveEdits[anIndex],
			rc.Max - rc.Min,
			ImGui::GetID(FormatString("o%i n%i", anIndex, 0)),
			&myCurveEdits[anIndex].editData,
			&clippingRect,
			NULL
		);

		static int sp, spc = -1;
		if (myCurveEdits[anIndex].editData.selectedPoint > -1)
		{
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
			{
				ImGui::OpenPopup("pointEditPopup");
				sp = myCurveEdits[anIndex].editData.selectedPoint;
				spc = myCurveEdits[anIndex].editData.selectedPointCurve;
			}
		}
		if (ImGui::BeginPopup("pointEditPopup"))
		{
			auto& point = myCurveEdits[anIndex].curveData->at(spc).myData[sp];
			ImGui::DragFloat("Keyframe", &point.x);
			ImGui::DragFloat("Value", &point.y);

			ImGui::EndPopup();
		}
		else if(myCurveEdits[anIndex].editData.overCurve > -1)
		{
			sp = -1; spc = -1;
			ImGui::SetTooltip("%s\nValue: %.3f",
				myCurveEdits[anIndex].curveData->at(myCurveEdits[anIndex].editData.overCurve).GetName(),
				myCurveEdits[anIndex].curveData->at(myCurveEdits[anIndex].editData.overCurve).GetEvaluatedValue(
					static_cast<int>(myCurveEdits[anIndex].editData.overCurveT * static_cast<float>((stamp.myStartpoint + stamp.myEndpoint))),
					stamp.myStartpoint,
					stamp.myEndpoint
				));
		}

		for (KE::VFXCurveDataSet& cd : *myCurveEdits[anIndex].curveData)
		{
			for (Vector2f& dd : cd.myData)
			{
				//turn dd.x back from the span [myStartpoint - > myEndpoint] to [0 - > 1]
				dd.x = (dd.x - (float)stamp.myStartpoint) / ((float)stamp.myEndpoint - (float)stamp.myStartpoint);
				dd.y = dd.y < 0.0f ? 0.0f : dd.y > 1.0f ? 1.0f : dd.y;
			}
		}
	}

	void VFXSequenceInterface::Link(KE::VFXSequence* aSequence)
	{
		mySequence = aSequence;
		myCurveEdits.clear();
		for (auto& timestamp : mySequence->myTimestamps)
		{
			myCurveEdits.emplace_back(&timestamp.myCurveDataSets);
		} 

	}

	int VFXSequenceInterface::GetFrameMin() const
	{
		return myStart;
	}

	int VFXSequenceInterface::GetFrameMax() const
	{
		return mySequence->myDuration;
	}

	int VFXSequenceInterface::GetItemCount() const
	{
		return static_cast<int>(mySequence->myTimestamps.size());/*static_cast<int>(mySequence->myVFXMeshes.size()) + static_cast<int>(mySequence->myParticleEmitters.size());*/
	}

	void VFXSequenceInterface::Get(int index, int** start, int** end, int* type, unsigned int* color)
	{
		KE::VFXTimeStamp& item = mySequence->myTimestamps[index];

		if (color)
		{
			if (item.myType == KE::VFXType::ParticleEmitter)
			{
				*color = 0xFF80AA80;
			}
			else
			{
				*color = 0xFFAA8080;
			}
		}

		if (start)
		{
			*start = &item.myStartpoint;
		}

		if (end)
		{
			*end = &item.myEndpoint;
		}

		if (type)
		{
			*type = (int)item.myType;
		}
	}

	void VFXSequenceInterface::BeginEdit(int anIndex)
	{
		mySelectedIndex = anIndex;
	}

	void VFXSequenceInterface::EndEdit()
	{
		mySelectedIndex = -1;
	}

	const char* VFXSequenceInterface::GetItemTypeName(int typeIndex) const
	{
		return typeIndex == (int)KE::VFXType::ParticleEmitter ? "ParticleEmitter" : "VFXMeshInstance";
	}

	const char* VFXSequenceInterface::GetItemLabel(int index) const
	{
		return FormatString("%s %i",
			mySequence->myTimestamps[index].myType == KE::VFXType::ParticleEmitter ? "ParticleEmitter" : "VFXMeshInstance",
			mySequence->myTimestamps[index].myEffectIndex
		);
	}

	const char* VFXSequenceInterface::GetSequenceName() const
	{
		return mySequence->myName.c_str();
	}

	void VFXSequenceInterface::Add(int type)
	{
		KE::VFXTimeStamp& ts = mySequence->myTimestamps.emplace_back();

		ts.myType = (KE::VFXType)type;
		
		if (type == (int)KE::VFXType::VFXMeshInstance)
		{
			mySequence->AddVFXMeshInstance();
			ts.myEffectIndex = static_cast<int>(mySequence->myVFXMeshes.size()) - 1;
		}
		else if (type == (int)KE::VFXType::ParticleEmitter)
		{
			mySequence->AddParticleEmitter();
			ts.myEffectIndex = static_cast<int>(mySequence->myParticleEmitters.size()) - 1;
			//ts.myCurveDataSets[(int)KE::VFXAttributeTypes::PARTICLE_BURST].myType = KE::VFXAttributeTypes::PARTICLE_BURST;
			//ts.myCurveDataSets[(int)KE::VFXAttributeTypes::PARTICLE_BURST].myCurveProfile = KE::VFXCurveProfiles::Linear;
			//ts.myCurveDataSets[(int)KE::VFXAttributeTypes::PARTICLE_BURST].myData.push_back({ 0.0f, 0.5f });
			//ts.myCurveDataSets[(int)KE::VFXAttributeTypes::PARTICLE_BURST].myData.push_back({ 0.0f, 0.5f });
		}
		
		
		//myCurveEdits.push_back(&ts.myCurveDataSets);

		Link(mySequence);
	};

	size_t VFXSequenceInterface::GetCustomHeight(int index)
	{
		if (!mySequence->myTimestamps[index].myIsOpened) { return 0; }

		switch (mySequence->myTimestamps[index].myType)
		{
		case KE::VFXType::ParticleEmitter:
		{
			return 32;
		}
		case KE::VFXType::VFXMeshInstance:
		{
			int count = 0;
			for (auto& curve : mySequence->myTimestamps[index].myCurveDataSets)
			{
				if (curve.IsValid()) { count++; }
			}
			int size = 128 + (count > 8 ? count - 8 : 0) * 16;
			return size;
		}
		default:
		{
			return 0;
		}
		}
	}

	void VFXSequenceInterface::StoreRect(int index, const ImRect& rect)
	{
		myCurveEdits[index].editData.sequenceRect = rect;
	}

	bool VFXSequenceInterface::IsExpanded(int index)
	{
		return mySequence->myTimestamps[index].myIsOpened;
	}

	void VFXSequenceInterface::SetExpanded(int index, bool state)
	{
		mySequence->myTimestamps[index].myIsOpened = state;
	}

	void VFXSequenceInterface::Del(int index)
	{
		//KE::VFXTimeStamp& ts = mySequence->myTimestamps[index];
		//switch(ts.myType)
		//{
		//case KE::VFXType::ParticleEmitter:
		//{
		//	mySequence->myParticleEmitters.erase(mySequence->myParticleEmitters.begin() + ts.myEffectIndex);
		//	break;
		//}
		//case KE::VFXType::VFXMeshInstance:
		//{
		//	mySequence->myVFXMeshes.erase(mySequence->myVFXMeshes.begin() + ts.myEffectIndex);
		//	break;
		//}
		//default: break;
		//}
		//
		//mySequence->myTimestamps.erase(mySequence->myTimestamps.begin() + index);
		//
		//Link(mySequence);
		
	}

	void VFXSequenceInterface::Duplicate(int index)
	{
		//return;

		Add((int)mySequence->myTimestamps[index].myType);
		
		auto& newTS = mySequence->myTimestamps.back();
		auto& oldTS = mySequence->myTimestamps[index];
		newTS.myStartpoint = oldTS.myStartpoint;
		newTS.myEndpoint = oldTS.myEndpoint;
		newTS.myType = oldTS.myType;
		newTS.myCurveDataSets = oldTS.myCurveDataSets;
		
		switch(newTS.myType)
		{
		case KE::VFXType::ParticleEmitter:
		{
			mySequence->myParticleEmitters.back() = mySequence->myParticleEmitters[oldTS.myEffectIndex];
			break;
		}
		case KE::VFXType::VFXMeshInstance:
		{
			mySequence->myVFXMeshes.back() = mySequence->myVFXMeshes[oldTS.myEffectIndex];
			break;
		}
		}
		
		Link(mySequence);
	}

	void VFXSequenceInterface::DoubleClick(int anIndex)
	{
		std::cout << anIndex << " double clicked" << std::endl;
	}

	void VFXSequenceInterface::CustomDraw(int anIndex, ImDrawList* aDrawList, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect)
	{
		const KE::VFXTimeStamp& stamp = mySequence->myTimestamps[anIndex];

		if (!stamp.myIsOpened) { return; }

		switch (stamp.myType)
		{
		case KE::VFXType::ParticleEmitter:
		{
			DisplayParticleEmitter(anIndex, aDrawList, rc, legendRect, clippingRect, legendClippingRect);
			break;
		}
		case KE::VFXType::VFXMeshInstance:
		{
			DisplayVFXMeshInstance(anIndex, aDrawList, rc, legendRect, clippingRect, legendClippingRect);
			break;
		}
		default: break;
		}
	}
#pragma endregion

#pragma endregion
}
#endif