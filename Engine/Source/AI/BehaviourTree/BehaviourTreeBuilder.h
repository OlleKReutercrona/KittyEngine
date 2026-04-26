#pragma once
#include "Engine/Source/AI/BehaviourTree/TreeNode.h"


namespace KE {
	class Script;
	class ScriptNode;
	struct ScriptMemberID;
	struct PinConnection;
}
namespace AI {
	class BehaviourTree;
}

namespace AI
{
	enum class EnemyType
	{
		eCOUNT
	};

	struct RecursiveData
	{
		RecursiveData(KE::Script* aScript);

		KE::Script* script = nullptr;
		std::unordered_map<KE::ScriptMemberID, KE::ScriptNode*, KE::ScriptMemberID>& scriptNodes;
		std::unordered_map<KE::ScriptMemberID, std::vector<KE::PinConnection>, KE::ScriptMemberID>& connections;
	};

	enum class TreeNodeType {
		Sequence,
		Selector,
		FLOW_END,

		LEAF_START,
		LEAF_END,

		DECORATOR_START,
		DECORATOR_END,

		COUNT,
	};

	inline std::string typeNames[(int)TreeNodeType::COUNT] = {
		"Sequence",
		"Selector",
		"FLOW_END",

		"LEAF_START",
		"LEAF_END",

		"DECORATOR_START",
		"DECORATOR_END",
	};

	class BehaviourTreeBuilder
	{
	public:
		static AI::BehaviourTree* GenerateBT(const EnemyType aType);

	private:
		static bool RecursiveAttach(KE::ScriptMemberID& aFromNodeID, AI::TreeNode* aTreeNode, RecursiveData& aData);
		static TreeNodeType EvaluateEnum(KE::ScriptNode*);

		static KE::ScriptNode* GetEditorRootNode(AI::RecursiveData& aData, std::string aRootName);
		static KE::Script* GetEnemyScript(const EnemyType aType);
		static TreeNode* GetTreeNode(TreeNodeType aType);
	};

}