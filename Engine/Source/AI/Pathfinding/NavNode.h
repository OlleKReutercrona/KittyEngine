#pragma once
#include <Engine/Source/Math/Vector2.h>
#include <array>
#include <vector>
#include "Engine/Source/Math/Line.h"
#include "Editor/Source/EditorInterface.h"

#define EPSILON 0.01f

namespace KE
{
	inline static float Sign(Vector2f aPoint0, Vector2f aPoint1, Vector2f aPoint2)
	{
		return (aPoint0.x - aPoint2.x) * (aPoint1.y - aPoint2.y) - (aPoint1.x - aPoint2.x) * (aPoint0.y - aPoint2.y);
	}
	inline static bool InsideTriangle(const Vector3f& aPoint, std::array<const Vector3f*, 3>& someVertices)
	{
		float d1, d2, d3;
		bool has_neg, has_pos;

		const Vector2f point2D = { aPoint.x, aPoint.z };
		const Vector2f v1 = { someVertices[0]->x, someVertices[0]->z };
		const Vector2f v2 = { someVertices[1]->x, someVertices[1]->z };
		const Vector2f v3 = { someVertices[2]->x, someVertices[2]->z };

		d1 = Sign(point2D, v1, v2);
		d2 = Sign(point2D, v2, v3);
		d3 = Sign(point2D, v3, v1);

		if (abs(d1) < EPSILON) d1 = 0;
		if (abs(d2) < EPSILON) d2 = 0;
		if (abs(d3) < EPSILON) d3 = 0;

		has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
		has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

		return !(has_neg && has_pos);
	}
	inline static bool GetHeightInsideTriangle(Vector3f& aOutPos, const std::array<const Vector3f*, 3>& someVertices)
	{
		const Vector2f point2D = { aOutPos.x, aOutPos.z };
		float d1, d2, d3;
		bool has_neg, has_pos;

		const Vector2f v1 = { someVertices[0]->x, someVertices[0]->z };
		const Vector2f v2 = { someVertices[1]->x, someVertices[1]->z };
		const Vector2f v3 = { someVertices[2]->x, someVertices[2]->z };

		d1 = Sign(point2D, v1, v2);
		d2 = Sign(point2D, v2, v3);
		d3 = Sign(point2D, v3, v1);

		if (abs(d1) < EPSILON) d1 = 0;
		if (abs(d2) < EPSILON) d2 = 0;
		if (abs(d3) < EPSILON) d3 = 0;

		has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
		has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

		if (!(has_neg && has_pos))
		{
			float u = d2 / (d1 + d2 + d3);
			float v = d3 / (d1 + d2 + d3);
			float w = 1.0f - u - v;

			float height = u * someVertices[0]->y + v * someVertices[1]->y + w * someVertices[2]->y;

			if (isnan(height))
			{
				KE_ERROR("GetHeightInsideTriangle gave nan-ind. Returning average triangle height");

				height = (someVertices[0]->y + someVertices[1]->y + someVertices[2]->y) / 3.0f;
			}

			aOutPos.y = height;
			return true;
		}
		return true;
	}

	enum class eNodeStatus
	{
		Unvisited,
		Open,
		Closed
	};

	struct NavNode
	{
		NavNode(int aID, std::array<unsigned int, 3> someIndices, std::array<const Vector3f*, 3>& someVertices) :
			index(aID), indices(someIndices), vertices(someVertices)
		{
			myCenter = (*vertices[0] + *vertices[1] + *vertices[2]) / 3.f;
		}

		inline void Reset()
		{
			cost = FLT_MAX;
			toDist = FLT_MAX;
			fromDist = FLT_MAX;
			status = eNodeStatus::Unvisited;
			myScorePos = myCenter;
		}

		float cost = FLT_MAX;
		float fromDist = FLT_MAX;
		float toDist = FLT_MAX;
		eNodeStatus status = eNodeStatus::Unvisited;

		int index = INT_MAX;
		Vector3f myCenter = {};
		Vector3f myScorePos = {};
		std::array<const Vector3f*, 3> vertices;
		std::array<unsigned int, 3> indices;

		std::vector<NavNode*> myNeighbours;


#pragma region LineColliders
		std::vector<Linef> myLineColliders;
		inline void ClampVelocity(const Vector3f& aPosition, Vector3f& aVelocity)
		{
			Vector2f velo_2D = { aVelocity.x, aVelocity.z };

			for (Linef& line : myLineColliders)
			{
				if (line.IsInside(aPosition + aVelocity))
				{
					float dotProduct = velo_2D.GetNormalized().Dot(line.GetNormal());
			
					velo_2D -= (dotProduct * velo_2D.Length()) * line.GetNormal();
				}
			}
			aVelocity.x = velo_2D.x;
			aVelocity.z = velo_2D.y;
		}
		inline void ClampVelocity(const Vector3f& aPosition, Vector2f& aVelocity)
		{
			Vector3f pos = aPosition;
			pos.x += aVelocity.x;
			pos.z += aVelocity.y;


			for (Linef& line : myLineColliders)
			{
				if (line.IsInside(pos))
				{
					float dotProduct = aVelocity.GetNormalized().Dot(line.GetNormal());

					aVelocity -= (dotProduct * aVelocity.Length()) * line.GetNormal();
				}
			}
		}
#pragma endregion

	};

	inline bool operator<(const NavNode& aFirst, const NavNode& aSecond)
	{
		return aFirst.cost > aSecond.cost;
	}
	inline bool operator<(NavNode& aFirst, NavNode& aSecond)
	{
		return aFirst.cost > aSecond.cost;
	}

}
#undef EPSILON