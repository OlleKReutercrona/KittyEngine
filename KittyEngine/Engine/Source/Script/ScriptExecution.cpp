#include "stdafx.h"
#include "Node.h"
#include "Script.h"

#include "ScriptExecution.h"

//systems
#include "Engine/Source/ComponentSystem/GameObjectManager.h"
#include "Engine/Source/Collision/CollisionHandler.h"

namespace KE
{
	//-----------------------------------------------------------------------------------------------------------------------------
	//NODE EXECUTOR
	//-----------------------------------------------------------------------------------------------------------------------------

	NodeExecutor::NodeExecutor()
	{
	}

	void NodeExecutor::Init(Script* aScript, ScriptRuntime* aRuntime)
	{
		myScript = aScript;
		myRuntime = aRuntime;
	}

	PinValue NodeExecutor::GetPinValue(Pin* aPin)
	{
		auto& connections = myScript->GetConnections();
		if (!connections.contains(aPin->ID))
		{
			return aPin->value;
		}

		auto& connectionVector = connections[aPin->ID];
		if (connectionVector.size() == 0)
		{
			return aPin->value;
		}

		ScriptMemberID connectedID = connectionVector[0].to;
		ScriptMemberID nodeID = connectedID;
		nodeID.idParts.pinID = 0;

		auto& nodes = myScript->GetNodes();
		auto& node = nodes[nodeID];
		int pinIndex = connectedID.idParts.pinID - (int)node->GetInputPins().size();

		auto result = node->GetOutputPinValue(pinIndex, this);

		return result;
	}

	void NodeExecutor::AddExecution(ScriptMemberID anID, int anExecutionContext)
	{
		executionSequence.push_back(anID);
		if (anExecutionContext >= 0)
		{
			ScriptMemberID nodeID = anID;
			nodeID.idParts.pinID = 0;
			myRuntime->executionContexts[nodeID].push_back(anExecutionContext);
		}
	}

	void NodeExecutor::ExecuteMacro(ScriptMacro* aMacro, ScriptMemberID aTriggeringNode)
	{
		ScriptRuntime subRuntime(aMacro->macroScript, myRuntime->runningGameObject);
		subRuntime.macroExecutionData.macroExecutionNode = myScript->GetNodes()[aTriggeringNode];
		subRuntime.macroExecutionData.macroExecutor = this;
		subRuntime.Execute(aMacro->macroStartID);
	}

	PinValue NodeExecutor::GetMacroValue(ScriptMacro* aMacro, ScriptMemberID aTriggeringNode, int aPinIndex)
	{
		ScriptRuntime subRuntime(aMacro->macroScript, myRuntime->runningGameObject);
		subRuntime.macroExecutionData.macroExecutionNode = myScript->GetNodes()[aTriggeringNode];
		subRuntime.macroExecutionData.macroExecutor = this;

		return subRuntime.myExecutor.GetPinValue(&aMacro->macroScript->GetNodes()[aMacro->macroEndID]->GetInputPins()[aPinIndex]);
	}

	bool NodeExecutor::IsFinished()
	{
		return executionSequence.size() == 0;
	}

	ScriptMemberID NodeExecutor::GetNextExecution()
	{
		ScriptMemberID id = executionSequence.back();
		return id;
	}

	void NodeExecutor::PopExecution()
	{
		executionSequence.pop_back();
	}

	void NodeExecutor::SetVariable(const std::string& aName, const PinValue& aValue)
	{
		myRuntime->runtimeScriptVariables[aName] = aValue;
		//myScript->SetVariable(aName, aValue);
	}

	PinValue NodeExecutor::GetVariable(const std::string& aName) const
	{
		return myRuntime->runtimeScriptVariables[aName];
		//return myScript->GetVariable(aName);
	}

	void NodeExecutor::SetNodeVariable(ScriptMemberID anID, const std::any& aValue)
	{
		nodeVariables[anID] = aValue;
	}

	std::any* NodeExecutor::GetNodeVariable(ScriptMemberID anID)
	{
		if (nodeVariables.contains(anID))
		{
			return &nodeVariables.at(anID);
		}
		return nullptr;
	}

	int NodeExecutor::GetContext(ScriptMemberID anID)
	{
		if (!myRuntime->executionContexts.contains(anID)) { return -1; }
		if (myRuntime->executionContexts[anID].size() == 0) { return -1; }
		return myRuntime->executionContexts[anID].back();
	}

	GameObject* NodeExecutor::GetGameObject() const
	{
		return myRuntime->runningGameObject;
	}

	KE::GameObject* NodeExecutor::GetGameObject(int aGameObjectID) const
	{
		return myRuntime->myGameObjectManager->GetGameObject(aGameObjectID);
	}

	std::vector<Collider*> NodeExecutor::GetCollidersInBox(const Vector3f& aPosition, const Vector3f& aSize, const int aLayerMask)
	{
		Box box(aSize / -2.0f, aSize / 2.0f, aPosition);

		std::vector<Collider*> result = myRuntime->myCollisionHandler->BoxCast(box, aLayerMask);

		return result;
	}

	MacroExecutionData* NodeExecutor::GetMacroExecutionData()
	{
		return &myRuntime->macroExecutionData;
	}

	//-----------------------------------------------------------------------------------------------------------------------------
	//SCRIPT RUNTIME
	//-----------------------------------------------------------------------------------------------------------------------------

	void ScriptRuntime::TriggerEntryPoint(EntryPointType aType)
	{
		for (auto& entryPoint : myScript->GetEntryPoints()[static_cast<int>(aType)])
		{
			Execute(entryPoint);
		}
	}

	ScriptRuntime::ScriptRuntime() {}

	void ScriptRuntime::Init(Script* aScript, GameObject* aGameObject)
	{
		myCollisionHandler = KE_GLOBAL::blackboard.Get<KE::CollisionHandler>("collisionHandler");
		myGameObjectManager = KE_GLOBAL::blackboard.Get<KE::GameObjectManager>("gameObjectManager");

		myScript = aScript;
		runningGameObject = aGameObject;
		myExecutor.Init(aScript, this);
	}

	void ScriptRuntime::Execute(const ScriptMemberID& anExecutionPointID)
	{
		for (auto& connection : myScript->GetConnections()[anExecutionPointID])
		{

			ScriptMemberID nodeID = connection.to;
			nodeID.idParts.pinID = 0;

			ScriptNode* node = myScript->GetNodes()[nodeID];

			size_t sizePre = myExecutor.executionSequence.size();
			node->Execute(&myExecutor);
			size_t sizePost = myExecutor.executionSequence.size();
			size_t sizeDiff = sizePost - sizePre;
			for (size_t i = 0; i < sizeDiff; i++)
			{
				ScriptMemberID nextNode = myExecutor.GetNextExecution();
				Execute(nextNode);

				ScriptMemberID fromNodeID = anExecutionPointID;
				fromNodeID.idParts.pinID = 0;

				if (executionContexts.contains(fromNodeID) && executionContexts[fromNodeID].size() > 0)
				{
					executionContexts[fromNodeID].pop_back();
				}
				myExecutor.PopExecution();
			}
		}
	}
}