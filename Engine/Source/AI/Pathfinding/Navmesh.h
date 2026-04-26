#pragma once
#include "NavNode.h"
#include "NavGrid.h"


namespace KE
{
	class Graphics;
	class KittyMesh;
	struct Mesh;

	struct NodesAttachedToIndex
	{
		NodesAttachedToIndex(int aIndex) { index = aIndex; };
		NodesAttachedToIndex(int aIndex, Vector3f aVertex, NavNode& aNode)
		{
			index = aIndex; vertex = aVertex; nodes.push_back(&aNode);
		};

		int index = INT_MIN;
		Vector3f vertex = { FLT_MIN, FLT_MIN, FLT_MIN };
		std::vector<NavNode*> nodes = {};
	};
	struct SharedLine
	{
		SharedLine(int v0, int v1) : indices({ v0, v1 }) {};

		bool IsShared(int aV0, int aV1)
		{
			if ((indices[0] == aV0 && indices[1] == aV1) || 
				(indices[1] == aV0 && indices[0] == aV1))
			{
				isShared = true; 
			}

			return isShared;
		}

		std::array<int, 2> indices;
		bool isShared = false;
	};
	struct EdgeLine
	{
		EdgeLine() {};
		EdgeLine(Vector3f aV0, Vector3f aV1) : v0(aV0), v1(aV1) {};

		Vector3f v0;
		Vector3f v1;
	};

	class Navmesh
	{
		friend class CollisionHandler;
		friend class Pathfinder;

		KE_EDITOR_FRIEND

	public:
		Navmesh() = default;
		~Navmesh() {}

		bool Init(Graphics* aGraphics, std::string& aObjFilepath);
		void DebugRender(Graphics* aGraphics);

		void LoadKittyMesh(KE::KittyMesh& aFile);
		void SaveKittyMesh(KE::KittyMesh& aFile);
		void ClearNavmesh();

		inline std::vector<NavNode*> GetNodesFromPos(Vector3f aPos);

	private:
		void CreateNodes(const Mesh& aMesh);
		void CreateNodes(const KE::KittyMesh& aMesh, Vector3f& aMin, Vector3f& aMax);
		void AttachNodes(NavNode* aNode);
		void GenerateLineColliders(NavNode* aNode);
		bool GotIndices(std::array<unsigned int, 2> aLine, NavNode& aNode);

		std::vector<NavNode> myNodes;
		std::vector<Vector3f> myVertices = {};
		std::vector<unsigned int> myIndices = {};
		std::unordered_map<int, NodesAttachedToIndex> myIndiceNodeMap;
		std::vector<EdgeLine> myEdges;
		NavGrid myNavGrid;
	};


	inline std::vector<NavNode*> Navmesh::GetNodesFromPos(Vector3f aPos)
	{
		return myNavGrid.GetNodesByPos(aPos);
	}
}