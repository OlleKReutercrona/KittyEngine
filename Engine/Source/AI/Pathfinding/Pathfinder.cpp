#include "stdafx.h"
#include <Engine/Source/AI/Pathfinding/Navmesh.h>
#include <Engine/Source/AI/Pathfinding/Pathfinder.h>
#include <Engine/Source/Collision/Intersection.h>


// Only used for DebugRender
#include <Engine/Source/Graphics/Graphics.h> 



namespace KE
{
	bool Pathfinder::Init(Navmesh& aNavmesh)
	{
		myNavmesh = &aNavmesh;
		if (aNavmesh.myNodes.size() < 1)
		{
			//KE_ERROR("Pathfinder recieved 0 valid nodes from Navmesh");
			return false;
		}

		myNodes = &aNavmesh.myNodes;
		return true;
	}
	void Pathfinder::DebugRender(Graphics* aGraphics)
	{
		float lineHeight = 0.0f;
		LineVertex v1;
		LineVertex v2;

		// <<[Render Path]>> //
		v1.colour = { 1,0,0,1 };
		v2.colour = { 1,0,0,1 };
		if (myDebugPath_2D.size() > 0)
		{
			v1.position = { myDebugStart_2D.x, lineHeight, myDebugStart_2D.y, 1.0f };
			v2.position = { myDebugPath_2D.back().x, lineHeight, myDebugPath_2D.back().y, 1.0f };
			aGraphics->AddLine(v1, v2);

			for (int i = 1; i < myDebugPath_2D.size(); i++)
			{
				v1.position = { myDebugPath_2D[i].x, lineHeight, myDebugPath_2D[i].y, 1.0f };
				v2.position = { myDebugPath_2D[i - 1].x, lineHeight, myDebugPath_2D[i - 1].y, 1.0f };
				aGraphics->AddLine(v1, v2);
			}
		}

		if (myDebugPath_3D.size() > 0)
		{
			v1.position = { myDebugStart_3D.x, myDebugStart_3D.y, myDebugStart_3D.z, 1.0f };
			v2.position = { myDebugPath_3D.back().x,  myDebugPath_3D.back().y, myDebugPath_3D.back().z, 1.0f };
			aGraphics->AddLine(v1, v2);

			for (int i = 1; i < myDebugPath_3D.size(); i++)
			{
				v1.colour = { 1,0,0,1 };
				v2.colour = { 1,0,0,1 };
				v1.position = { myDebugPath_3D[i].x,     myDebugPath_3D[i].y,     myDebugPath_3D[i].z    , 1.0f };
				v2.position = { myDebugPath_3D[i - 1].x, myDebugPath_3D[i - 1].y, myDebugPath_3D[i - 1].z, 1.0f };
				aGraphics->AddLine(v1, v2);

				v1.colour = { 1,0,1,1 };
				v2.colour = { 1,0,1,1 };
				v1.position = { myDebugPath_3D[i].x,     myDebugPath_3D[i].y,     myDebugPath_3D[i].z    , 1.0f };
				v2.position = { myDebugPath_3D[i].x,     myDebugPath_3D[i].y + 0.5f,     myDebugPath_3D[i].z    , 1.0f };
				aGraphics->AddLine(v1, v2);
			}
		}

		v1.colour = { 1,1,0,1 };
		v2.colour = { 1,1,0,1 };
	}

	// <[ 2D Version ]> //
	std::vector<Vector2f> Pathfinder::FindPath_2D(Vector3f aStart, Vector3f aEnd, NavNode* aStartNode)
	{
		std::vector<Vector2f> empty = {};

		if (myNodes == nullptr || myNodes->size() == 0) {
			KE_ERROR("Pathfinding Error: No valid nodes");
			return empty;
		}
		
		int startIndex = aStartNode ? aStartNode->index : GetNodeIndexFromPoint(aEnd);
		int endIndex = GetNodeIndexFromPoint(aStart);

		// If either location is outside the Navmesh, try to get nearest point on Navmesh.
		if (!ConstrainToNavmesh(aStart, endIndex)) { return empty; }
		if (!ConstrainToNavmesh(aEnd, startIndex)) { return empty; }

		if (startIndex == endIndex)
		{
			empty.emplace_back(aEnd.x, aEnd.z);
			return empty;
		}

		NavNode* startNode = &myNodes->at(startIndex);
		NavNode* endNode = &myNodes->at(endIndex);

		myDebugStartNode = startNode;
		myDebugStart_2D = { aStart.x, aStart.z };

		FunnelData data;
		data.apex.position = aEnd;
		std::vector<NavNode*> channel;
		
		if (!AStar(startNode, endNode, aEnd)) { return empty; }
		CreateChannel(channel, startNode, endNode);
		CreateFunnelLocations(channel, data, aStart);

		return CreatePath_2D(data, aStart);
	}
	std::vector<Vector2f> Pathfinder::CreatePath_2D(FunnelData& aData, const Vector3f& aEnd)
	{
		std::vector<Vector2f> path = {};

		path.emplace_back(aData.apex.position.x, aData.apex.position.z);

		aData.left = aData.leftTargets.back();
		aData.right = aData.rightTargets.back();

		aData.leftSaved.emplace_back(aData.leftTargets.back());
		aData.rightSaved.emplace_back(aData.rightTargets.back());

		aData.leftTargets.pop_back();
		aData.rightTargets.pop_back();

		aData.angle = GetFunnelAngle(aData.left, aData.right, aData.apex);
		int targetCount = static_cast<int>(aData.leftTargets.size() + aData.rightTargets.size());

		while (targetCount > 0)
		{
			Funneling_2D(aData, path);

			targetCount = static_cast<int>(aData.leftTargets.size() + aData.rightTargets.size());
		}

		//path.emplace_back(aEnd.x, aEnd.z);
		myDebugPath_2D = path;

		return path;
	}
	void Pathfinder::Funneling_2D(FunnelData& aData, std::vector<Vector2f>& aPath)
	{
		FunnelLocation tempLeft = aData.left;
		FunnelLocation tempRight = aData.right;

		if (aData.leftTargets.size() > 0) {
			tempLeft = aData.leftTargets.back();
			aData.leftSaved.emplace_back(aData.leftTargets.back());
			aData.leftTargets.pop_back();
		}
		if (aData.rightTargets.size() > 0) {
			tempRight = aData.rightTargets.back();
			aData.rightSaved.emplace_back(aData.rightTargets.back());
			aData.rightTargets.pop_back();
		}

		if (aData.left.index != tempLeft.index)
		{
			float angle = GetFunnelAngle(tempLeft, aData.right, aData.apex);

			if (angle < 0)
			{
				Vector3f smoothedApex = GetSmoothedWaypoint(aData.right, aData.apex);
				aPath.emplace_back(smoothedApex.x, smoothedApex.z);

				aData.apex.index = aData.right.index;
				aData.apex.position = aData.right.position;
				int apexNeighbour = aData.right.neighbourIndex;

				CollectFromSaved(aData.rightSaved, aData.rightTargets, aData.right.index, aData.apex.index);
				CollectFromSaved(aData.leftSaved, aData.leftTargets, apexNeighbour, aData.apex.index);

				AlignTargets(aData.leftTargets, aData.rightTargets);

				if (aData.rightTargets.size() > 0) { aData.right = aData.rightTargets.back(); }
				if (aData.leftTargets.size() > 0) { aData.left = aData.leftTargets.back(); }

				aData.angle = GetFunnelAngle(aData.left, aData.right, aData.apex);

				return;
			}
			if (angle <= aData.angle)
			{
				aData.angle = angle;
				aData.left = tempLeft;
			}
		}

		if (aData.right.index != tempRight.index)
		{
			float angle = GetFunnelAngle(aData.left, tempRight, aData.apex);

			if (angle < 0)
			{
				Vector3f smoothedApex = GetSmoothedWaypoint(aData.left, aData.apex);
				aPath.emplace_back(smoothedApex.x, smoothedApex.z);

				aData.apex.index = aData.left.index;
				aData.apex.position = aData.left.position;
				int apexNeighbour = aData.left.neighbourIndex;

				CollectFromSaved(aData.leftSaved, aData.leftTargets, aData.left.index, aData.apex.index);
				CollectFromSaved(aData.rightSaved, aData.rightTargets, apexNeighbour, aData.apex.index);

				AlignTargets(aData.rightTargets, aData.leftTargets);

				if (aData.leftTargets.size() > 0) {
					aData.left = aData.leftTargets.back();
				}
				if (aData.rightTargets.size() > 0) {
					aData.right = aData.rightTargets.back();
				}

				aData.angle = GetFunnelAngle(aData.left, aData.right, aData.apex);

				return;
			}
			if (angle <= aData.angle)
			{
				aData.angle = angle;
				aData.right = tempRight;
			}
		}
	}


	// <[ 3D Version ]> //
	std::vector<Vector3f> Pathfinder::FindPath(Vector3f aStart, Vector3f aEnd)
	{
		std::vector<Vector3f> empty = {};

		if (myNodes == nullptr || myNodes->size() == 0) {
			KE_ERROR("Pathfinding Error: No valid nodes");
			return empty;
		}

		int startIndex = GetNodeIndexFromPoint(aEnd);
		int endIndex = GetNodeIndexFromPoint(aStart);

		// If either location is outside the Navmesh, try to get nearest point on Navmesh.
		if (!ConstrainToNavmesh(aStart, endIndex)) { return empty; }
		if (!ConstrainToNavmesh(aEnd, startIndex)) { return empty; }

		if (startIndex == endIndex)
		{
			empty.emplace_back(aEnd);
			return empty;
		}

		NavNode* startNode = &myNodes->at(startIndex);
		NavNode* endNode = &myNodes->at(endIndex);

		myDebugStartNode = startNode;
		myDebugStart_3D = aStart;

		FunnelData data;
		data.apex.position = aEnd;
		data.apex.index = INT_MIN;
		std::vector<NavNode*> channel;
		std::vector<Portal> crossingPortals;

		if (!AStar(startNode, endNode, aEnd)) { return empty; }
		CreateChannel(channel, startNode, endNode);
		CreateFunnelLocations(channel, data, aStart, &crossingPortals);

		return CreatePath(data, crossingPortals, aStart);
	}
	std::vector<Vector3f> Pathfinder::CreatePath(FunnelData& aData, std::vector<Portal>& aPortalsToCross, Vector3f& aTarget)
	{
		std::vector<Vector3f> path = {};
		path.emplace_back(aData.apex.position);

		aData.left = aData.leftTargets.back();
		aData.right = aData.rightTargets.back();

		aData.leftSaved.emplace_back(aData.leftTargets.back());
		aData.rightSaved.emplace_back(aData.rightTargets.back());

		aData.leftTargets.pop_back();
		aData.rightTargets.pop_back();

		aData.angle = GetFunnelAngle(aData.left, aData.right, aData.apex);

		int targetCount = (int)aData.leftTargets.size() + (int)aData.rightTargets.size();
		while (targetCount > 0)
		{
			Funneling(aData, path, aPortalsToCross);

			targetCount = (int)aData.leftTargets.size() + (int)aData.rightTargets.size();
		}

		FunnelLocation target(aTarget, -1);
		AddEndCrossings(aData, target, aPortalsToCross, path);

		path.emplace_back(aTarget);
		myDebugPath_3D = path;

		return path;
	}
	void Pathfinder::Funneling(FunnelData& aData, std::vector<Vector3f>& aPath, std::vector<Portal>& aPortals)
	{
		FunnelLocation tempLeft = aData.left;
		FunnelLocation tempRight = aData.right;

		if (aData.leftTargets.size() > 0) {
			tempLeft = aData.leftTargets.back();
			aData.leftSaved.emplace_back(aData.leftTargets.back());
			aData.leftTargets.pop_back();
		}
		if (aData.rightTargets.size() > 0) {
			tempRight = aData.rightTargets.back();
			aData.rightSaved.emplace_back(aData.rightTargets.back());
			aData.rightTargets.pop_back();
		}

		if (tempLeft.index != aData.left.index)
		{
			float angle = GetFunnelAngle(tempLeft, aData.right, aData.apex);

			if (angle < 0)
			{
				AddPortalCrossings(aData, aData.right, aPortals, aPath);

				Vector3f smoothedApex = GetSmoothedWaypoint(aData.right, aData.apex);
				aPath.emplace_back(smoothedApex);

				aData.apex.index = aData.right.index;
				aData.apex.position = aData.right.position;
				int apexNeighbour = aData.right.neighbourIndex;

				RemoveExcessPortals(aPortals, aData.apex);

				CollectFromSaved(aData.rightSaved, aData.rightTargets, aData.apex.index, aData.right.index);
				CollectFromSaved(aData.leftSaved, aData.leftTargets, apexNeighbour, aData.apex.index);

				AlignTargets(aData.leftTargets, aData.rightTargets);

				if (aData.rightTargets.size() > 0) {
					aData.right = aData.rightTargets.back();
				}
				if (aData.leftTargets.size() > 0) {
					aData.left = aData.leftTargets.back();
				}

				aData.angle = GetFunnelAngle(aData.left, aData.right, aData.apex);
				return;
			}
			if (angle <= aData.angle)
			{
				aData.angle = angle;
				aData.left = tempLeft;
			}
		}

		if (aData.right.index != tempRight.index)
		{
			float angle = GetFunnelAngle(aData.left, tempRight, aData.apex);

			if (angle < 0)
			{
				AddPortalCrossings(aData, aData.left, aPortals, aPath);
				Vector3f smoothedApex = GetSmoothedWaypoint(aData.left, aData.apex);
				aPath.emplace_back(smoothedApex);

				aData.apex.index = aData.left.index;
				aData.apex.position = aData.left.position;
				int apexNeighbour = aData.left.neighbourIndex;

				RemoveExcessPortals(aPortals, aData.apex);

				CollectFromSaved(aData.leftSaved, aData.leftTargets, aData.apex.index, aData.left.index);
				CollectFromSaved(aData.rightSaved, aData.rightTargets, apexNeighbour, aData.apex.index);

				AlignTargets(aData.rightTargets, aData.leftTargets);

				if (aData.leftTargets.size() > 0) {
					aData.left = aData.leftTargets.back();
				}
				if (aData.rightTargets.size() > 0) {
					aData.right = aData.rightTargets.back();
				}

				aData.angle = GetFunnelAngle(aData.left, aData.right, aData.apex);
				return;
			}
			if (angle <= aData.angle)
			{
				aData.angle = angle;
				aData.right = tempRight;
			}
		}
	}
	bool Pathfinder::LineIntersect(Vector2f& aOutLeft, Vector2f& aOutRight, const Vector2f aStartPos, const Vector2f aLineDir)
	{
		Vector2f toVertex0 = (aOutLeft - aStartPos).GetNormalized();
		Vector2f toVertex1 = (aOutRight - aStartPos).GetNormalized();
		Vector2f counterClock = { -aLineDir.y, aLineDir.x };

		float dotRightv0 = aLineDir.Dot(toVertex0);
		float dotRightv1 = aLineDir.Dot(toVertex1);

		// If none of the vertices lies infront of right intersectLine -> continue;
		if (dotRightv0 < 0 && dotRightv1 < 0) { return false; }

		float isSameSidev0 = counterClock.Dot(toVertex0);
		float isSameSidev1 = counterClock.Dot(toVertex1);

		bool isSameSide = (isSameSidev0 < 0 && isSameSidev1 < 0) || (isSameSidev0 >= 0 && isSameSidev1 >= 0) ? true : false;

		if (isSameSide) { return false; }

		// This way we can assure that v0_2D is always left and v1_2D is always right
		if (isSameSidev0 < 0) {

			Vector2f tempLeft = aOutLeft;
			aOutLeft = aOutRight;
			aOutRight = tempLeft;
		}
		return true;
	}
	void Pathfinder::AddPortalCrossings(const FunnelData& aData, const FunnelLocation& aSide, std::vector<Portal>& aTraversedPortals, std::vector<Vector3f>& aPath)
	{
		Vector3f result;
		Vector2f v0_2D;
		Vector2f v1_2D;
		Vector3f vecBetweenVertices;
		Vector2f outIntersectPoint2D;
		Linef funnelLine;
		Linef portalLine;
		float scalar;

		for (int i = (int)aTraversedPortals.size() - 1; i >= 0; i--)
		{
			const Portal& portal = aTraversedPortals[i];

			if (portal.indices[0] == aSide.index || portal.indices[1] == aSide.index) { break; }

			v0_2D = { portal.vertices[0].x, portal.vertices[0].z };
			v1_2D = { portal.vertices[1].x, portal.vertices[1].z };

			funnelLine.InitWith2Points({ aData.apex.position.x, aData.apex.position.z }, { aSide.position.x, aSide.position.z });
			portalLine.InitWith2Points(v0_2D, v1_2D);


			if (!IntersectLineLine(funnelLine, portalLine, outIntersectPoint2D)) { continue; }

			vecBetweenVertices = (portal.vertices[1] - portal.vertices[0]);
			scalar = (outIntersectPoint2D - v0_2D).Length() / (v0_2D - v1_2D).Length();

			if (scalar >= 1.0f || scalar < 0.0f) { continue; }

			result = (portal.vertices[0]) + (vecBetweenVertices * scalar);

			//myCrossingPaths.emplace_back(result);
			//aPath.emplace_back(result);
			aPath.emplace_back(result);
			aTraversedPortals.pop_back();
		}
	}
	void Pathfinder::AddEndCrossings(const FunnelData& aData, const FunnelLocation& aSide, std::vector<Portal>& aTraversedPortals, std::vector<Vector3f>& aPath)
	{
		Vector3f result;
		Vector2f v0_2D;
		Vector2f v1_2D;
		Vector3f vecBetweenVertices;
		Vector2f outIntersectPoint2D;
		Linef funnelLine;
		Linef portalLine;
		float scalar;

		// If ApexIndex equals INT_MIN we have never assigned it. Meaning the path is straight(No Funnels overlapped eachother).
		if (aData.apex.index == INT_MIN)
		{
			for (int i = (int)aTraversedPortals.size() - 1; i >= 0; i--)
			{
				Portal& portal = aTraversedPortals[i];

				v0_2D = { portal.vertices[0].x, portal.vertices[0].z };
				v1_2D = { portal.vertices[1].x, portal.vertices[1].z };

				funnelLine.InitWith2Points({ aData.apex.position.x, aData.apex.position.z }, { aSide.position.x, aSide.position.z });
				portalLine.InitWith2Points(v0_2D, v1_2D);

				if (!IntersectLineLine(funnelLine, portalLine, outIntersectPoint2D)) { continue; }

				scalar = (outIntersectPoint2D - v0_2D).Length() / (v0_2D - v1_2D).Length();

				if (scalar >= 1.0f || scalar <= 0.0f) { continue; }

				vecBetweenVertices = (portal.vertices[1] - portal.vertices[0]);
				result = (portal.vertices[0]) + (vecBetweenVertices * scalar);

				//myCrossingPaths.emplace_back(result);
				aPath.emplace_back(result);
			}
		}
		else
		{
			Portal portal;

			for (int j = (int)aTraversedPortals.size() - 1; j > 0; j--)
			{
				portal = aTraversedPortals[j];

				v0_2D = { portal.vertices[0].x, portal.vertices[0].z };
				v1_2D = { portal.vertices[1].x, portal.vertices[1].z };

				funnelLine.InitWith2Points({ aData.apex.position.x, aData.apex.position.z }, { aSide.position.x, aSide.position.z });
				portalLine.InitWith2Points(v0_2D, v1_2D);

				if (!IntersectLineLine(funnelLine, portalLine, outIntersectPoint2D)) { continue; }

				scalar = (outIntersectPoint2D - v0_2D).Length() / (v0_2D - v1_2D).Length();

				if (scalar >= 1.0f || scalar <= 0.0f) { continue; }

				vecBetweenVertices = (portal.vertices[1] - portal.vertices[0]);
				result = (portal.vertices[0]) + (vecBetweenVertices * scalar);

				//myCrossingPaths.emplace_back(result);
				aPath.emplace_back(result);
			}
		}
	}
	void Pathfinder::RemoveExcessPortals(std::vector<Portal>& aTraversedPortals, const Apex& aApex)
	{
		for (int i = 0; i < aTraversedPortals.size(); i++)
		{
			const Portal& portal = aTraversedPortals[i];

			if (portal.indices[0] == aApex.index || portal.indices[1] == aApex.index)
			{
				while (i < aTraversedPortals.size())
				{
					aTraversedPortals.pop_back();
				}
			}
		}
	}


	// <[ SETUP ]> //
	bool Pathfinder::AStar(NavNode* aStartNode, NavNode* aEndNode, const Vector3f& aEnd)
	{
		ResetScore();

		aStartNode->status = eNodeStatus::Open;
		aStartNode->fromDist = 0;
		aStartNode->myScorePos = aEnd;
		aStartNode->cost = 0;

		std::priority_queue<NavNode> cellsToCheck;

		NavNode* current = aStartNode;
		while (current != aEndNode)
		{
			ScoreNeighbours(*current, cellsToCheck, aEnd);

			if (cellsToCheck.size() == 0)
			{
				KE_ERROR("FunnelPath failed, returning empty vector");
				return false;
			}

			int next = cellsToCheck.top().index;
			current = &myNodes->at(next);

			cellsToCheck.pop();
		}

		aEndNode->cost = 0.0f;

		return true;
	}
	void Pathfinder::ResetScore()
	{
		for (int i = 0; i < myNodes->size(); i++)
		{
			myNodes->at(i).Reset();
		}
	}
	void Pathfinder::ScoreCell(NavNode& aFromNode, NavNode& aNodeToScore, const Vector3f& aTarget)
	{
		// <[SOLUTION 3.0]> //
		Vector3f v0(FLT_MIN, FLT_MIN, FLT_MIN);
		Vector3f v1(FLT_MIN, FLT_MIN, FLT_MIN);
		int v0_debugIndex;
		int v1_debugIndex;

		for (int i = 0; i < aFromNode.indices.size(); i++)
		{
			int fromIndice = aFromNode.indices[i];

			for (int k = 0; k < aNodeToScore.indices.size(); k++)
			{
				int toIndice = aNodeToScore.indices[k];

				if (fromIndice != toIndice) { continue; }


				if (v0.x == FLT_MIN)
				{
					v0_debugIndex = toIndice;
					v0 = *aNodeToScore.vertices[k];
				}
				else if (v1.x == FLT_MIN)
				{
					v1_debugIndex = toIndice;
					v1 = *aNodeToScore.vertices[k];
					break;
				}
			}
		}

		aNodeToScore.myScorePos = v1 + ((v0 - v1) / 2.0f);
		aNodeToScore.fromDist = (aFromNode.myScorePos - aNodeToScore.myScorePos).Length() + aFromNode.fromDist;
		aNodeToScore.toDist = (aNodeToScore.myScorePos - aTarget).Length();
		aNodeToScore.cost = aNodeToScore.fromDist + aNodeToScore.toDist;
	}
	void Pathfinder::ScoreNeighbours(NavNode& aNode, std::priority_queue<NavNode>& aQueue, const Vector3f& aTarget)
	{
		for (int i = 0; i < aNode.myNeighbours.size(); i++)
		{
			NavNode& currentNeighbour = *aNode.myNeighbours[i];

			if (currentNeighbour.status == eNodeStatus::Unvisited)
			{
				currentNeighbour.status = eNodeStatus::Open;
				ScoreCell(aNode, currentNeighbour, aTarget);
				aQueue.push(currentNeighbour);
			}
		}
	}
	void Pathfinder::CreateChannel(std::vector<NavNode*>& aChannel, NavNode* aStartNode, NavNode* aEndNode)
	{
		aChannel.emplace_back(aEndNode);

		NavNode* current = aEndNode;
		NavNode* bestNode = nullptr;
		float fromDist = FLT_MAX;
		aStartNode->cost = 0;

		while (current != aStartNode)
		{
			bestNode = nullptr;
			fromDist = FLT_MAX;

			for (int i = 0; i < current->myNeighbours.size(); i++)
			{
				NavNode* neighbour = current->myNeighbours[i];

				if (!neighbour) { continue; }

				if (neighbour->fromDist <= fromDist) {
					bestNode = neighbour;
					fromDist = neighbour->fromDist;
				}
			}
			current = bestNode;

			aChannel.emplace_back(bestNode);
		}
	}
	void Pathfinder::CreateFunnelLocations(std::vector<NavNode*>& aChannel, FunnelData& aData, Vector3f& aTarget, std::vector<Portal>* aCrossingPortals)
	{
		NavNode* fromNode = nullptr;
		NavNode* toNode = nullptr;

		aData.leftTargets.emplace_back(aTarget, -1);
		aData.rightTargets.emplace_back(aTarget, -1);

		for (int i = 0; i < aChannel.size() - 1; i++)
		{
			toNode = aChannel[i];
			fromNode = aChannel[i + 1];

			Portal portal;
			GeneratePortal(portal, fromNode, toNode);

			if (aCrossingPortals) {
				aCrossingPortals->emplace_back(portal);
			}

			// Determine left and right vertices.
			Vector3f v0 = portal.vertices[0];
			Vector3f v1 = portal.vertices[1];
			Vector3f center = fromNode->myCenter;

			Vector2f dir = Vector2f(v0.x - center.x, v0.z - center.z).GetNormalized();
			Vector2f counterclockDir = { -dir.y, dir.x };
			Vector2f point = { v1.x - center.x, v1.z - center.z };

			float dot = counterclockDir.Dot(point);

			bool right = dot < 0.0f ? 1 : 0;
			bool left = !right;

			aData.leftTargets.emplace_back(portal.vertices[left], portal.indices[left]);
			aData.leftTargets.back().neighbourIndex = portal.indices[right];

			aData.rightTargets.emplace_back(portal.vertices[right], portal.indices[right]);
			aData.rightTargets.back().neighbourIndex = portal.indices[left];
		}
	}
	void Pathfinder::GeneratePortal(Portal& aOutPortal, const NavNode* aFromNode, const NavNode* aToNode)
	{
		int assignCount = 0;
		for (int i = 0; i < aFromNode->indices.size(); i++)
		{
			for (int j = 0; j < aToNode->indices.size(); j++)
			{
				if (aToNode->indices[j] == aFromNode->indices[i])
				{
					aOutPortal.vertices[assignCount] = *aToNode->vertices[j];
					aOutPortal.indices[assignCount] = aToNode->indices[j];
					assignCount++;
				}
			}
		}
	}
	bool Pathfinder::ConstrainToNavmesh(Vector3f& aOutPos, int& aOutIndex)
	{
		if (aOutIndex >= 0) return true;

		Vector3f closestPos = GetNearestNavmeshPos(aOutPos);
		aOutIndex = GetNodeIndexFromPoint(closestPos);

		if (aOutIndex < 0) { return false; }

		aOutPos = closestPos;
		return true;
	}


	// <[ COMMON ]> //
	Vector3f Pathfinder::GetSmoothedWaypoint(const FunnelLocation& aSide, const Apex& aApex, const float aSmoothFactor) const
	{
		Vector3f dirToPoint = (aSide.position - aApex.position).GetNormalized();
		Vector3f counterclockDir = Vector3f({ -dirToPoint.z, aSide.position.y, dirToPoint.x });
		Vector3f smoothedApex = aSide.position + (counterclockDir * aSmoothFactor);

		return smoothedApex;
	}
	Vector3f Pathfinder::GetNearestNavmeshPos(const Vector3f& aTargetPos)
	{
		if (!myNavmesh || myNavmesh->myNodes.empty())
		{
			KE_ERROR(
				"Pathfinding Error: Failed to retrieve nearest navmesh pos. Navmesh was null or empty. \
				Returning input pos: (x %.2f, y %.2f, z %.2f)", aTargetPos.x, aTargetPos.y, aTargetPos.z);

			return aTargetPos;
		}

		NodesAttachedToIndex* indexToNodeMap = nullptr;
		Vector3f vertex;
		int debugIndex = INT_MIN;
		float distance = FLT_MAX;

		// <<<[Get the closest vertex and all nodes with that vertex]>>> //
		for (int i = 0; i < myNavmesh->myVertices.size(); i++)
		{
			if (myNavmesh->myIndiceNodeMap.find(i) == myNavmesh->myIndiceNodeMap.end())
			{
				continue;
			}

			float tempDistance = (myNavmesh->myVertices[i] - aTargetPos).Length();

			if (tempDistance < distance)
			{
				distance = tempDistance;
				indexToNodeMap = &myNavmesh->myIndiceNodeMap.at(i);
				vertex = myNavmesh->myIndiceNodeMap.at(i).vertex;
				debugIndex = i;
			}
		}

		Vector2f targetPos2D = { aTargetPos.x, aTargetPos.z };

		std::array<Linef, 4> myIntersectionLines = {
				Linef(targetPos2D, {targetPos2D.x + 1, targetPos2D.y	}),	// Right
				Linef(targetPos2D, {targetPos2D.x - 1, targetPos2D.y	}),	// Left
				Linef(targetPos2D, {targetPos2D.x	 , targetPos2D.y + 1}), // Up
				Linef(targetPos2D, {targetPos2D.x	 , targetPos2D.y - 1})	// Down
		};

		Vector3f closestPoint = { 0, 0, 0 };

		distance = FLT_MAX;
		int intersectedLine = INT_MIN;

		for (int i = 0; i < indexToNodeMap->nodes.size(); i++)
		{
			NavNode& node = *indexToNodeMap->nodes[i];

			Vector3f result;
			Vector2f outIntersectPoint2D;
			Vector2f v0_2D;
			Vector2f v1_2D;
			Linef vertexLine;

			for (int j = 0; j < node.vertices.size(); j++)
			{
				int min = j;
				int max = j > 1 ? 0 : j + 1;

				v0_2D = { node.vertices[min]->x, node.vertices[min]->z };
				v1_2D = { node.vertices[max]->x, node.vertices[max]->z };


				for (int k = 0; k < myIntersectionLines.size(); k++)
				{
					Linef& intersectLine = myIntersectionLines[k];

					// If none of the vertices lies on each side of the intersectLine Direction -> Skip it
					// Else the function modifies v0 to be leftside and v1 to be rightside
					if (!LineIntersect(v0_2D, v1_2D, targetPos2D, intersectLine.GetDirection())) { continue; }

					vertexLine.InitWith2Points(v0_2D, v1_2D);

					if (IntersectLineLine(intersectLine, vertexLine, outIntersectPoint2D))
					{
						Vector3f vecBetweenVertices = (*node.vertices[max] - *node.vertices[min]);
						float scalar = (outIntersectPoint2D - v0_2D).Length() / (v0_2D - v1_2D).Length();

						if (scalar >= 1.0f || scalar <= 0.0f) { continue; }

						result = (*node.vertices[min]) + (vecBetweenVertices * scalar);

						float tempDistance = (aTargetPos - result).Length();

						if (tempDistance < distance)
						{
							intersectedLine = k;
							distance = tempDistance;
							closestPoint = result;
						}
					}
				}
			}
		}

		if (intersectedLine != INT_MIN) {
			Vector2f offset = myIntersectionLines[intersectedLine].GetDirection() / 50.f;

			closestPoint.x += offset.x;
			closestPoint.z += offset.y;
		}
		else
		{
			closestPoint = vertex;
		}

		return closestPoint;
	}
	float Pathfinder::GetFunnelAngle(const FunnelLocation& aLeft, const FunnelLocation& aRight, const Apex& aApex) const
	{
		Vector3f left = aLeft.position - aApex.position;
		Vector3f right = aRight.position - aApex.position;

		float dot = right.x * left.x + right.z * left.z;
		float det = right.x * left.z - right.z * left.x;

		// This is required to not make the angle wrap around to -179,99 /DR
		if (abs(det) < 0.01) det = 0;

		float angle = atan2(det, dot) * 180.f / PI;

		// Checks if the vectors cross each other
		if (det < 0)
		{
			if (angle < 0) {
				return angle;
			}

			angle = 360 + angle;
		}

		return angle;
	}
	void Pathfinder::CollectFromSaved(std::vector<FunnelLocation>& aSavedList, std::vector<FunnelLocation>& aTargetList, const int aIndexToFind, const int aIndexToAvoid)
	{
		for (int i = 0; i < aSavedList.size(); i++)
		{
			if (aIndexToFind == aSavedList[i].index) {

				for (int j = (int)aSavedList.size() - 1; j > i; j--)
				{
					if (aSavedList[j].index != aIndexToAvoid)
					{
						aTargetList.emplace_back(aSavedList[j]);
					}
				}
				aSavedList.clear();
				break;
			}
		};
	}
	int Pathfinder::GetNodeIndexFromPoint(const Vector3f aPosition)
	{
		Vector3f origin = { aPosition.x, aPosition.y + 1.0f, aPosition.z };
		Vector3f direction = { 0, -1, 0 };
		Rayf ray = { origin,  direction };
		Plane plane;
		Vector3f intersectPoint;
		Vector3f v1;
		Vector3f v2;
		Vector3f v3;
		float distance = FLT_MAX;
		int nodeIndex = -1;

		std::vector<NavNode*> nodes = myNavmesh->GetNodesFromPos(aPosition);

		for (int i = 0; i < nodes.size(); i++)
		{
			auto& node = nodes[i];

			v1 = *node->vertices[0];
			v2 = *node->vertices[1];
			v3 = *node->vertices[2];

			plane.InitWith3Points(v3, v2, v1);

			if (!IntersectionPlaneRay(plane, ray, intersectPoint)) { continue; }

			float tempDist = (aPosition - intersectPoint).Length();

			if (InsideTriangle(intersectPoint, node->vertices) && tempDist < distance)
			{
				distance = tempDist;
				nodeIndex = node->index;
			}
		}
		return nodeIndex;
	}
	void Pathfinder::AlignTargets(std::vector<FunnelLocation>& aListToReduce, const std::vector<FunnelLocation>& aCompareList)
	{
		while (aListToReduce.size() > aCompareList.size())
		{
			aListToReduce.pop_back();
		}
	}
	NavNode* Pathfinder::GetNode(const Vector3f& aPosition)
	{
		if (!myNodes || !myNavmesh) {
			KE_ERROR("Pathfinder: GetNode failed, myNodes or myNavmesh was nullptr");
			return nullptr;
		}

		// <<[Raycast from above the position]>> //
		Rayf ray = Rayf({ aPosition.x, aPosition.y + 1.0f, aPosition.z }, { 0, -1, 0 });
		Plane plane;
		Vector3f intersectPoint;
		const Vector3f* v1;
		const Vector3f* v2;
		const Vector3f* v3;
		float distance = FLT_MAX;
		NavNode* output = nullptr;

		std::vector<NavNode*> nodes = myNavmesh->GetNodesFromPos(aPosition);

		for (int i = 0; i < nodes.size(); i++)
		{
			auto node = nodes.at(i);

			v1 = node->vertices[0];
			v2 = node->vertices[1];
			v3 = node->vertices[2];

			plane.InitWith3Points(*v3, *v2, *v1);

			if (!IntersectionPlaneRay(plane, ray, intersectPoint)) { continue; }

			float tempDist = (aPosition - intersectPoint).Length();

			if (InsideTriangle(intersectPoint, node->vertices) && tempDist < distance)
			{
				distance = tempDist;
				output = node;
			}
		}
		return output;
	}
	const float Pathfinder::GetHeightByPos(const Vector3f& aPoint)
	{
		Vector3f output = aPoint;
		const NavNode* node = GetNode(aPoint);

		if (!node) { return aPoint.y; }

		GetHeightInsideTriangle(output, node->vertices);

		return output.y;
	}

}