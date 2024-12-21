#pragma once
#include <Engine/Source/Graphics/Graphics.h>
#include "Editor/Source/EditorWindows/Window.h"

namespace KE_EDITOR
{
	class Editor;

	enum class RenderType
	{
		DeferredModel,
		VFXModel,
		VFXSequence,
		DeferredSkeletalModel,
		FullscreenQuad,

		Count
	};

	inline const char* EnumToString(RenderType aType)
	{
		switch (aType)
		{
		case RenderType::DeferredModel: return "Deferred Model";
		case RenderType::VFXModel: return "VFX Model";
		case RenderType::VFXSequence: return "VFX Sequence";
		case RenderType::DeferredSkeletalModel: return "Deferred Skeletal Model";
		case RenderType::FullscreenQuad: return "Fullscreen Quad";
		default: return "Unknown";
		}
	}


	class ModelViewer : public EditorWindowBase
	{
		KE_EDITOR_FRIEND
	private:
		union
		{
			KE::ModelData myModelData = {};
			KE::SkeletalModelData mySkeletalModelData;
		};
		Transform myModelTransform;

		KE::RenderTarget* myRenderTarget;
		KE::GBuffer* myBuffer;
		KE::DeferredLightManager myLightManager;
		KE::FullscreenAsset myFullscreenAsset;

		bool isSkeletal = false;

		KE::ModelData myModelViewerGround = {};
		Transform myGroundTransform;
		Vector2f lastRenderRectMin;
		Vector2f lastRenderRectMax;

		KE::VFXSequence* myVFXSequence = nullptr;

		RenderType myRenderType = RenderType::DeferredModel;

		bool HoveringOverRender() const;
	public:
		ModelViewer(EditorWindowInput aStartupData = {}) : EditorWindowBase(aStartupData) {}
		~ModelViewer() override {};

		const char* GetWindowName() const override { return "Model Viewer"; }
		void Init() override;
		void Update() override;
		void Render() override;

		const KE::ModelData& GetModelData() const { return myModelData; }

		void RenderCustom(RenderType aType, KE::VertexShader* aVS, KE::PixelShader* aPS, KE::Material* aMaterial, const Vector2f& aViewportSize);
	};

}
