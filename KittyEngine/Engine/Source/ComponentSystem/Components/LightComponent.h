#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Graphics/Lighting/Lighting.h"
#include <string>

namespace KE
{
	class DeferredLightManager;
	class LightManager;
	class Light;

	struct LightComponentData
	{
		LightData* myLightData;
		eLightType myLightType;
		std::string myLightTypeName;
		UINT myIndex;
		UINT myTypeIndex;
		DeferredLightManager* myLightManager;
	};

	class LightComponent : public Component
	{
		friend class LightManager;

	public:
		LightComponent(GameObject& aGameObject);

		~LightComponent();

		void SetData(void* aDataObject = nullptr) override;
		void Update() override;

		inline const LightComponentData* GetLightData() const { return &myLightComponentData; }

		void ToggleDrawDebug();

		void DrawDebug(KE::DebugRenderer& aDbg) override;
	private:
		void UpdateLightData() const;

	protected:
		void OnEnable() override;
		void OnDisable() override;
		void OnDestroy() override;

	private:
		bool myIsDebugDraw = false;
		LightComponentData myLightComponentData = {};
		DeferredLightManager* myManager = nullptr;
	};
}
