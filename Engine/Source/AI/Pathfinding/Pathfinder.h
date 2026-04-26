#pragma once
#include <Engine/Source/AI/Pathfinding/NavNode.h>


namespace KE
{
	class Navmesh;
	class Graphics; 

	class Pathfinder
	{
#pragma region InternalData
	private:
		struct FunnelLocation
		{
			FunnelLocation() {};
			FunnelLocation(Vector3f aVertex, int aIndex) : position(aVertex), index(aIndex) {};
			Vector3f position;
			int index = INT_MIN;
			int neighbourIndex = INT_MIN;
		};
		struct Portal
		{
			std::array<Vector3f, 2> vertices;
			std::array<int, 2> indices = { INT_MIN, INT_MIN };
		};
		struct Apex
		{
			int index = INT_MIN;
			Vector3f position;
		};
		struct FunnelData
		{
			float angle = 0;
			Apex apex;
			FunnelLocation left;
			FunnelLocation right;
			std::vector<FunnelLocation> leftTargets;
			std::vector<FunnelLocation> leftSaved;
			std::vector<FunnelLocation> rightTargets;
			std::vector<FunnelLocation> rightSaved;
		};
#pragma endregion

	public:
		Pathfinder() = default;
		bool Init(Navmesh& aNavmesh);
		void DebugRender(Graphics* aGraphics);

		std::vector<Vector3f> FindPath(Vector3f aStart, Vector3f aEnd);
		std::vector<Vector2f> FindPath_2D(Vector3f aStart, Vector3f aEnd, NavNode* aStartNode = nullptr);

		NavNode* GetNode(const Vector3f& aPosition);
		const float GetHeightByPos(const Vector3f& aPoint);
		Vector3f GetNearestNavmeshPos(const Vector3f& aTargetPos);
	
	private:
		// <[ 2D Version ]> //
		std::vector<Vector2f> CreatePath_2D(FunnelData& aPackage, const Vector3f& aEnd);
		void Funneling_2D(FunnelData& aAngle, std::vector<Vector2f>& aPath);

		// <[ 3D Version ]> //
		std::vector<Vector3f> CreatePath(FunnelData& aData, std::vector<Portal>& aPortalsToCross, Vector3f& aTarget);
		void Funneling(FunnelData& aData, std::vector<Vector3f>& aPath, std::vector<Portal>& aPortals);
		void RemoveExcessPortals(std::vector<Portal>& aTraversedPortals, const Apex& aApex);
		void AddPortalCrossings(const FunnelData& aData, const FunnelLocation& aSide, std::vector<Portal>& aTraversedPortals, std::vector<Vector3f>& aPath);
		void AddEndCrossings(const FunnelData& aData, const FunnelLocation& aSide, std::vector<Portal>& aTraversedPortals, std::vector<Vector3f>& aPath);
		bool LineIntersect(Vector2f& aOutLeft, Vector2f& aOutRight, const Vector2f aStartPos, const Vector2f aLineDir);

		// <[ SETUP ]> //
		bool AStar(NavNode* aStartNode, NavNode* aEndNode, const Vector3f& aEnd);
		void ResetScore();
		void ScoreCell(NavNode& aFromNode, NavNode& aNodeToScore, const Vector3f& aTarget);
		void ScoreNeighbours(NavNode& aNode, std::priority_queue<NavNode>& aQueue, const Vector3f& aTarget);
		void CreateChannel(std::vector<NavNode*>& aChannel, NavNode* aStartNode, NavNode* aEndNode);
		void GeneratePortal(Portal& aOutPortal, const NavNode* aFromNode, const NavNode* aToNode);
		void CreateFunnelLocations(std::vector<NavNode*>& aChannel, FunnelData& aData, Vector3f& aTarget, std::vector<Portal>* aCrossingPortals = nullptr);
		bool ConstrainToNavmesh(Vector3f& aOutPos, int& aOutIndex);
		inline float GetManhattanDist(Vector3f aFrom, Vector3f aTo)
		{
			return (aFrom.x - aTo.x) + (aFrom.y - aTo.y) + (aFrom.z - aTo.z);
		}
		inline float GetEuclideanDist(Vector3f aFrom, Vector3f aTo)
		{
			return (aFrom - aTo).Length();
		}

		// <[ COMMON ]> //
		Vector3f GetSmoothedWaypoint(const FunnelLocation& aSide, const Apex& aApex, const float aSmoothFactor = 0.03f) const;
		float GetFunnelAngle(const FunnelLocation& aLeft, const FunnelLocation& aRight, const Apex& aApex) const;
		void CollectFromSaved(std::vector<FunnelLocation>& aSavedList, std::vector<FunnelLocation>& aTargetList, const int aIndexToFind, const int aIndexToAvoid);
		int GetNodeIndexFromPoint(const Vector3f aPoint);
		void AlignTargets(std::vector<FunnelLocation>& aListToReduce, const std::vector<FunnelLocation>& aCompareList);


		Navmesh* myNavmesh = nullptr;
		std::vector<NavNode>* myNodes = nullptr;

		//<[DEBUG]>//
		Vector2f myDebugStart_2D = {};
		Vector3f myDebugStart_3D = {};
		NavNode* myDebugStartNode = nullptr;
		std::vector<Vector2f> myDebugPath_2D = {};
		std::vector<Vector3f> myDebugPath_3D = {};
	};

}