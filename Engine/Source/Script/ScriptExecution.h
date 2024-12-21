#pragma once

namespace KE
{
	class CollisionHandler;
	class GameObjectManager;
	class GameObject;
	class Script;
	struct ScriptMacro;
	class NodeExecutor;
	class ScriptNode;

	struct MacroExecutionData
	{
		//the node that started the macro, not a node inside the macro, but a macro execution node.
		ScriptNode* macroExecutionNode = nullptr;

		//who started the macro, not who is running it
		NodeExecutor* macroExecutor = nullptr;
	};

	//Class used in script execution
	class NodeExecutor
	{
		friend class ScriptRuntime;
	private:
		Script* myScript;
		ScriptRuntime* myRuntime;
		std::vector<ScriptMemberID> executionSequence;

		std::unordered_map<ScriptMemberID, std::any, ScriptMemberID> nodeVariables;
		void PopExecution();



	public:
		NodeExecutor();
		void Init(Script* aScript, ScriptRuntime* aRuntime);

		PinValue GetPinValue(Pin* aPin);

		void AddExecution(ScriptMemberID anID, int anExecutionContext = -1);

		void ExecuteMacro(ScriptMacro* aMacro, ScriptMemberID aTriggeringNode);
		PinValue GetMacroValue(ScriptMacro* aMacro, ScriptMemberID aTriggeringNode, int aPinIndex);

		bool IsFinished();
		ScriptMemberID GetNextExecution();

		void SetVariable(const std::string& aName, const PinValue& aValue);
		PinValue GetVariable(const std::string& aName) const;

		void SetNodeVariable(ScriptMemberID anID, const std::any& aValue);
		std::any* GetNodeVariable(ScriptMemberID anID);

		int GetContext(ScriptMemberID anID);

		//objects!
		//returns the gameobject that started the script
		KE::GameObject* GetGameObject() const;
		//returns the gameobject with the given id
		KE::GameObject* GetGameObject(int aGameObjectID) const;

		//collision!
		std::vector<KE::Collider*> GetCollidersInBox(const Vector3f& aPosition, const Vector3f& aSize, const int aLayerMask = 0);

		MacroExecutionData* GetMacroExecutionData();
	};

	class ScriptRuntime
	{
		friend class NodeExecutor;
	private:
		Script* myScript = nullptr;
		NodeExecutor myExecutor;
		std::unordered_map<std::string, PinValue> runtimeScriptVariables;

		struct ExecutionContext
		{
			int context = -1;
			ScriptMemberID owner = {};
		};

		MacroExecutionData macroExecutionData;

		std::unordered_map<ScriptMemberID, std::vector<int>, ScriptMemberID> executionContexts;

		KE::GameObject* runningGameObject = nullptr;

		//available systems
		CollisionHandler* myCollisionHandler = nullptr;
		GameObjectManager* myGameObjectManager = nullptr;

	public:
		//You also NEED to Init the ScriptRuntime with a script and a gameobject!
		ScriptRuntime();
		ScriptRuntime(Script* aScript, GameObject* aGameObject) { Init(aScript, aGameObject); }
		void Init(Script* aScript, GameObject* aGameObject);
		~ScriptRuntime() = default;

		void Execute(const ScriptMemberID& anExecutionPointID);
		void TriggerEntryPoint(EntryPointType aType);

	};

}