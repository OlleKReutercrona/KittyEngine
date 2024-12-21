#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Utility/KittyArray.h"

namespace KE
{
	struct ShellModelData;

	struct ShellTexturingComponentData
	{
		LaserPtr<ShellModelData> shellModelData;
		bool liveUpdate = true;
	};

	struct DisplacementObject
	{
		GameObject* gameObject;
		Vector3f displacementScale;
	};

	class ShellTexturingComponent : public Component
	{
		KE_EDITOR_FRIEND
	private:
		LaserPtr<ShellModelData> shellModelData = {nullptr,0};

		std::vector<DisplacementObject> myDisplacementObjects;
		std::vector<ShellTexturingComponent*> otherShellTexturingComponents;

		Graphics* myGraphics = nullptr; //ew

		struct Bounds
		{
			Vector3f min;
			Vector3f max;
		} sharedBounds;

		bool primary = true;

		float refreshTimer = 10.0f;
		float refreshTime = 10.0f;

		bool hasSetlevelDisplacement = false;
		bool liveUpdate = true;

	public:
		ShellTexturingComponent(GameObject& aParentGameObject);

		void SetData(void* aDataObject) override;
		void DefaultData() override;

		void Refresh();
		void CalculateBounds();
		void RenderDisplacement();
		void SetDisplacementForLevel();
		void ClearDisplacement();

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
