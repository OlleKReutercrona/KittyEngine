#include "stdafx.h"
#include "NodeDatabase.h"

#include "ScriptExecution.h"
#include "Script.h"

#include <Editor/Source/EditorWindows/NodeEditor.h>

#include "Collision/Collider.h"
#include "Collision/Layers.h"
#include "ComponentSystem/GameObject.h"
#include "ComponentSystem/Components/Component.h"
#include "Engine/Source/Math/KittyMath.h"
#include <External/Include/nlohmann/json.hpp>
#include <Engine/Source/AI/BehaviourTree/BehaviourTreeBuilder.h>

#include "CodeNode.h"


//project!

#include "HLSLDefiner.h"
#include "ComponentSystem/Components/Collider/BoxColliderComponent.h"
#include "Editor/Source/ImGui/ImGuiHandler.h"
#include "ImNodes/ImNodes.h"

//

#define DefineNode(name, category, description) \
	const char* GetName() const override { return name; } \
	NodeCategory GetCategory() const override { return category; } \
	const char* GetDescription() const override { return description; }\

KE::NodeTypeDatabase::NodeTypeDatabase()
{
	//--------------------------------------------------------------------------------------
	//NODE DEFINITIONS
	//--------------------------------------------------------------------------------------

	//Constant Constructors
	class ConstantBoolNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Bool", NodeCategory::Math, "Outputs a Bool constant.");

		ConstantBoolNode() : ScriptNode()
		{
		}

		virtual void Init() override { AddOutputPin(ValueType::Bool, PinType::Value, "Value"); }
	}; RegisterNode<ConstantBoolNode>("Math/Constants");
	class ConstantIntNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Int", NodeCategory::Math, "Outputs an Integer constant.");

		ConstantIntNode() : ScriptNode()
		{
		}

		virtual void Init() override { AddOutputPin(ValueType::Int, PinType::Value, "Value"); }
	};RegisterNode<ConstantIntNode>("Math/Constants");
	class ConstantFloatNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Float", NodeCategory::Math, "Outputs a float constant.");

		ConstantFloatNode() : ScriptNode()
		{
		}

		virtual void Init() override { AddOutputPin(ValueType::Float, PinType::Value, "Value"); }
	};RegisterNode<ConstantFloatNode>("Math/Constants");

	//Time
	class TotalTimeNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Total Time", NodeCategory::Math, "Outputs the seconds since the game started.");

		TotalTimeNode() : ScriptNode()
		{
		}

		virtual void Init() override { AddOutputPin(ValueType::Float, PinType::OutputValue, "Value"); }

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Float};
			result.value.Float = KE_GLOBAL::totalTime;
			return result;
		}
	};RegisterNode<TotalTimeNode>("Math/Time");
	class DeltaTimeNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Delta Time", NodeCategory::Math, "Outputs the seconds since the last frame.");

		DeltaTimeNode() : ScriptNode()
		{
		}

		virtual void Init() override { AddOutputPin(ValueType::Float, PinType::OutputValue, "Value"); }

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Float};
			result.value.Float = KE_GLOBAL::deltaTime;
			return result;
		}
	};RegisterNode<DeltaTimeNode>("Math/Time");

	//Entrypoints
	class AwakeNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Awake", NodeCategory::Flow, "Triggered when the object is awoken.");

		AwakeNode() : ScriptNode()
		{
		}

		virtual void Init() override { AddOutputPin(ValueType::FlowValue, PinType::Flow, ""); }

		virtual void CustomRender() override
		{
			ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 100);
			ImGui::Text(GetDescription());
			ImGui::PopTextWrapPos();
		}

		virtual void Execute(NodeExecutor* anExecutor) override { anExecutor->AddExecution(outputPins[0].ID); }
	};RegisterNode<AwakeNode>("Flow/Entrypoints");
	class UpdateNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Update", NodeCategory::Flow, "Triggered every frame.");

		UpdateNode() : ScriptNode()
		{
		}

		virtual void Init() override { AddOutputPin(ValueType::FlowValue, PinType::Flow, ""); }

		virtual void CustomRender() override
		{
			ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + 100);
			ImGui::Text(GetDescription());
			ImGui::PopTextWrapPos();
		}

		virtual void Execute(NodeExecutor* anExecutor) override { anExecutor->AddExecution(outputPins[0].ID); }
	};RegisterNode<UpdateNode>("Flow/Entrypoints");

	//Flow Control
	class ConditionNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Condition", NodeCategory::Flow, "Branching flows based on condition.");

		ConditionNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::FlowValue, PinType::Flow, "");
			AddInputPin(ValueType::Bool, PinType::Value, "Condition");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "True");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "False");
		}

		virtual void Execute(NodeExecutor* anExecutor) override
		{
			if (anExecutor->GetPinValue(&inputPins[1]).value.Bool) { anExecutor->AddExecution(outputPins[0].ID); }
			else { anExecutor->AddExecution(outputPins[1].ID); }
		}
	};RegisterNode<ConditionNode>("Flow/Control");

	class SwitchNode : public ScriptNode
	{
	private:
		int myCasePairs = 0;
	public:
		DefineNode("Switch", NodeCategory::Flow, "Branching flows based on defined cases.");

		SwitchNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::FlowValue, PinType::Flow, "");
			AddInputPin(ValueType::Any, PinType::Value, "Value");

			AddOutputPin(ValueType::FlowValue, PinType::Flow, "");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "Default");
		}

		

		void AddCasePair()
		{
			myCasePairs++;
			const std::string caseName = "Case " + std::to_string(myCasePairs);
			const std::string flowName = "Flow " + std::to_string(myCasePairs);

			AddInputPin(inputPins[1].value.type, inputPins[1].type, caseName.c_str());
			AddOutputPin(ValueType::FlowValue, PinType::Flow, flowName.c_str());
		}

		virtual void OnConnectPin(Pin* aFromPin, Pin* aToPin) override
		{
			if (aToPin == &inputPins[1])
			{
				inputPins[1].value.type = aFromPin->value.type;
			}
			if (aToPin == &inputPins.back())
			{
				AddCasePair();
			}
		}

		virtual void Execute(NodeExecutor* anExecutor) override
		{
			const PinValue inputValue = anExecutor->GetPinValue(&inputPins[1]);

			for (int i = 2; i < inputPins.size(); i++)
			{
				PinValue pinValue = anExecutor->GetPinValue(&inputPins[i]);
				if (inputValue == pinValue)
				{
					anExecutor->AddExecution(outputPins[i].ID);
					return;
				}
			}
			anExecutor->AddExecution(outputPins[1].ID);

		}

		virtual void ExtraSerialize(void* aOutJson, Script* aLoadingScript) override
		{
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			(*json)["casePairs"] = myCasePairs;
		}

		virtual void ExtraDeserialize(void* aOutJson, Script* aLoadingScript) override
		{
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			if (!json->contains("casePairs")) { return; }
			int casePairs = (*json)["casePairs"].get<int>();

			for (int i = 0; i < casePairs; i++) { AddCasePair(); }

		}

		virtual void CustomRender() override
		{
			if (ImGui::Button("Add Case")) { AddCasePair(); }
		}

	};RegisterNode<SwitchNode>("Flow/Control");



	class SequencerNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Sequencer", NodeCategory::Flow, "Executes flows in order.");

		SequencerNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin( ValueType::FlowValue, PinType::Flow, "");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "Flow 1");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "Flow 2");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "Flow 3");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "Flow 4");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "Flow 5");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "Flow 6");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "Flow 7");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "Flow 8");
		}

		virtual void Execute(NodeExecutor* anExecutor) override
		{
			for (int i = (int)outputPins.size() - 1; i >= 0; i--) { anExecutor->AddExecution(outputPins[i].ID); }
		}
	};RegisterNode<SequencerNode>("Flow/Control");
	class ValueLinkNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Value Link", NodeCategory::Flow, "Connects two value links, to improve structure.");

		ValueLinkNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Float, PinType::OutputValue, "");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "");
		}

		virtual bool TryAssignUnsupportedPinValue(int aPinIndex, PinValue aValue) override
		{
			inputPins[0].value.type = aValue.type;
			outputPins[0].value.type = aValue.type;
			return true;
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			return anExecutor->GetPinValue(&inputPins[0]);
		}

		virtual bool DrawHeader() override { return false; }
		virtual void BeginDrawNode() override { ImNodes::PushStyleVar(ImNodesStyleVar_NodePadding, ImVec2(4.0f, 0)); }
		virtual void EndDrawNode() override { ImNodes::PopStyleVar(); }

		virtual void CustomRender() override
		{
#ifndef KITTYENGINE_NO_EDITOR
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddRect(ImGui::GetCursorScreenPos() - ImVec2(4.0f, 1.0f),
			                  ImGui::GetCursorScreenPos() + ImVec2(20.0f, 14.0f),
			                  KE_EDITOR::NodeEditor::GetPinColour(&inputPins[0]), 5);
#endif // !KITTYENGINE_NO_EDITOR
		}
	};RegisterNode<ValueLinkNode>("Flow/Structure");
	class FlowLinkNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Flow Link", NodeCategory::Flow, "Connects two flow links, to improve structure.");

		FlowLinkNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::FlowValue, PinType::Flow, "");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "");
		}

		virtual bool DrawHeader() override { return false; }
		virtual void BeginDrawNode() override { ImNodes::PushStyleVar(ImNodesStyleVar_NodePadding, ImVec2(4.0f, 0)); }
		virtual void EndDrawNode() override { ImNodes::PopStyleVar(); }

		virtual void CustomRender() override
		{
#ifndef KITTYENGINE_NO_EDITOR

			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddRect(ImGui::GetCursorScreenPos() - ImVec2(4.0f, 1.0f),
			                  ImGui::GetCursorScreenPos() + ImVec2(20.0f, 14.0f),
			                  KE_EDITOR::NodeEditor::GetPinColour(&inputPins[0]), 5);
#endif
		}

		virtual void Execute(NodeExecutor* anExecutor) override { anExecutor->AddExecution(outputPins[0].ID); }
	};RegisterNode<FlowLinkNode>("Flow/Structure");

	//Single-Value Math
	class AddNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Add", NodeCategory::Math, "Adds two values together.");

		AddNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Float, PinType::Value, "A");
			AddInputPin(ValueType::Float, PinType::Value, "B");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "Result");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Float};
			result.value.Float += anExecutor->GetPinValue(&inputPins[0]).value.Float;
			result.value.Float += anExecutor->GetPinValue(&inputPins[1]).value.Float;
			return result;
		}
	};RegisterNode<AddNode>("Math/Operations");
	class SubtractNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Subtracts", NodeCategory::Math, "Subtracts input B from input A.");

		SubtractNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Float, PinType::Value, "A");
			AddInputPin(ValueType::Float, PinType::Value, "B");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "Result");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Float};
			result.value.Float = anExecutor->GetPinValue(&inputPins[0]).value.Float;
			result.value.Float -= anExecutor->GetPinValue(&inputPins[1]).value.Float;
			return result;
		}
	};RegisterNode<SubtractNode>("Math/Operations");
	class MultiplyNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Multiply", NodeCategory::Math, "Multiplies two values together.");

		MultiplyNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Float, PinType::Value, "A");
			AddInputPin(ValueType::Float, PinType::Value, "B");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "Result");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Float};
			result.value.Float = anExecutor->GetPinValue(&inputPins[0]).value.Float;
			result.value.Float *= anExecutor->GetPinValue(&inputPins[1]).value.Float;
			return result;
		}
	};RegisterNode<MultiplyNode>("Math/Operations");
	class DivideNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Divide", NodeCategory::Math, "Divides input A by input B.");

		DivideNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Float, PinType::Value, "A");
			AddInputPin(ValueType::Float, PinType::Value, "B");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "Result");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Float};
			result.value.Float = anExecutor->GetPinValue(&inputPins[0]).value.Float;
			result.value.Float /= anExecutor->GetPinValue(&inputPins[1]).value.Float;
			return result;
		}
	};RegisterNode<DivideNode>("Math/Operations");
	class SinNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Sine", NodeCategory::Math, "Returns the sine of the input value.");

		SinNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Float, PinType::Value, "A");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "Result");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Float};
			result.value.Float = sinf(anExecutor->GetPinValue(&inputPins[0]).value.Float);
			return result;
		}
	};RegisterNode<SinNode>("Math/Operations");
	class CosNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Cosine", NodeCategory::Math, "Returns the cosine of the input value.");

		CosNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Float, PinType::Value, "A");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "Result");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Float};
			result.value.Float = cosf(anExecutor->GetPinValue(&inputPins[0]).value.Float);
			return result;
		}
	};RegisterNode<CosNode>("Math/Operations");

	//Logic Gates
	class NOTNode : public ScriptNode
	{
	private:
	public:
		DefineNode("NOT", NodeCategory::Math, "Returns the inverse of the input.");

		NOTNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Bool, PinType::Value, "A");
			AddOutputPin(ValueType::Bool, PinType::OutputValue, "Result");
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			return {ValueType::Bool, !anExecutor->GetPinValue(&inputPins[0]).value.Bool};
		}
	};RegisterNode<NOTNode>("Math/Logic Gates");
	class ANDNode : public ScriptNode
	{
	private:
	public:
		DefineNode("AND", NodeCategory::Math, "Returns true if both inputs are true.");

		ANDNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Bool, PinType::Value, "A");
			AddInputPin(ValueType::Bool, PinType::Value, "B");
			AddOutputPin(ValueType::Bool, PinType::OutputValue, "Result");
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			return {
				ValueType::Bool,
				anExecutor->GetPinValue(&inputPins[0]).value.Bool && anExecutor->GetPinValue(&inputPins[1]).value.Bool
			};
		}
	};RegisterNode<ANDNode>("Math/Logic Gates");
	class NANDNode : public ScriptNode
	{
	private:
	public:
		DefineNode("NAND", NodeCategory::Math, "Returns true if any input is false.");

		NANDNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Bool, PinType::Value, "A");
			AddInputPin(ValueType::Bool, PinType::Value, "B");
			AddOutputPin(ValueType::Bool, PinType::OutputValue, "Result");
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			bool a = anExecutor->GetPinValue(&inputPins[0]).value.Bool;
			bool b = anExecutor->GetPinValue(&inputPins[1]).value.Bool;
			return {ValueType::Bool, !(a && b)};
		}
	};RegisterNode<NANDNode>("Math/Logic Gates");
	class ORNode : public ScriptNode
	{
	private:
	public:
		DefineNode("OR", NodeCategory::Math, "Returns true if either inputs is true.");

		ORNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Bool, PinType::Value, "A");
			AddInputPin(ValueType::Bool, PinType::Value, "B");
			AddOutputPin(ValueType::Bool, PinType::OutputValue, "Result");
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			return {
				ValueType::Bool,
				anExecutor->GetPinValue(&inputPins[0]).value.Bool || anExecutor->GetPinValue(&inputPins[1]).value.Bool
			};
		}
	};RegisterNode<ORNode>("Math/Logic Gates");
	class NORNode : public ScriptNode
	{
	private:
	public:
		DefineNode("NOR", NodeCategory::Math, "Returns true if both inputs are false.");

		NORNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Bool, PinType::Value, "A");
			AddInputPin(ValueType::Bool, PinType::Value, "B");
			AddOutputPin(ValueType::Bool, PinType::OutputValue, "Result");
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			bool a = anExecutor->GetPinValue(&inputPins[0]).value.Bool;
			bool b = anExecutor->GetPinValue(&inputPins[1]).value.Bool;
			return {ValueType::Bool, !(a || b)};
		}
	};RegisterNode<NORNode>("Math/Logic Gates");
	class XORNode : public ScriptNode
	{
	private:
	public:
		DefineNode("XOR", NodeCategory::Math, "Returns true if only one input is true.");

		XORNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Bool, PinType::Value, "A");
			AddInputPin(ValueType::Bool, PinType::Value, "B");
			AddOutputPin(ValueType::Bool, PinType::OutputValue, "Result");
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			bool a = anExecutor->GetPinValue(&inputPins[0]).value.Bool;
			bool b = anExecutor->GetPinValue(&inputPins[1]).value.Bool;
			return {ValueType::Bool, (a || b) && !(a && b)};
		}
	};RegisterNode<XORNode>("Math/Logic Gates");
	class XNORNode : public ScriptNode
	{
	private:
	public:
		DefineNode("XNOR", NodeCategory::Math, "Returns true if both inputs are the same.");

		XNORNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Bool, PinType::Value, "A");
			AddInputPin(ValueType::Bool, PinType::Value, "B");
			AddOutputPin(ValueType::Bool, PinType::OutputValue, "Result");
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			bool a = anExecutor->GetPinValue(&inputPins[0]).value.Bool;
			bool b = anExecutor->GetPinValue(&inputPins[1]).value.Bool;
			return {ValueType::Bool, a == b};
		}
	};RegisterNode<XNORNode>("Math/Logic Gates");

	class EqualNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Equals", NodeCategory::Math, "Returns true if both inputs are the same.");

		EqualNode() : ScriptNode() {}

		virtual void Init() override
		{
			AddInputPin(ValueType::Float, PinType::Value, "A");
			AddInputPin(ValueType::Float, PinType::Value, "B");
			AddOutputPin(ValueType::Bool, PinType::OutputValue, "Result");
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			float a = anExecutor->GetPinValue(&inputPins[0]).value.Float;
			float b = anExecutor->GetPinValue(&inputPins[1]).value.Float;
			return { ValueType::Bool, a == b };
		}
	}; RegisterNode<EqualNode>("Math/Comparisons");
	class GreaterThanNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Greater", NodeCategory::Math, "Returns true if A is greater than B.");

		GreaterThanNode() : ScriptNode() {}

		virtual void Init() override
		{
			AddInputPin(ValueType::Float, PinType::Value, "A");
			AddInputPin(ValueType::Float, PinType::Value, "B");
			AddOutputPin(ValueType::Bool, PinType::OutputValue, "Result");
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			float a = anExecutor->GetPinValue(&inputPins[0]).value.Float;
			float b = anExecutor->GetPinValue(&inputPins[1]).value.Float;
			return { ValueType::Bool, a > b };
		}
	}; RegisterNode<GreaterThanNode>("Math/Comparisons");
	class LesserThanNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Lesser", NodeCategory::Math, "Returns true if A is lesser than B.");

		LesserThanNode() : ScriptNode() {}

		virtual void Init() override
		{
			AddInputPin(ValueType::Float, PinType::Value, "A");
			AddInputPin(ValueType::Float, PinType::Value, "B");
			AddOutputPin(ValueType::Bool, PinType::OutputValue, "Result");
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			float a = anExecutor->GetPinValue(&inputPins[0]).value.Float;
			float b = anExecutor->GetPinValue(&inputPins[1]).value.Float;
			return { ValueType::Bool, a < b };
		}
	}; RegisterNode<LesserThanNode>("Math/Comparisons");

	//Output
	class PrintStringNode : public ScriptNode
	{
	private:
		char text[64] = "";

	public:
		DefineNode("Print String", NodeCategory::Logic, "Prints a string.");

		PrintStringNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::FlowValue, PinType::Flow, "");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "");
		}

		virtual void CustomRender() override
		{
			ImGui::SetNextItemWidth(100);
			ImGui::InputTextMultiline("##text", text, 64);
		}

		virtual void ExtraSerialize(void* aOutJson, Script* aLoadingScript) override
		{
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			(*json)["text"] = text;
		}

		virtual void ExtraDeserialize(void* aOutJson, Script* aLoadingScript) override
		{
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			if (!json->contains("text")) { return; }
			strcpy_s(text, (*json)["text"].get<std::string>().c_str());
		}

		virtual void Execute(NodeExecutor* anExecutor) override
		{
			KE_LOG(text);
			anExecutor->AddExecution(outputPins[0].ID);
		}
	};RegisterNode<PrintStringNode>("Logic/Output");


	class PrintValueNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Print Value", NodeCategory::Logic, "Prints a float.");

		PrintValueNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::FlowValue, PinType::Flow, "");
			AddInputPin(ValueType::Float, PinType::Value, "Value");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "");
		}

		virtual void Execute(NodeExecutor* anExecutor) override
		{
			switch (anExecutor->GetPinValue(&inputPins[1]).type)
			{
			case ValueType::Float: KE_LOG("%.3f",anExecutor->GetPinValue(&inputPins[1]).value.Float);
				break;
			case ValueType::Int: KE_LOG("%d",anExecutor->GetPinValue(&inputPins[1]).value.Int);
				break;
			case ValueType::Bool: KE_LOG("%s",anExecutor->GetPinValue(&inputPins[1]).value.Bool ? "true" : "false");
				break;
			case ValueType::Vector2: KE_LOG("%.3f, %.3f",anExecutor->GetPinValue(&inputPins[1]).value.Vector2.x, anExecutor->GetPinValue(&inputPins[1]).value.Vector2.y);
				break;
			case ValueType::Vector3: KE_LOG("%.3f, %.3f, %.3f", anExecutor->GetPinValue(&inputPins[1]).value.Vector3.x, anExecutor->GetPinValue(&inputPins[1]).value.Vector3.y,anExecutor->GetPinValue(&inputPins[1]).value.Vector3.z);
				break;
			case ValueType::Vector4: KE_LOG("%.3f, %.3f, %.3f, %.3f",anExecutor->GetPinValue(&inputPins[1]).value.Vector4.x, anExecutor->GetPinValue(&inputPins[1]).value.Vector4.y,anExecutor->GetPinValue(&inputPins[1]).value.Vector4.z, anExecutor->GetPinValue(&inputPins[1]).value.Vector4.w);
				break;
			case ValueType::String: KE_LOG("%s", anExecutor->GetPinValue(&inputPins[1]).value.String);
				break;
			default: KE_LOG("Unsupported type");
				break;
			}
			anExecutor->AddExecution(outputPins[0].ID);
		}

		virtual bool TryAssignUnsupportedPinValue(int aPinIndex, PinValue aValue) override
		{
			if (aPinIndex == 1)
			{
				inputPins[1].value = aValue;
				return true;
			}
			return false;
		}
	};RegisterNode<PrintValueNode>("Logic/Output");

	//Collision
	class LayerMaskNode : public ScriptNode
	{
	private:
		int layerMask = 1;

	public:
		DefineNode("Layer Mask", NodeCategory::Math, "Outputs a collision layer mask.");

		LayerMaskNode() : ScriptNode()
		{
		}

		virtual void Init() override { AddOutputPin(ValueType::Int, PinType::OutputValue, "Mask"); }

		virtual void CustomRender() override
		{
			for (int i = 0; i < (int)KE::Collision::Layers::Count; i++)
			{
				bool selected = layerMask & (1 << i);
				if (ImGui::Checkbox(KE::Collision::GetLayerAsString((KE::Collision::Layers)(1 << i)).c_str(),
				                    &selected))
				{
					if (selected) { layerMask |= (1 << i); }
					else { layerMask &= ~(1 << i); }
				}
			}
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Int, layerMask};
			return result;
		}
	};RegisterNode<LayerMaskNode>("Game/Collision");
	class CollisionNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Box Collision", NodeCategory::Logic, "Returns if any object collides with the box.");

		CollisionNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::FlowValue, PinType::Flow, "");
			AddInputPin(ValueType::Vector3, PinType::Value, "Position");
			AddInputPin(ValueType::Vector3, PinType::Value, "Size    ");
			AddInputPin(ValueType::Int, PinType::Value, "Mask");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "");
			AddOutputPin(ValueType::Container, PinType::OutputValue, "Collisions");
			inputPins[3].value.value.Int = 1;
			outputPins[1].value.value.Container.type = ValueType::GameObject;
		}

		virtual void Execute(NodeExecutor* anExecutor) override
		{
			std::vector<Collider*> collisionResult = anExecutor->GetCollidersInBox(
				anExecutor->GetPinValue(&inputPins[1]).value.Vector3,
				anExecutor->GetPinValue(&inputPins[2]).value.Vector3, 
				anExecutor->GetPinValue(&inputPins[3]).value.Int
			);
			std::vector<GameObject*> collidedGameObjects;
			for (Collider* collider : collisionResult)
			{
				if (!collider->myComponent) { continue; }
				GameObject* object = &collider->myComponent->GetGameObject();
				if (!object->IsActive()) { continue; }
				collidedGameObjects.push_back(object);
			}
			anExecutor->SetNodeVariable(ID, collidedGameObjects);
			anExecutor->AddExecution(outputPins[0].ID);
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			if (std::any* outValue = anExecutor->GetNodeVariable(ID))
			{
				std::vector<GameObject*>* collisionResult = std::any_cast<std::vector<GameObject*>>(outValue);
				return {
					ValueType::Container,
					ContainerValue(collisionResult->data(), (int)collisionResult->size(), ValueType::GameObject)
				};
			}
			return {ValueType::Container, ContainerValue(nullptr, 0, ValueType::GameObject)};
		}
	};RegisterNode<CollisionNode>("Game/Collision");

	//Variables
	class SetVariableNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Set Variable", NodeCategory::Variable, "Sets a variable.");

		SetVariableNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::FlowValue, PinType::Flow, "");
			AddInputPin(ValueType::Float, PinType::Value, "Value");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "");
		}

		virtual void Execute(NodeExecutor* anExecutor) override
		{
			auto in = anExecutor->GetPinValue(&inputPins[1]);
			anExecutor->SetVariable(inputPins[1].name, in);
			anExecutor->AddExecution(outputPins[0].ID);
		}
	};RegisterNode<SetVariableNode>("Logic/Variables");
	class GetVariableNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Get Variable", NodeCategory::Variable, "Gets a variable.");

		GetVariableNode() : ScriptNode()
		{
		}

		virtual void Init() override { AddOutputPin(ValueType::Float, PinType::OutputValue, "Value"); }

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			auto out = anExecutor->GetVariable(outputPins[0].name);
			return out;
		}
	};RegisterNode<GetVariableNode>("Logic/Variables");

	//GameObjects
	class GetGameObjectNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Get Object", NodeCategory::Variable, "Gets an object from its ID.");

		GetGameObjectNode() : ScriptNode() {}

		virtual void Init() override
		{
			AddInputPin(ValueType::Int, PinType::Value, "ID");
			AddOutputPin(ValueType::GameObject, PinType::OutputValue, "GameObject");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			int id = anExecutor->GetPinValue(&inputPins[0]).value.Int;
			GameObject* object = anExecutor->GetGameObject(id);
			return { ValueType::GameObject, object};
		}
	}; RegisterNode<GetGameObjectNode>("Game/Objects");

	class ThisGameObjectNode : public ScriptNode
	{
	private:
	public:
		DefineNode("This Object", NodeCategory::Variable, "Gets the object running the script.");

		ThisGameObjectNode() : ScriptNode()
		{
		}

		virtual void Init() override { AddOutputPin(ValueType::GameObject, PinType::OutputValue, "GameObject"); }

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			return {ValueType::GameObject, anExecutor->GetGameObject()};
		}
	};RegisterNode<ThisGameObjectNode>("Game/Objects");
	class GameObjectSetTransformNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Set Transform", NodeCategory::Variable, "Sets a GameObject's transform.");

		GameObjectSetTransformNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::FlowValue, PinType::Flow, "");
			AddInputPin(ValueType::GameObject, PinType::Value, "Object");
			AddInputPin(ValueType::Transform, PinType::Value, "Transform");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "");
		}

		virtual void Execute(NodeExecutor* anExecutor) override
		{
			if(GameObject* object = anExecutor->GetPinValue(&inputPins[1]).value.GameObject)
			{
				object->myTransform = anExecutor->GetPinValue(&inputPins[2]).value.Transform;
				KE::BoxColliderComponent* box;
				if (object->TryGetComponent(box))
				{
					box->GetCollider()->UpdatePosition(object->myTransform.GetPosition());
				}
			}
			anExecutor->AddExecution(outputPins[0].ID);
		}
	};RegisterNode<GameObjectSetTransformNode>("Game/Objects");
	class GameObjectAttributesNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Object Attributes", NodeCategory::Variable, "Gets attributes of an object");

		GameObjectAttributesNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::GameObject, PinType::Value, "GameObject");
			AddOutputPin(ValueType::Int, PinType::OutputValue, "ID");
			AddOutputPin(ValueType::String, PinType::OutputValue, "Name");
			AddOutputPin(ValueType::Transform, PinType::OutputValue, "Local Transform");
			AddOutputPin(ValueType::Transform, PinType::OutputValue, "World Transform");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			GameObject* object = anExecutor->GetPinValue(&inputPins[0]).value.GameObject;
			switch (aPinIndex)
			{
			case 0: return {ValueType::Int, object->myID};
			case 1: return {ValueType::String, object->GetName().c_str()};
			case 2: return {ValueType::Transform, object->myTransform};
			case 3: return {ValueType::Transform, object->myWorldSpaceTransform};
			default: return {};
			}
		}
	};RegisterNode<GameObjectAttributesNode>("Game/Objects");

	//Loops
	class ForEachLoopNode : public ScriptNode
	{
	private:
	public:
		DefineNode("For Each", NodeCategory::Flow, "Executes a flow for each object in a list.");

		ForEachLoopNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::FlowValue, PinType::Flow, "");
			AddInputPin(ValueType::Container, PinType::Value, "Container");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "");
			AddOutputPin(ValueType::Bool, PinType::OutputValue, "Value");
		}

		virtual void OnConnectPin(Pin* aFromPin, Pin* aToPin) override
		{
			if (aToPin != &inputPins[1]) { return; }
			outputPins[1].value.type = aFromPin->value.value.Container.type;
		}

		virtual void Execute(NodeExecutor* anExecutor) override
		{
			auto container = anExecutor->GetPinValue(&inputPins[1]).value.Container;
			switch (container.type)
			{
			case ValueType::GameObject:
				{
					for (int i = 0; i < container.size; i++)
					{
						GameObject* gameObject = ((GameObject**)container.data)[i];
						anExecutor->AddExecution(outputPins[0].ID, i);
					}
					break;
				}
			}
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			auto container = anExecutor->GetPinValue(&inputPins[1]).value.Container;
			PinValue val = {};
			val.type = container.type;
			switch (container.type)
			{
			case ValueType::GameObject:
				{
					const int context = anExecutor->GetContext(ID);
					if (context >= 0) { val.value.GameObject = ((GameObject**)container.data)[context]; }
					break;
				}
			default: break;
			}
			return val;
		}
	};RegisterNode<ForEachLoopNode>("Logic/Loops");
	class ForLoopNode : public ScriptNode
	{
	private:
	public:
		DefineNode("For Loop", NodeCategory::Flow,
		           "Executes its connections once for every number between min and max.");

		ForLoopNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::FlowValue, PinType::Flow, "");
			AddInputPin(ValueType::Int, PinType::Value, "Start");
			AddInputPin(ValueType::Int, PinType::Value, "End");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "");
			AddOutputPin(ValueType::Int, PinType::OutputValue, "Index");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "Completed");
		}

		virtual void Execute(NodeExecutor* anExecutor) override
		{
			int min = anExecutor->GetPinValue(&inputPins[1]).value.Int;
			int max = anExecutor->GetPinValue(&inputPins[2]).value.Int;
			anExecutor->AddExecution(outputPins[2].ID);
			for (int i = max; i >= min; i--) { anExecutor->AddExecution(outputPins[0].ID, i); }
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			const int context = anExecutor->GetContext(ID);
			return PinValue(ValueType::Int, context);
		}
	};RegisterNode<ForLoopNode>("Logic/Loops");

	//Transform
	class TransformGetAttributes : public ScriptNode
	{
	private:
	public:
		DefineNode("Get Transform Attributes", NodeCategory::Variable, "Gets attributes of a Transform.");

		TransformGetAttributes() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Transform, PinType::Value, "Transform");
			AddOutputPin(ValueType::Vector3, PinType::OutputValue, "Position");
			AddOutputPin(ValueType::Vector3, PinType::OutputValue, "Rotation");
			AddOutputPin(ValueType::Vector3, PinType::OutputValue, "Scale");
			AddOutputPin(ValueType::Vector3, PinType::OutputValue, "Right");
			AddOutputPin(ValueType::Vector3, PinType::OutputValue, "Up");
			AddOutputPin(ValueType::Vector3, PinType::OutputValue, "Forward");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			const Transform& transform = anExecutor->GetPinValue(&inputPins[0]).value.Transform;
			switch (aPinIndex)
			{
			case 0: return {ValueType::Vector3, transform.GetPosition()};
			case 1: return {ValueType::Vector3, transform.GetRotation()};
			case 2: return {ValueType::Vector3, transform.GetScale()};
			case 3: return {ValueType::Vector3, transform.GetRight()};
			case 4: return {ValueType::Vector3, transform.GetUp()};
			case 5: return {ValueType::Vector3, transform.GetForward()};
			default: return {};
			}
		}
	};RegisterNode<TransformGetAttributes>("Math/Transform");
	class TransformSetPosition : public ScriptNode
	{
	private:
	public:
		DefineNode("Set Transform Position", NodeCategory::Variable, "Sets position of a Transform.");

		TransformSetPosition() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Transform, PinType::Value, "Base");
			AddInputPin(ValueType::Vector3, PinType::Value, "Position");
			AddOutputPin(ValueType::Transform, PinType::OutputValue, "Modified");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			const Transform& transform = anExecutor->GetPinValue(&inputPins[0]).value.Transform;
			Transform outTransform = transform;
			outTransform.SetPosition(anExecutor->GetPinValue(&inputPins[1]).value.Vector3);
			return {ValueType::Transform, outTransform};
		}
	};RegisterNode<TransformSetPosition>("Math/Transform");
	class TransformSetRotation : public ScriptNode
	{
	private:
	public:
		DefineNode("Set Transform Rotation", NodeCategory::Variable, "Sets Rotation of a Transform.");

		TransformSetRotation() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Transform, PinType::Value, "Base");
			AddInputPin(ValueType::Vector3, PinType::Value, "Rotation");
			AddOutputPin(ValueType::Transform, PinType::OutputValue, "Modified");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			const Transform& transform = anExecutor->GetPinValue(&inputPins[0]).value.Transform;
			Transform outTransform = transform;
			outTransform.SetRotation(anExecutor->GetPinValue(&inputPins[1]).value.Vector3);
			return {ValueType::Transform, outTransform};
		}
	};RegisterNode<TransformSetRotation>("Math/Transform");
	class TransformSetScale : public ScriptNode
	{
	private:
	public:
		DefineNode("Set Transform Scale", NodeCategory::Variable, "Gets attributes of a Transform.");

		TransformSetScale() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Transform, PinType::Value, "Base");
			AddInputPin(ValueType::Vector3, PinType::Value, "Scale");
			AddOutputPin(ValueType::Transform, PinType::OutputValue, "Modified");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			const Transform& transform = anExecutor->GetPinValue(&inputPins[0]).value.Transform;
			Transform outTransform = transform;
			outTransform.SetScale(anExecutor->GetPinValue(&inputPins[1]).value.Vector3);
			return {ValueType::Transform, outTransform};
		}
	};RegisterNode<TransformSetScale>("Math/Transform");

	//Vector Construct/Deconstruct
	class ConstructVector2Node : public ScriptNode
	{
	private:
	public:
		DefineNode("Vector2", NodeCategory::Math, "Outputs a 2D Vector constant.");

		ConstructVector2Node() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Float, PinType::Value, "x");
			AddInputPin(ValueType::Float, PinType::Value, "y");
			AddOutputPin(ValueType::Vector2, PinType::OutputValue, "Value");
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Vector2};
			result.value.Vector2.x = anExecutor->GetPinValue(&inputPins[0]).value.Float;
			result.value.Vector2.y = anExecutor->GetPinValue(&inputPins[1]).value.Float;
			return result;
		}
	};RegisterNode<ConstructVector2Node>("Math/Vector/Construct");
	class ConstructVector3Node : public ScriptNode
	{
	private:
	public:
		DefineNode("Vector3", NodeCategory::Math, "Outputs a 3D Vector constant.");

		ConstructVector3Node() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Float, PinType::Value, "x");
			AddInputPin(ValueType::Float, PinType::Value, "y");
			AddInputPin(ValueType::Float, PinType::Value, "z");
			AddOutputPin(ValueType::Vector3, PinType::OutputValue, "Value");
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Vector3};
			result.value.Vector3.x = anExecutor->GetPinValue(&inputPins[0]).value.Float;
			result.value.Vector3.y = anExecutor->GetPinValue(&inputPins[1]).value.Float;
			result.value.Vector3.z = anExecutor->GetPinValue(&inputPins[2]).value.Float;
			return result;
		}
	};RegisterNode<ConstructVector3Node>("Math/Vector/Construct");
	class ConstructVector4Node : public ScriptNode
	{
	private:
	public:
		DefineNode("Vector4", NodeCategory::Math, "Outputs a 4D Vector constant.");

		ConstructVector4Node() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Float, PinType::Value, "x");
			AddInputPin(ValueType::Float, PinType::Value, "y");
			AddInputPin(ValueType::Float, PinType::Value, "z");
			AddInputPin(ValueType::Float, PinType::Value, "w");
			AddOutputPin(ValueType::Vector4, PinType::OutputValue, "Value");
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Vector4};
			result.value.Vector4.x = anExecutor->GetPinValue(&inputPins[0]).value.Float;
			result.value.Vector4.y = anExecutor->GetPinValue(&inputPins[1]).value.Float;
			result.value.Vector4.z = anExecutor->GetPinValue(&inputPins[2]).value.Float;
			result.value.Vector4.w = anExecutor->GetPinValue(&inputPins[3]).value.Float;
			return result;
		}
	};RegisterNode<ConstructVector4Node>("Math/Vector/Construct");
	class DeconstructVector2Node : public ScriptNode
	{
	private:
	public:
		DefineNode("Deconstruct Vector2", NodeCategory::Math, "Deconstructs a Vector2 to float outputs");

		DeconstructVector2Node() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Vector2, PinType::Value, "Vector");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "x");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "y");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			switch (aPinIndex)
			{
			case 0: return {ValueType::Float, anExecutor->GetPinValue(&inputPins[0]).value.Vector2.x};
			case 1: return {ValueType::Float, anExecutor->GetPinValue(&inputPins[0]).value.Vector3.y};
			default: return {};
			}
		}
	};;RegisterNode<DeconstructVector2Node>("Math/Vector/Deconstruct");
	class DeconstructVector3Node : public ScriptNode
	{
	private:
	public:
		DefineNode("Deconstruct Vector3", NodeCategory::Math, "Deconstructs a Vector3 to float outputs");

		DeconstructVector3Node() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Vector3, PinType::Value, "Vector");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "x");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "y");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "z");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			switch (aPinIndex)
			{
			case 0: return {ValueType::Float, anExecutor->GetPinValue(&inputPins[0]).value.Vector3.x};
			case 1: return {ValueType::Float, anExecutor->GetPinValue(&inputPins[0]).value.Vector3.y};
			case 2: return {ValueType::Float, anExecutor->GetPinValue(&inputPins[0]).value.Vector3.z};
			default: return {};
			}
		}
	};RegisterNode<DeconstructVector3Node>("Math/Vector/Deconstruct");
	class DeconstructVector4Node : public ScriptNode
	{
	private:
	public:
		DefineNode("Deconstruct Vector4", NodeCategory::Math, "Deconstructs a Vector4 to float outputs");

		DeconstructVector4Node() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Vector4, PinType::Value, "Vector4");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "x");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "y");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "z");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "w");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			switch (aPinIndex)
			{
			case 0: return {ValueType::Float, anExecutor->GetPinValue(&inputPins[0]).value.Vector4.x};
			case 1: return {ValueType::Float, anExecutor->GetPinValue(&inputPins[0]).value.Vector4.y};
			case 2: return {ValueType::Float, anExecutor->GetPinValue(&inputPins[0]).value.Vector4.z};
			case 3: return {ValueType::Float, anExecutor->GetPinValue(&inputPins[0]).value.Vector4.w};
			default: return {};
			}
		}
	};RegisterNode<DeconstructVector4Node>("Math/Vector/Deconstruct");

	//Vector Math
	class AddVectorNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Add Vector", NodeCategory::Math, "Adds two values together.");

		AddVectorNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Vector3, PinType::Value, "A");
			AddInputPin(ValueType::Vector3, PinType::Value, "B");
			AddOutputPin(ValueType::Vector3, PinType::OutputValue, "Result");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Vector3};
			result.value.Vector3 += anExecutor->GetPinValue(&inputPins[0]).value.Vector3;
			result.value.Vector3 += anExecutor->GetPinValue(&inputPins[1]).value.Vector3;
			return result;
		}
	};RegisterNode<AddVectorNode>("Math/Vector/Operations");
	class SubtractVectorNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Subtract Vector", NodeCategory::Math, "Subtracts input B from input A.");

		SubtractVectorNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Vector3, PinType::Value, "A");
			AddInputPin(ValueType::Vector3, PinType::Value, "B");
			AddOutputPin(ValueType::Vector3, PinType::OutputValue, "Result");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Vector3};
			result.value.Vector3 = anExecutor->GetPinValue(&inputPins[0]).value.Vector3;
			result.value.Vector3 -= anExecutor->GetPinValue(&inputPins[1]).value.Vector3;
			return result;
		}
	};RegisterNode<SubtractVectorNode>("Math/Vector/Operations");
	class MultiplyVectorNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Multiply Vector", NodeCategory::Math, "Multiplies a vector by a scalar.");

		MultiplyVectorNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Vector3, PinType::Value, "Vector");
			AddInputPin(ValueType::Float, PinType::Value, "Scalar");
			AddOutputPin(ValueType::Vector3, PinType::OutputValue, "Result");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Vector3};
			result.value.Vector3 = anExecutor->GetPinValue(&inputPins[0]).value.Vector3;
			result.value.Vector3 *= anExecutor->GetPinValue(&inputPins[1]).value.Float;
			return result;
		}
	};RegisterNode<MultiplyVectorNode>("Math/Vector/Operations");
	class DivideVectorNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Divide Vector", NodeCategory::Math, "Divides a vector by a scalar.");

		DivideVectorNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Vector3, PinType::Value, "Vector");
			AddInputPin(ValueType::Float, PinType::Value, "Scalar");
			AddOutputPin(ValueType::Vector3, PinType::OutputValue, "Result");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Vector3};
			result.value.Vector3 = anExecutor->GetPinValue(&inputPins[0]).value.Vector3;
			result.value.Vector3 /= anExecutor->GetPinValue(&inputPins[1]).value.Float;
			return result;
		}
	};RegisterNode<DivideVectorNode>("Math/Vector/Operations");
	class NormalizeVectorNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Normalize Vector", NodeCategory::Math, "Normalizes a vector so its lenth is 1.");

		NormalizeVectorNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Vector3, PinType::Value, "Vector");
			AddOutputPin(ValueType::Vector3, PinType::OutputValue, "Normalized");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Vector3};
			result.value.Vector3 = anExecutor->GetPinValue(&inputPins[0]).value.Vector3.GetNormalized();
			return result;
		}
	};RegisterNode<NormalizeVectorNode>("Math/Vector/Operations");
	class VectorLengthNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Vector Length", NodeCategory::Math, "Gets the length of a vector.");

		VectorLengthNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
			AddInputPin(ValueType::Vector3, PinType::Value, "Vector");
			AddOutputPin(ValueType::Float, PinType::OutputValue, "Length");
		}

		PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			PinValue result = {ValueType::Float};
			result.value.Float = anExecutor->GetPinValue(&inputPins[0]).value.Vector3.Length();
			return result;
		}
	};RegisterNode<VectorLengthNode>("Math/Vector/Operations");

	//Macro Nodes
	class MacroDefineInputNode : public ScriptNode
	{
	private:
		char macroName[PIN_STRING_MAX] = {0};

	public:
		DefineNode("Macro Input",NodeCategory::Macro,"Input node for a macro definition.");

		MacroDefineInputNode() : ScriptNode(){}

		virtual void Init() override { AddOutputPin(ValueType::Any, PinType::OutputValue, ""); }
		virtual void CustomRender() override { ImGui::Text(macroName); }

		virtual void Execute(NodeExecutor* anExecutor)
		{
			for (Pin& outPin : outputPins)
			{
				if (outPin.type == PinType::Flow)
				{
					anExecutor->AddExecution(outPin.ID);
				}
			}
		}

		virtual void OnConnectPin(Pin* aFromPin, Pin* aToPin) override
		{
			aFromPin->value.type = aToPin->value.type;
			aFromPin->type = aToPin->type;
			aFromPin->SetName(aToPin->name);
			if (aFromPin->ID == outputPins.back().ID) { AddOutputPin(ValueType::Any, PinType::OutputValue, ""); }
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			return anExecutor->GetMacroExecutionData()->macroExecutor->GetPinValue(&anExecutor->GetMacroExecutionData()->macroExecutionNode->GetInputPins()[aPinIndex]);
		}

		virtual void ExtraSerialize(void* aOutJson, Script* aLoadingScript) override
		{
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			(*json)["macroName"] = macroName;
		}

		virtual void ExtraDeserialize(void* aOutJson, Script* aLoadingScript) override
		{
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			strcpy_s(macroName, (*json)["macroName"].get<std::string>().c_str());
			size_t storedOutputPinCount = (*json)["outPins"].size();
			for (size_t i = outputPins.size(); i < storedOutputPinCount; i++)
			{
				AddOutputPin(ValueType::Any, PinType::OutputValue, "");
			}
		}

		virtual void* GetCustomData() override { return macroName; }

		virtual void SetCustomData(void* aData) override
		{
			struct
			{
				char name[PIN_STRING_MAX];
				ScriptMacro* macro;
			} inData;
			memcpy(&inData, aData, sizeof(inData));
			memcpy(macroName, inData.name, PIN_STRING_MAX);
		}
	};RegisterNode<MacroDefineInputNode>("Macro");
	class MacroDefineOutputNode : public ScriptNode
	{
	private:
		char macroName[PIN_STRING_MAX] = {0};

	public:
		DefineNode("Macro Output",NodeCategory::Macro,"Output node for a macro definition.");

		MacroDefineOutputNode() : ScriptNode(){}

		virtual void Init() override { AddInputPin(ValueType::Any, PinType::Value, ""); }
		virtual void CustomRender() override { ImGui::Text(macroName); }

		virtual void OnConnectPin(Pin* aFromPin, Pin* aToPin) override
		{
			aToPin->value.type = aFromPin->value.type;
			aToPin->type = aFromPin->type;
			aToPin->SetName(aFromPin->name);
			if (aToPin->ID == inputPins.back().ID) { AddInputPin(ValueType::Any, PinType::Value, ""); }
		}

		virtual void Execute(NodeExecutor* anExecutor)
		{
			for (int i = 0; i < inputPins.size(); i++)
			{
				if (inputPins[i].type != PinType::Flow) { continue; }
				auto* execData = anExecutor->GetMacroExecutionData();
				execData->macroExecutor->AddExecution(execData->macroExecutionNode->GetOutputPins()[i].ID);
			}
		}


		virtual void ExtraSerialize(void* aOutJson, Script* aLoadingScript) override
		{
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			(*json)["macroName"] = macroName;
		}

		virtual void ExtraDeserialize(void* aOutJson, Script* aLoadingScript) override
		{
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			strcpy_s(macroName, (*json)["macroName"].get<std::string>().c_str());
			size_t storedInputPinCount = (*json)["inPins"].size();
			for (size_t i = inputPins.size(); i < storedInputPinCount; i++)
			{
				AddInputPin(ValueType::Any, PinType::Value, "");
			}
			
		}

		virtual void* GetCustomData() override { return macroName; }

		virtual void SetCustomData(void* aData) override
		{
			struct
			{
				char name[PIN_STRING_MAX];
				ScriptMacro* macro;
			} inData;
			memcpy(&inData, aData, sizeof(inData));
			memcpy(macroName, inData.name, PIN_STRING_MAX);
		}
	};RegisterNode<MacroDefineOutputNode>("Macro");
	class MacroExecuteNode : public ScriptNode
	{
	private:
		char macroName[PIN_STRING_MAX] = {0};
		ScriptMacro* macro;

	public:
		DefineNode("Execute Macro", NodeCategory::Macro, "Executes a macro.");

		MacroExecuteNode() : ScriptNode()
		{
		}

		virtual void Init() override
		{
		}

		virtual void Execute(NodeExecutor* anExecutor) override
		{
			anExecutor->ExecuteMacro(macro, ID);
		}

		virtual PinValue GetOutputPinValue(int aPinIndex, NodeExecutor* anExecutor) override
		{
			return anExecutor->GetMacroValue(macro, ID, aPinIndex);
		}

		virtual void CustomRender() override { ImGui::Text(macroName); }

		virtual void ExtraSerialize(void* aOutJson, Script* aLoadingScript) override
		{
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			(*json)["macroName"] = macroName;
		}

		virtual void ExtraDeserialize(void* aOutJson, Script* aLoadingScript) override
		{
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			strcpy_s(macroName, (*json)["macroName"].get<std::string>().c_str());
			for (size_t i = 0; i < (*json)["inPins"].size(); i++) { AddInputPin(ValueType::Any, PinType::Value, ""); }
			for (size_t i = 0; i < (*json)["outPins"].size(); i++) { AddOutputPin(ValueType::Any, PinType::Value, ""); }

			macro = &aLoadingScript->GetMacros()[macroName];
		}

		virtual void* GetCustomData() override { return macroName; }

		virtual void SetCustomData(void* aData) override
		{
			struct
			{
				char name[PIN_STRING_MAX];
				ScriptMacro* macro;
			} inData;
			memcpy(&inData, aData, sizeof(inData));
			memcpy(macroName, inData.name, PIN_STRING_MAX);
			macro = inData.macro;
			for (auto& inValue : inData.macro->inputValues)
			{
				inputPins.push_back(inValue);
				inputPins.back().ID = ID;
				inputPins.back().ID.idParts.pinID = (short)inputPins.size() - 1;
			}
			for (auto& ouValue : inData.macro->outputValues)
			{
				outputPins.push_back(ouValue);
				outputPins.back().ID = ID;
				outputPins.back().ID.idParts.pinID = (short)(inputPins.size() + outputPins.size()) - 1;
			}
		}
	}; RegisterNode<MacroExecuteNode>("Macro");


	// AI BehaviourTree Nodes
	class BehaviourTreeRootNode : public ScriptNode
	{
	private:
	public:
		DefineNode("Behaviour Root", NodeCategory::Flow, "Root node for a behaviour tree.");
		BehaviourTreeRootNode() : ScriptNode() {}

		void Init() override
		{
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "");
		}
		
	}; RegisterNode<BehaviourTreeRootNode>("AI/BehaviourTree");
	class BehaviourTreeCompositeNode : public ScriptNode
	{
	private:
		AI::TreeNodeType type = AI::TreeNodeType::COUNT;
	public:
		DefineNode("Composite", NodeCategory::AIFlow, "Sequencer node for a behaviour tree.");
		BehaviourTreeCompositeNode() : ScriptNode() {}

		void Init() override
		{
			AddInputPin(ValueType::FlowValue, PinType::Flow, "");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "");
		}
		virtual void ExtraSerialize(void* aOutJson, Script* aLoadingScript) override {
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			(*json)["type"] = static_cast<int>(type);
		}
		virtual void ExtraDeserialize(void* aOutJson, Script* aLoadingScript) override {
			nlohmann::json* json = (nlohmann::json*)aOutJson;

			if ((*json).find("type") == ((*json).end()))
				return;

			int value = (*json)["type"];
			type = static_cast<AI::TreeNodeType>(value);
		}

		void* GetCustomData() final override { return (void*)&type; }
		void CustomRender() override
		{
			ImGui::SetNextItemWidth(200.f);

			std::string infoLabel = type == AI::TreeNodeType::COUNT ? "Select" : AI::typeNames[(int)type];

			if (ImGui::BeginCombo("##", infoLabel.c_str()))
			{
				for (int i = 0; i < static_cast<int>(AI::TreeNodeType::FLOW_END); i++)
				{
					if (ImGui::Selectable(AI::typeNames[i].c_str()))
					{
						type = static_cast<AI::TreeNodeType>(i);
					}
				}
				ImGui::EndCombo();
			}
		}

	}; RegisterNode<BehaviourTreeCompositeNode>("AI/BehaviourTree");

	class BehaviourTreeLeafNode : public ScriptNode
	{
	private:
		AI::TreeNodeType type = AI::TreeNodeType::COUNT;

	public:
		DefineNode("Leaf", NodeCategory::AILeaf, "Leaf Node");
		BehaviourTreeLeafNode() : ScriptNode() {}

		virtual void ExtraSerialize(void* aOutJson, Script* aLoadingScript) override {
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			(*json)["type"] = static_cast<int>(type); //name.c_str();
		}
		virtual void ExtraDeserialize(void* aOutJson, Script* aLoadingScript) override {
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			int value = (*json)["type"];
			type = static_cast<AI::TreeNodeType>(value);
		}
		void* GetCustomData() final override { return (void*)&type; }

		
		void CustomRender() override
		{
			ImGui::SetNextItemWidth(200.f);

			std::string infoLabel = type == AI::TreeNodeType::COUNT ? "Select" : AI::typeNames[(int)type];

			if (ImGui::BeginCombo("##", infoLabel.c_str()))
			{
				for (int i = static_cast<int>(AI::TreeNodeType::LEAF_START) + 1; i < static_cast<int>(AI::TreeNodeType::LEAF_END); i++)
				{
					if (ImGui::Selectable(AI::typeNames[i].c_str()))
					{
						type = static_cast<AI::TreeNodeType>(i);
					}
				}
				ImGui::EndCombo();
			}
		}

		void Init() override
		{
			AddInputPin(ValueType::FlowValue, PinType::Flow, "");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "");
		}
	}; RegisterNode<BehaviourTreeLeafNode>("AI/BehaviourTree");
	class BehaviourTreeDecoratorNode : public ScriptNode
	{
	private:
		AI::TreeNodeType type = AI::TreeNodeType::COUNT;

	public:
		DefineNode("AI Decorator", NodeCategory::AIDecorator, "AI Decorator Node");
		BehaviourTreeDecoratorNode() : ScriptNode() {}

		void Init() override
		{
			AddInputPin(ValueType::FlowValue, PinType::Flow, "");
			AddOutputPin(ValueType::FlowValue, PinType::Flow, "");
		}
		virtual void ExtraSerialize(void* aOutJson, Script* aLoadingScript) override {
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			(*json)["type"] = static_cast<int>(type);
		}
		virtual void ExtraDeserialize(void* aOutJson, Script* aLoadingScript) override {
			nlohmann::json* json = (nlohmann::json*)aOutJson;
			int value = (*json)["type"];
			type = static_cast<AI::TreeNodeType>(value);
		}
		void* GetCustomData() final override { return (void*)&type; }
		void CustomRender() override
		{
			ImGui::SetNextItemWidth(125.f);

			std::string infoLabel = type == AI::TreeNodeType::COUNT ? "Select" : AI::typeNames[(int)type];

			if (ImGui::BeginCombo("##", infoLabel.c_str()))
			{
				for (int i = static_cast<int>(AI::TreeNodeType::DECORATOR_START) + 1; i < static_cast<int>(AI::TreeNodeType::DECORATOR_END); i++)
				{
					if (ImGui::Selectable(AI::typeNames[i].c_str()))
					{
						type = static_cast<AI::TreeNodeType>(i);
					}
				}
				ImGui::EndCombo();
			}
		}
	}; RegisterNode<BehaviourTreeDecoratorNode>("AI/BehaviourTree");


	////CODE NODES!

	//class EntryPointScopeNode : public CodeScriptNode
	//{
	//private:
	//public:
	//	DefineNode("Entrypoint Scope", NodeCategory::CodeEntryPoint, "Creates a scope for code.");

	//	virtual void Init() override
	//	{
	//		AddOutputPin(ValueType::FlowValue, PinType::Flow, "Out");
	//		AddOutputPin(ValueType::FlowValue, PinType::Flow, "Out 2");
	//	}

	//	const char* GetCode() const override
	//	{
	//		return "";
	//	}

	//	CommandList GetCommands(const LanguageDefinition& aLanguage) override
	//	{
	//		CommandList commands;

	//		//setup commands, this needs to be done here since our pin IDS are not yet set at Init. sucks!
	//		{
	//			CreateScopeCommand* command = commands.Add<CreateScopeCommand>();
	//			command->scopeOwnerID = outputPins[0].ID;
	//			command->scopeStart = "void main() \n{\n";
	//			command->scopeEnd = "}\n";
	//		}
	//		{
	//			CreateScopeCommand* command = commands.Add<CreateScopeCommand>();
	//			command->scopeOwnerID = outputPins[1].ID;
	//			command->scopeStart = "void notMain() \n{\n";
	//			command->scopeEnd = "}\n";
	//		}

	//		return commands;
	//	}

	//}; RegisterNode<EntryPointScopeNode>("Code");

	//class ScopeBranchNode : public CodeScriptNode
	//{
	//private:
	//public:
	//	DefineNode("Scope Branch", NodeCategory::CodeLogic, "Creates a scope for code.");

	//	virtual void Init() override
	//	{
	//		AddInputPin( ValueType::FlowValue, PinType::Flow, "");
	//		AddOutputPin(ValueType::FlowValue, PinType::Flow, "Branch 0");
	//		AddOutputPin(ValueType::FlowValue, PinType::Flow, "Branch 1");
	//	}

	//	const char* GetCode() const override
	//	{
	//		return "";
	//	}

	//	CommandList GetCommands(const LanguageDefinition& aLanguage) override
	//	{
	//		CommandList commands;

	//		//setup commands, this needs to be done here since our pin IDS are not yet set at Init. sucks!
	//		{
	//			CreateScopeCommand* command = commands.Add<CreateScopeCommand>();
	//			command->scopeOwnerID = outputPins[0].ID;
	//			command->scopeStart = "void branch0() \n{\n";
	//			command->scopeEnd = "}\n";
	//		}
	//		{
	//			CreateScopeCommand* command = commands.Add<CreateScopeCommand>();
	//			command->scopeOwnerID = outputPins[1].ID;
	//			command->scopeStart = "void branch1() \n{\n";
	//			command->scopeEnd = "}\n";
	//		}

	//		return commands;
	//	}

	//}; RegisterNode<ScopeBranchNode>("Code");

	//class NewFloatConstantNodeTest : public CodeScriptNode
	//{
	//private:
	//public:
	//	DefineNode("New float constant", NodeCategory::CodeMath, "Defines a float constant using new system.");

	//	NewFloatConstantNodeTest() : CodeScriptNode() {}

	//	virtual void Init() override
	//	{
	//		AddInputPin(ValueType::CodeValue, PinType::Value, "Input 0");
	//		AddInputPin(ValueType::CodeValue, PinType::Value, "Input 1");
	//		AddInputPin(ValueType::CodeValue, PinType::Value, "Input 2");
	//		AddInputPin(ValueType::CodeValue, PinType::Value, "Input 3");

	//		AddOutputPin(ValueType::CodeValue, PinType::OutputValue, "-> 0");
	//		AddOutputPin(ValueType::CodeValue, PinType::OutputValue, "-> 0+1");
	//		AddOutputPin(ValueType::CodeValue, PinType::OutputValue, "-> 0*1");
	//		AddOutputPin(ValueType::CodeValue, PinType::OutputValue, "-> 0/1+2*3");
	//	}

	//	const char* GetCode() const override
	//	{
	//		return "";
	//	}

	//	CommandList GetCommands(const LanguageDefinition& aLanguage) override
	//	{
	//		CommandList commands;

	//		//setup commands, this needs to be done here since our pin IDS are not yet set at Init. sucks!
	//		{
	//			//no assigment inputs
	//			GenerateVariableCommand* command = commands.Add<GenerateVariableCommand>();
	//			command->origin = outputPins[0].ID;
	//			command->type = aLanguage.dataTypes.at("float");// = CodeVariableType::Float;
	//			command->mutability = CodeVariableMutability::Constant;
	//			command->data.assignmentInputs.push_back({ inputPins[0].ID, CodeOperation::None });
	//		}

	//		{
	//			//input 0 + input 1
	//			GenerateVariableCommand* command = commands.Add<GenerateVariableCommand>();
	//			command->origin = outputPins[1].ID;
	//			command->type = aLanguage.dataTypes.at("float");// = CodeVariableType::Float;
	//			command->mutability = CodeVariableMutability::Constant;
	//			command->data.assignmentInputs.push_back({ inputPins[0].ID, CodeOperation::Add });
	//			command->data.assignmentInputs.push_back({ inputPins[1].ID, CodeOperation::None });
	//		}

	//		{
	//			//input 0 * input 1
	//			GenerateVariableCommand* command = commands.Add<GenerateVariableCommand>();
	//			command->origin = outputPins[2].ID;
	//			command->type = aLanguage.dataTypes.at("float");// = CodeVariableType::Float;
	//			command->mutability = CodeVariableMutability::Constant;
	//			command->data.assignmentInputs.push_back({ inputPins[0].ID, CodeOperation::Multiply });
	//			command->data.assignmentInputs.push_back({ inputPins[1].ID, CodeOperation::None });
	//		}

	//		{
	//			//input 0 / input 1 + input 2 * input 3
	//			GenerateVariableCommand* command = commands.Add<GenerateVariableCommand>();
	//			command->origin = outputPins[3].ID;
	//			command->type = aLanguage.dataTypes.at("float");// = CodeVariableType::Float;
	//			command->mutability = CodeVariableMutability::Constant;
	//			command->data.assignmentInputs.push_back({ inputPins[0].ID, CodeOperation::Divide });
	//			command->data.assignmentInputs.push_back({ inputPins[1].ID, CodeOperation::Add });
	//			command->data.assignmentInputs.push_back({ inputPins[2].ID, CodeOperation::Multiply });
	//			command->data.assignmentInputs.push_back({ inputPins[3].ID, CodeOperation::None });
	//		}

	//		return commands;
	//	}

	//}; RegisterNode<NewFloatConstantNodeTest>("Code");

	//class EmptyDependencyNode : public CodeScriptNode
	//{
	//private:

	//public:
	//	DefineNode("Empty Dependency", NodeCategory::CodeLogic, "Does nothing. Useful for testing");

	//	virtual void Init() override
	//	{
	//		AddInputPin(ValueType::FlowValue, PinType::Flow, "Flow In");
	//		AddInputPin(ValueType::CodeValue, PinType::Value, "Value In");
	//		AddOutputPin(ValueType::FlowValue, PinType::Flow, "Flow Out");
	//		AddOutputPin(ValueType::CodeValue, PinType::Value, "Value Out");
	//	}

	//	const char* GetCode() const override
	//	{
	//		return "";
	//	}

	//	CommandList GetCommands(const LanguageDefinition& aLanguage) override
	//	{
	//		CommandList commands;

	//		{
	//			GenerateVariableCommand* command = commands.Add<GenerateVariableCommand>();
	//			command->origin = outputPins[1].ID;
	//			command->type = aLanguage.dataTypes.at("float"); //= CodeVariableType::Float;
	//			command->mutability = CodeVariableMutability::Constant;
	//			command->data.assignmentInputs.push_back({ inputPins[1].ID, CodeOperation::None });
	//		}

	//		return commands;
	//	}
	//
	//}; RegisterNode<EmptyDependencyNode>("Code");

	//class CodeEntryNode : public CodeScriptNode
	//{
	//private:

	//public:
	//	virtual const char* GetName() const override { return "Entry"; }
	//	virtual NodeCategory GetCategory() const override { return NodeCategory::CodeEntryPoint; }
	//	virtual const char* GetDescription() const override { return "Entry point for the script"; }

	//	virtual const char* GetCode() const override { return ""; }

	//	virtual void Init() override
	//	{
	//		AddOutputPin(ValueType::FlowValue, PinType::Flow, "Start");
	//	}

	//	virtual CommandList GetCommands(const LanguageDefinition& aLanguage) override
	//	{
	//		CommandList commands;

	//		CreateScopeCommand* scope = commands.Add<CreateScopeCommand>();
	//		scope->scopeOwnerID = outputPins[0].ID;
	//		scope->scopeStart = "float4 main()\n{\n";
	//		scope->scopeEnd = "}\n";

	//		return commands;
	//	}
	//}; RegisterNode<CodeEntryNode>("Code");

	//class CodeExitNode : public CodeScriptNode
	//{
	//private:
	//	std::vector<CodePin> codePinData;

	//public:
	//	virtual const char* GetName() const override { return "Exit"; }
	//	virtual NodeCategory GetCategory() const override { return NodeCategory::CodeLogic; }
	//	virtual const char* GetDescription() const override { return "Exit point for the script"; }

	//	virtual const char* GetCode() const override { return ""; }

	//	virtual void Init() override
	//	{
	//		AddInputPin(ValueType::FlowValue, PinType::Flow, "End");
	//		AddInputPin(ValueType::CodeValue, PinType::Value, "Output");

	//		codePinData.push_back({ CodePinType::Value, parentScript->GetLanguageDefinition()->dataTypes.at("float4") });
	//		inputPins[1].codeData = &codePinData.back();
	//	}

	//	virtual CommandList GetCommands(const LanguageDefinition& aLanguage) override
	//	{
	//		CommandList commands;

	//		ReturnValueCommand* returnVal = commands.Add<ReturnValueCommand>();
	//		returnVal->origin = inputPins[1].ID;

	//		return commands;
	//	}
	//}; RegisterNode<CodeExitNode>("Code");

}

KE::NodeTypeDatabase::~NodeTypeDatabase()
{
	for (auto& language : myLanguageDefinitions)
	{
		delete language.second;
	}
}

void KE::NodeTypeDatabase::RegisterLanguage(const std::string& aLanguageFilePath)
{

	auto languageDef = LanguageDefinerNew<HLSLDefiner>::Define(aLanguageFilePath.c_str());
	myLanguageDefinitions["hlsl"] = new HLSLDefiner::LangDefinition(languageDef);

	for (const auto& function : languageDef.functions)
	{
		RegisterNode(
			function.second.name.c_str(),
			NodeCategory::CodeLogic,
			"",
			[function, languageDef](Script* aScript) -> ScriptNode*
			{
				CodeFunctionNode* node = new CodeFunctionNode();
				/*node->SetVariantIndex(0);*/
				node->SetVariantCount(static_cast<unsigned int>(function.second.variants.size()));
				node->AddData(function.second);
				node->AssignScript(aScript);
				return node;
			},
			"HLSL/Functions"
		);
	}

	for (const auto& dataStruct : languageDef.structs)
	{
		RegisterNode(
			dataStruct.second.name.c_str(),
			NodeCategory::CodeLogic,
			"",
			[dataStruct, languageDef](Script* aScript) -> ScriptNode*
			{
				CodeStructNode* node = new CodeStructNode();
				node->AddData(dataStruct.second);
				node->AssignScript(aScript);
				return node;
			},
			"HLSL/Structs"
		);
	}

	for (const auto& buffer : languageDef.buffers)
	{
		RegisterNode(
			buffer.second.name.c_str(),
			NodeCategory::CodeLogic,
			"",
			[buffer, languageDef](Script* aScript) -> ScriptNode*
			{
				CodeBufferNode* node = new CodeBufferNode();
				node->AddData(buffer.second);
				node->AssignScript(aScript);
				return node;
			},
			"HLSL/Buffers"
		);
	}

	for (const auto& texture : languageDef.textures)
	{
		RegisterNode(
			texture.second.name.c_str(),
			NodeCategory::CodeLogic,
			"",
			[texture, languageDef](Script* aScript) -> ScriptNode*
			{
				CodeTextureNode* node = new CodeTextureNode();
				node->AddData(texture.second);
				node->AssignScript(aScript);
				return node;
			},
			"HLSL/Textures"
		);
	}

	for (const auto& entryPoint : languageDef.entrypoints)
	{
		std::string entryPointName = entryPoint.second.name;
		std::string inName = "Enter " + entryPointName;
		std::string outName = "Exit " + entryPointName;


		RegisterNode(
			inName.c_str(),
			NodeCategory::CodeLogic,
			"",
			[entryPoint, languageDef](Script* aScript) -> ScriptNode*
			{
				CodeEntryPointNode* node = new CodeEntryPointNode();
				node->AddData(entryPoint.second);
				node->AssignScript(aScript);
				return node;
			},
			"HLSL/Entrypoints"
		);

		RegisterNode(
			outName.c_str(),
			NodeCategory::CodeLogic,
			"",
			[entryPoint, languageDef](Script* aScript) -> ScriptNode*
			{
				CodeExitPointNode* node = new CodeExitPointNode();
				node->AddData(entryPoint.second);
				node->AssignScript(aScript);
				return node;
			},
			"HLSL/Exitpoints"
		);
	}

}

void KE::NodeTypeDatabase::CalculateNodeCategoryStack(const std::string& aNodeName, const char* aCategoryStack)
{
	std::string categoryStack = aCategoryStack;
	std::string category = "";
	for (int i = 0; i < categoryStack.size(); i++)
	{
		if (categoryStack[i] == '/')
		{
			if (category.size() > 0)
			{
				nodeCategoryStacks[aNodeName].names.push_back(category);
				category = "";
			}
		}
		else
		{
			category += categoryStack[i];
		}
	}
	if (category.size() > 0)
	{
		nodeCategoryStacks[aNodeName].names.push_back(category);
	}
}

void KE::NodeTypeDatabase::ProcessNodeData(const std::string& aNodeName)
{
	Script tempScript;
	tempScript.SetLanguageDefinition(myLanguageDefinitions["hlsl"]);

	KE::ScriptNode* node = nodeTypes[aNodeName].createFunction(&tempScript);
	node->Init();

	auto* variant = dynamic_cast<IVariant*>(node);
	unsigned int iterCount = variant ? variant->GetVariantCount() : 1;

	for (unsigned int variantIter = 0; variantIter < iterCount; variantIter++)
	{
		if (variant) { variant->SetVariantIndex(variantIter); }

		for (int i = 0; i < node->GetInputPins().size(); i++)
		{
			KE::Pin* pin = &node->GetInputPins()[i];

			NodePinDatabaseValue v{ i, "", "", &nodeTypes[aNodeName], variantIter };
			strcpy_s(v.pinName, pin->name);
			strcpy_s(v.nodeName, aNodeName.c_str());
			if (pin->codeData)
			{
				nodePinCodeInDatabase[((CodePin*)pin->codeData)->value.type.typeName].push_back(v);
			}
			else
			{
				nodeInPinDatabase[pin->value.type].push_back(v);
			}
		}

		for (int i = 0; i < node->GetOutputPins().size(); i++)
		{
			KE::Pin* pin = &node->GetOutputPins()[i];

			NodePinDatabaseValue v{ i, "", "", &nodeTypes[aNodeName], variantIter };
			strcpy_s(v.pinName, pin->name);
			strcpy_s(v.nodeName, aNodeName.c_str());
			
			if (pin->codeData)
			{
				nodePinCodeOutDatabase[((CodePin*)pin->codeData)->value.type.typeName].push_back(v);
			}
			else
			{
				nodeOutPinDatabase[pin->value.type].push_back(v);
			}
		}	
	}

	delete node;
}

void KE::NodeTypeDatabase::RegisterNode(const char* aNodeName, NodeCategory aCategory, const char* aDescription, const NodeCreateFunction& aCreateFunction, const char* aCategoryStack)
{
	nodeTypes[aNodeName] = { aNodeName, aCategory, aDescription, aCreateFunction };
	ProcessNodeData(aNodeName);
	CalculateNodeCategoryStack(aNodeName, aCategoryStack);
	nodeNameMap[aNodeName] = aNodeName;
}

