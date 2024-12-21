#pragma once
#include "Component.h"
#include "Engine/Source/Script/ScriptExecution.h"

namespace KE
{
	class Script;
	class ScriptManager;

	struct ScriptComponentData
	{
		Script* script = nullptr;
	};

	class ScriptComponent : public Component
	{
	private:
		ScriptRuntime myScriptRuntime;
		Script* myScript = nullptr;

	public:
		ScriptComponent(GameObject& aParentGameObject);
		~ScriptComponent();

		void SetData(void* aDataObject) override;
		void DefaultData() override;

		void Awake() override;
		void Update() override;
	};

}
