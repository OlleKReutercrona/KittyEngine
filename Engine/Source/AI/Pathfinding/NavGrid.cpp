#include "stdafx.h"
#include "NavGrid.h"
#include "NavNode.h"

namespace KE
{
	NavGrid::NavGrid(Vector2i aGridSize, float aCellSize) : myGridSize(aGridSize), myCellSize(aCellSize)
	{

	}

	bool NavGrid::Init(float aCellSize, Vector3f aMin, Vector3f aMax, std::vector<KE::NavNode>& someNodes)
	{
		myCellSize = aCellSize;
		myMin = { aMin.x, aMin.z };
		myMax = { aMax.x, aMax.z };

		myGridSize = { (int)(abs(aMax.x - aMin.x) / myCellSize) + 1, (int)(abs(aMax.z - aMin.z) / myCellSize) + 1 };
		myNavGrid.resize(myGridSize.x * myGridSize.y);

		for (int i = 0; i < someNodes.size(); i++)
		{
			KE::NavNode& node = someNodes[i];

			Vector3f min = { 1000000, 0.0f, 1000000 };
			Vector3f max = { -1000000, 0.0f, -1000000 };

			for (auto& vertex : node.vertices)
			{
				if (vertex->x < min.x) {
					min.x = vertex->x;
				}
				if (vertex->z < min.z) {
					min.z = vertex->z;
				}
				if (vertex->x > max.x) {
					max.x = vertex->x;
				}
				if (vertex->z > max.z) {
					max.z = vertex->z;
				}
			}

			Vector2i minCord = GetCoordinate(min);
			Vector2i maxCord = GetCoordinate(max);

			int xSize = abs(maxCord.x - minCord.x) + 1;
			int ySize = abs(maxCord.y - minCord.y) + 1;

			for (int y = 0; y < ySize; y++)
			{
				for (int x = 0; x < xSize; x++)
				{
					int index = (minCord.x + x) + (minCord.y + y) * myGridSize.x;

					myNavGrid[index].myData.push_back(&node);
				}
			}
		}
		return true;
	}

	Vector2i NavGrid::GetCoordinate(Vector3f aPos)
	{
		float relativeX = aPos.x - myMin.x;
		float relativeY = aPos.z - myMin.y;

		return Vector2i((int)(relativeX / myCellSize), (int)(relativeY / myCellSize));
	}

	int NavGrid::GetIndexByPos(Vector3f aPos)
	{
		Vector2i coordinate = GetCoordinate(aPos);

		return coordinate.x + coordinate.y * myGridSize.x;
	}

	std::vector<KE::NavNode*> NavGrid::GetNodesByPos(Vector3f aPos)
	{
		int index = GetIndexByPos(aPos);

		if (index < 0 || index >= myNavGrid.size())
			return std::vector<KE::NavNode*>();

		return myNavGrid[index].myData;
	}
}