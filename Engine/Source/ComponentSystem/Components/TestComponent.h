#pragma once
#include "Component.h"
#include "Reflection/Reflection.h"

namespace KE
{
	class GameObject;

	struct TestComponentData
	{
		bool testBool;
		int testInt;
		float testFloat;
	};

	class TestComponent : public Component
	{
	public:
		TestComponent(GameObject& aGameObject);
		~TestComponent();

		void Awake() override;
		void LateUpdate() override;
		void Update() override;

		void OnDestroy() override;
		void OnEnable() override;
		void OnDisable() override;

		void SetData(void* aDataObject = nullptr) override;

		void Test() {};


	private:
		friend class KE_SER::Serializer;
		template<class SerializationIO>
		void Serialize(SerializationIO& aSerializer)
		{
			SET_REFLECTION(TestComponent);
			aSerializer & PARAM(myTestBool)
						& PARAM(myTestInt)
						& PARAM(myTestFloat)
						& PARAM(myTestVector3)
						& PARAM(myTestVector2)
			;
		}
	private:
		bool myTestBool = false;
		int myTestInt = -4;
		float myTestFloat = 0.521f;

		Vector3f myTestVector3 = {123.0f, 234.0f, 999.0f};
		Vector2i myTestVector2 = {1337, 1984};
	};
}
