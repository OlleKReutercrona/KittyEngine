#pragma once
#include <Engine/Source/Graphics/Graphics.h>
#include <Engine/Source/Graphics/Texture/TextureLoader.h>
#include <Engine/Source/Graphics/FX/VFX.h>
#include <External/Include/imguizmo/ImCurveEdit.h>
#include <External/Include/imguizmo/ImSequencer.h>
#include <External/Include/imgui/imgui_internal.h>

#include <string>

#include "EditorUtils.h"

namespace KE_EDITOR
{
#pragma region ModelThumbnail

	bool RenderModelThumbnail(KE::Graphics* aGraphics, const std::string& aFilePath, bool aSave, KE::RenderTarget** aOutRenderTarget = nullptr);

#pragma endregion

#pragma region EditorIcons

	enum class EditorIconType
	{
		eFolder,
		eReturn,
		eFile,
		eLevel,
		eUnknown,
		eArchive,

		eTexture,

		eCount
	};
	struct EditorIcon
	{
		KE::Texture* myTexture;
		
		EditorIcon() = default;
		EditorIcon(KE::Graphics* aGraphics, const std::string& aPath);
		EditorIcon(KE::Texture* aTexture) : myTexture(aTexture) {}

		ID3D11ShaderResourceView* GetSRV() const;
	};
#pragma endregion

#pragma region KittyVFX



	inline static uint32_t CURVE_COLORS[] = 
	{
		ImColor(127, 255, 212, 255),
		ImColor(249, 66,  158, 255),

		ImColor(0,0,0,255),
		ImColor(0,0,0,255),
		ImColor(0,0,0,255),
		
		ImColor(0,0,0,255),
		ImColor(0,0,0,255),
		ImColor(0,0,0,255),
		
		ImColor(0,0,0,255),
		ImColor(0,0,0,255),
		ImColor(0,0,0,255),
		
		ImColor(255,0,0,255),
		ImColor(0,255,0,255),
		ImColor(0,0,255,255),
		ImColor(255,255,255,255),

		ImColor(255,255,255,255),
		ImColor(255,255,255,255),

		ImColor(255,255,255,255),
	};

	struct VFXCurveEdit final : public ImCurveEdit::Delegate
	{
		explicit VFXCurveEdit(std::array<KE::VFXCurveDataSet, (size_t)KE::VFXAttributeTypes::Count>* aCurveData);
		
		size_t GetCurveCount() override;
		bool IsVisible(size_t curveIndex) override;
		size_t GetPointCount(size_t curveIndex) override;
		uint32_t GetCurveColor(size_t curveIndex) override;
		ImVec2* GetPoints(size_t curveIndex) override;

		ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const override;
		int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value) override;
		void CreateCurve(KE::VFXAttributeTypes aType);
		void RemoveCurve(KE::VFXAttributeTypes aType);
		void AddPoint(size_t curveIndex, ImVec2 value) override;

		virtual bool IsPointVisible(size_t aCurveIndex, size_t aPointIndex) override;


		ImVec2& GetMax() override { return mMax; }
		ImVec2& GetMin() override { return mMin; }
		unsigned int GetBackgroundColor() override { return 0; }
		
		std::array<KE::VFXCurveDataSet, (size_t)KE::VFXAttributeTypes::Count>* curveData;
		ImCurveEdit::EditData editData;

		ImVec2 mMin;
		ImVec2 mMax;

	private:
		
		void SortValues(size_t curveIndex);
	};

	class VFXSequenceInterface final : public ImSequencer::SequenceInterface
	{
		KE_EDITOR_FRIEND;
	private:
		
		KE::VFXSequence* mySequence = nullptr;
		int myStart = 0;
		int mySelectedIndex = -1;
		std::vector<VFXCurveEdit> myCurveEdits;

		void DisplayParticleEmitter(int anIndex, ImDrawList* aDrawList, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect);
		void DisplayCurveEditMenu(int anIndex, int& curveToEdit);
		void DisplayVFXMeshInstance(int anIndex, ImDrawList* aDrawList, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect);

	public:
		void Link(KE::VFXSequence* aSequence);

		int GetFrameMin() const override;
		int GetFrameMax() const override;
		int GetItemCount() const override;

		int GetItemTypeCount() const override { return 2; }

		void Get(int index, int** start, int** end, int* type, unsigned int* color) override;
		void BeginEdit(int anIndex) override;
		void EndEdit() override;

		void Add(int type) override;
		void Del(int index) override;
		void Duplicate(int index) override;
		const char* GetItemTypeName(int typeIndex) const override;
		const char* GetItemLabel(int index) const override;
		const char* GetSequenceName() const override;
		size_t GetCustomHeight(int index) override;

		void StoreRect(int index, const ImRect& rect) override;

		bool IsExpanded(int index) override;
		void SetExpanded(int index, bool state) override;

		void DoubleClick(int anIndex) override;

		void CustomDraw(int anIndex, ImDrawList* aDrawList, const ImRect& rc, const ImRect& legendRect, const ImRect& clippingRect, const ImRect& legendClippingRect) override;
	};

#pragma endregion
}