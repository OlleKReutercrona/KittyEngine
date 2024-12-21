#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"

namespace KE
{
	class DecalManager;

	struct DecalComponentData
	{
		int myDecalIndex;
	};

	class DecalComponent : public Component
	{
		KE_EDITOR_FRIEND
	private:
		int myDecalIndex;
		DecalManager* myDecalManager;

	public:
		DecalComponent(GameObject& aParentGameObject);

		void SetData(void* aDataObject) override;
		void DefaultData() override;

	protected:
		void Awake() override;
		void LateUpdate() override;
		void Update() override;
		void OnEnable() override;
		void OnDisable() override;
		void OnDestroy() override;
		void DrawDebug(KE::DebugRenderer& aDbg) override;
	};

}