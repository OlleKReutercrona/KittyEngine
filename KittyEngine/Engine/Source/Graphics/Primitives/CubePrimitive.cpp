#include "stdafx.h"
#include "CubePrimitive.h"

KE::CubePrimitive::CubePrimitive()
{
	if (myIsInitialized) return;

	float halfSize = 0.5f; // This should always be defaulted to 0.5f as size should be set elsewhere

	myVertices[0] = VertexPoint(-halfSize , -halfSize, -halfSize, +1.0f);
	myVertices[1] = VertexPoint(+halfSize , -halfSize, -halfSize, +1.0f);
	myVertices[2] = VertexPoint(-halfSize , +halfSize, -halfSize, +1.0f);
	myVertices[3] = VertexPoint(+halfSize , +halfSize, -halfSize, +1.0f);
	myVertices[4] = VertexPoint(-halfSize , -halfSize, +halfSize, +1.0f);
	myVertices[5] = VertexPoint(+halfSize , -halfSize, +halfSize, +1.0f);
	myVertices[6] = VertexPoint(-halfSize , +halfSize, +halfSize, +1.0f);
	myVertices[7] = VertexPoint(+halfSize , +halfSize, +halfSize, +1.0f);


	myIndices[0]	= 0;
	myIndices[1]	= 1;
	myIndices[2]	= 1;
	myIndices[3]	= 3;
	myIndices[4]	= 3;
	myIndices[5]	= 2;
	myIndices[6]	= 2;
	myIndices[7]	= 0;
	myIndices[8]	= 0;
	myIndices[9]	= 4;
	myIndices[10]	= 4;
	myIndices[11]	= 5;
	myIndices[12]	= 5;
	myIndices[13]	= 1;
	myIndices[14]	= 5;
	myIndices[15]	= 7;
	myIndices[16]	= 4;
	myIndices[17]	= 6;
	myIndices[18]	= 6;
	myIndices[19]	= 7;
	myIndices[20]	= 3;
	myIndices[21]	= 7;
	myIndices[22]	= 2;
	myIndices[23]	= 6;

	myIsInitialized = true;
}

KE::CubePrimitive::~CubePrimitive()
{
}
