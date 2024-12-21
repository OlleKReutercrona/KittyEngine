#pragma once
#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"


namespace P8
{
	class Player;

	struct PowerupAreaData
	{
		Vector3f min;
		Vector3f max;
	};

	class PowerupAreaComponent : public KE::Component
	{
	private:
		Vector3f myMin;
		Vector3f myMax;

	public:
		PowerupAreaComponent(KE::GameObject& aParentGameObject) : Component(aParentGameObject)
		{
		}

		void SetData(void* aDataObject) override;

		inline const Vector3f& GetMin() const { return myMin; }
		inline const Vector3f& GetMax() const { return myMax; }

	protected:
		void Awake() override;
		void Update() override;
		void OnEnable() override;
		void OnDisable() override;
		void DrawDebug(KE::DebugRenderer& aDbg) override;
	};
}
