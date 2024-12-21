#pragma once

namespace KE
{
	struct NavNode;
	class Navmesh;


	class GridCell
	{
		friend class NavGrid;

	public:
		GridCell() {};
		~GridCell() {};
		inline std::vector<KE::NavNode*> GetNodes() { return myData; };

	private:
		std::vector<KE::NavNode*> myData;
	};

	class NavGrid
	{
		friend class KE::Navmesh;

	public:
		NavGrid() {};
		~NavGrid() {};
		NavGrid(Vector2i aGridSize, float aCellSize);

		bool Init(float aCellSize, Vector3f aMin, Vector3f aMax, std::vector<KE::NavNode>& someNodes);
		int GetIndexByPos(Vector3f aPos);
		Vector2i GetCoordinate(Vector3f aPos);
		std::vector<KE::NavNode*> GetNodesByPos(Vector3f aPos);

	private:
		float myCellSize = 0;
		Vector2f myMin;
		Vector2f myMax;
		Vector2i myGridSize;

		std::vector<GridCell> myNavGrid;
	};
}
