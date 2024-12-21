#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Graphics/CBuffer.h"
#include "Engine/Source/Graphics/ModelData.h"

namespace P8
{
	struct WaterBufferData
	{
		Vector3f waterFogColour = {0.0f, 0.25f, 1.0f};
		float waterFogDensity = 0.1f;
		Vector3f causticColour = { 0.8f, 0.8f, 1.0f };
		float causticStrength = 1.0f;
		Vector3f waterFoamColour = { 1.0f, 1.0f, 1.0f };
		float waterFoamStrength = 1.0f;
	};

	class WaterPlane : public KE::Component
	{
	private:
		KE::ModelData waterModel;
		WaterBufferData waterBufferData;

		KE::CBuffer waterBuffer;

	public:
		WaterPlane(KE::GameObject& aGO);
		~WaterPlane();
		void Awake() override;
		void SetData(void* aData = nullptr) override;
		void Update() override;

		WaterBufferData* GetBufferData() { return &waterBufferData; }
		void UpdateBuffer();

	protected:
		void OnDestroy() override;
	};
}
