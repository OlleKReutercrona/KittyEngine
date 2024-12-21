#include "stdafx.h"
#include "CapsulePrimitive.h"

KE::CapsulePrimitive::CapsulePrimitive()
{
	if (hasInit) return;

	hasInit = true;

	const int aPolyCount = 50;
	const float radius = 1.0f;
	float angle = 360.0f / aPolyCount;

	// lower x
	{
		// Bottom half of the X Circle

		const int firstInd = aPolyCount / 2;

		for (int i = firstInd; i <= aPolyCount; i++)
		{
			float currentAngle = angle * i;
			float x = radius * cos(KE::DegToRad(currentAngle));
			float y = radius + radius * sin(KE::DegToRad(currentAngle));
			float z = 0.0f;

			myMesh.myVertices.push_back({ x,y,z,1.0f });
			myMesh.myIndices.push_back(i - firstInd);
			if (i != firstInd)
			{
				myMesh.myIndices.push_back(i - firstInd);
			}
		}

		myMesh.lengthIndices[0].x = 0;
		myMesh.lengthIndices[1].x = (int)myMesh.myVertices.size() - 1;

		myMesh.myIndices.pop_back();
	}


	// lower z
	{
		const int firstInd = (int)myMesh.myVertices.size();

		for (int i = aPolyCount / 2; i <= aPolyCount; i++)
		{
			float currentAngle = angle * i;
			float x = 0.0f;
			float y = radius + radius * sin(KE::DegToRad(currentAngle));
			float z = radius * cos(KE::DegToRad(currentAngle));

			myMesh.myVertices.push_back({ x,y,z,1.0f });

			int vIndex = firstInd + i - aPolyCount / 2;
			myMesh.myIndices.push_back(vIndex);
			if (i != aPolyCount / 2)
			{
				myMesh.myIndices.push_back(vIndex);
			}
		}
		myMesh.lengthIndices[2].x = firstInd;
		myMesh.lengthIndices[3].x = (int)myMesh.myVertices.size() - 1;

		myMesh.myIndices.pop_back();

	}



	// lower y
	{
		const int firstInd = (int)myMesh.myVertices.size();

		for (int i = 0; i < aPolyCount; i++)
		{
			float currentAngle = angle * i;
			float x = radius * cos(KE::DegToRad(currentAngle));
			float y = radius + 0.0f;
			float z = radius * sin(KE::DegToRad(currentAngle));

			myMesh.myVertices.push_back({ x,y,z,1.0f });

			const int vIndex = firstInd + i;
			myMesh.myIndices.push_back(vIndex);
			if (i != 0)
			{
				myMesh.myIndices.push_back(vIndex);
			}
		}
		myMesh.myIndices.push_back(firstInd);
	}

	myMesh.halfWayIndex = (int)myMesh.myVertices.size();

	// upper y
	{
		const int firstInd = (int)myMesh.myVertices.size();

		for (int i = 0; i < aPolyCount; i++)
		{
			float currentAngle = angle * i;
			float x = radius * cos(KE::DegToRad(currentAngle));
			float y = radius + 0.0f;
			float z = radius * sin(KE::DegToRad(currentAngle));

			myMesh.myVertices.push_back({ x,y,z,1.0f });

			const int vIndex = firstInd + i;
			myMesh.myIndices.push_back(vIndex);
			if (i != 0)
			{
				myMesh.myIndices.push_back(vIndex);
			}
		}
		myMesh.myIndices.push_back(firstInd);
	}

	// Upper x
	{
		// Upper half of the X Circle

		const int firstInd = (int)myMesh.myVertices.size();

		for (int i = 0; i <= aPolyCount / 2; i++)
		{
			float currentAngle = angle * i;
			float x = radius * cos(KE::DegToRad(currentAngle));
			float y = radius + radius * sin(KE::DegToRad(currentAngle));
			float z = 0.0f;

			myMesh.myVertices.push_back({ x,y,z,1.0f });
			const int vIndex = firstInd + i;
			myMesh.myIndices.push_back(vIndex);
			if (i != 0)
			{
				myMesh.myIndices.push_back(vIndex);
			}
		}

		myMesh.lengthIndices[0].y = (int)myMesh.myVertices.size() - 1;
		myMesh.lengthIndices[1].y = firstInd;

		myMesh.myIndices.pop_back();
	}


	// Upper z
	{
		const int firstInd = (int)myMesh.myVertices.size();

		for (int i = 0; i <= aPolyCount / 2; i++)
		{
			float currentAngle = angle * i;
			float x = 0.0f;
			float y = radius + radius * sin(KE::DegToRad(currentAngle));
			float z = radius * cos(KE::DegToRad(currentAngle));

			myMesh.myVertices.push_back({ x,y,z,1.0f });

			int vIndex = firstInd + i;
			myMesh.myIndices.push_back(vIndex);
			if (i != 0)
			{
				myMesh.myIndices.push_back(vIndex);
			}
		}
		myMesh.lengthIndices[2].y = (int)myMesh.myVertices.size() - 1;
		myMesh.lengthIndices[3].y = firstInd;

		myMesh.myIndices.pop_back();
	}
}
