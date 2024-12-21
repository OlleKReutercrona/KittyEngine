#include "stdafx.h"
#include "SpherePrimitive.h"
#include "Math\KittyMath.h"

KE::SpherePrimitive::SpherePrimitive()
{
	if (myIsInitialized) return;

	ContructSphere(KE::SphereLOD::lowPoly, myLowPolyLatDiv, myLowPolyLongDiv);

	ContructSphere(KE::SphereLOD::highPoly, myHighPolyLatDiv, myHighPolyLongDiv);

	ContructAxisSphere(50);

	myIsInitialized = true;
}

void KE::SpherePrimitive::ContructSphere(KE::SphereLOD aLOD, const int aLat, const int aLong)
{
	SphereData data;

	int aLatDiv = aLat;
	int aLongDiv = aLong;

	constexpr float radius = 1.0f;
	const auto base = DirectX::XMVectorSet(0.0f, 0.0f, radius, 0.0f);
	const float lattitudeAngle = PI / aLatDiv;
	const float longitudeAngle = 2.0f * PI / aLongDiv;

	for (int iLat = 1; iLat < aLatDiv; iLat++)
	{
		const auto latBase = DirectX::XMVector3Transform(
			base,
			DirectX::XMMatrixRotationX(lattitudeAngle * iLat)
		);
		for (int iLong = 0; iLong < aLongDiv; iLong++)
		{
			DirectX::XMFLOAT3 calculatedPos;
			auto v = DirectX::XMVector3Transform(
				latBase,
				DirectX::XMMatrixRotationZ(longitudeAngle * iLong)
			);
			DirectX::XMStoreFloat3(&calculatedPos, v);
			data.myVertices.emplace_back(calculatedPos.x, calculatedPos.y, calculatedPos.z, 1.0f);
		}
	}

	// Add the cap vertices
	const auto iNorthPole = static_cast<unsigned short>(data.myVertices.size());
	{
		DirectX::XMFLOAT3 northPos;
		DirectX::XMStoreFloat3(&northPos, base);
		data.myVertices.emplace_back(northPos.x, northPos.y, northPos.z, 1.0f);
	}
	const auto iSouthPole = static_cast<unsigned short>(data.myVertices.size());
	{
		DirectX::XMFLOAT3 southPos;
		DirectX::XMStoreFloat3(&southPos, DirectX::XMVectorNegate(base));
		data.myVertices.emplace_back(southPos.x, southPos.y, southPos.z, 1.0f);
	}

	const auto calcIdx = [aLatDiv, aLongDiv](unsigned short aILat, unsigned short aILong)
		{
			return aILat * aLongDiv + aILong;
		};

#pragma warning (push)
#pragma warning (disable : 4244)
	for (unsigned short iLat = 0; iLat < aLatDiv - 2; iLat++)
	{
		for (unsigned short iLong = 0; iLong < aLongDiv - 1; iLong++)
		{
			data.myIndices.push_back(calcIdx(iLat, iLong));
			data.myIndices.push_back(calcIdx(iLat + 1, iLong));
			data.myIndices.push_back(calcIdx(iLat, iLong + 1));
			data.myIndices.push_back(calcIdx(iLat, iLong + 1));
			data.myIndices.push_back(calcIdx(iLat + 1, iLong));
			data.myIndices.push_back(calcIdx(iLat + 1, iLong + 1));
		}
		// Wrap band
		data.myIndices.push_back(calcIdx(iLat, aLongDiv - 1));
		data.myIndices.push_back(calcIdx(iLat + 1, aLongDiv - 1));
		data.myIndices.push_back(calcIdx(iLat, 0));
		data.myIndices.push_back(calcIdx(iLat, 0));
		data.myIndices.push_back(calcIdx(iLat + 1, aLongDiv - 1));
		data.myIndices.push_back(calcIdx(iLat + 1, 0));
	}

	// Cap fans
	for (unsigned short iLong = 0; iLong < aLongDiv - 1; iLong++)
	{
		// North
		data.myIndices.push_back(iNorthPole);
		data.myIndices.push_back(calcIdx(0, iLong));
		data.myIndices.push_back(calcIdx(0, iLong + 1));
		// South
		data.myIndices.push_back(calcIdx(aLatDiv - 2, iLong + 1));
		data.myIndices.push_back(calcIdx(aLatDiv - 2, iLong));
		data.myIndices.push_back(iSouthPole);
	}
	// Wrap triangles
	// North
	data.myIndices.push_back(iNorthPole);
	data.myIndices.push_back(calcIdx(0, aLongDiv - 1));
	data.myIndices.push_back(calcIdx(0, 0));
	// South
	data.myIndices.push_back(calcIdx(aLatDiv - 2, 0));
	data.myIndices.push_back(calcIdx(aLatDiv - 2, aLongDiv - 1));
	data.myIndices.push_back(iSouthPole);

	mySphereData[aLOD] = data;
}

void KE::SpherePrimitive::ContructSphere2(KE::SphereLOD aLOD, const int someStacks, const int someSlices)
{
	SphereData mesh;

	// add top vertex
	mesh.myVertices.push_back({ 0.0f,1.0f,0.0f,1.0f });

	// generate vertices per stack / slice
	for (int i = 0; i < someStacks - 1; i++)
	{
		auto phi = KE::PI * double(i + 1) / double(someStacks);
		for (int j = 0; j < someSlices; j++)
		{
			auto theta = 2.0 * KE::PI * double(j) / double(someSlices);
			auto x = std::sin(phi) * std::cos(theta);
			auto y = std::cos(phi);
			auto z = std::sin(phi) * std::sin(theta);
			mesh.myVertices.push_back({ (float)x,(float)y,(float)z,1.0f });
		}
	}

	// add bottom vertex
	mesh.myVertices.push_back({ 0.0f,-1.0f,0.0f,1.0f });


	// Top plates indices
	for (int i = 0; i < someStacks; i++)
	{
		mesh.myIndices.push_back(0);
		mesh.myIndices.push_back(i + 1);
		if (i < someStacks - 1)
		{
			mesh.myIndices.push_back(0);
			mesh.myIndices.push_back(i + 2);

			mesh.myIndices.push_back(i + 1);
			mesh.myIndices.push_back(i + 2);
		}
	}
	mesh.myIndices.push_back(mesh.myIndices.back());
	mesh.myIndices.push_back(1);

	// The middle
	for (int i = 1; i < mesh.myVertices.size() - someSlices - 1; i++)
	{
		if (i % someSlices != 0)
		{
			mesh.myIndices.push_back(i);
			mesh.myIndices.push_back(i + 1);

			mesh.myIndices.push_back(i);
			mesh.myIndices.push_back(i + someSlices);
		}
		else
		{
			int index = i / someSlices;

			mesh.myIndices.push_back((index - 1) * someSlices + 1);
			mesh.myIndices.push_back(i);

			mesh.myIndices.push_back(index * someSlices);
			mesh.myIndices.push_back(i + someSlices);
		}
	}


	// Top plates indices

	const int botVertice = (int)mesh.myVertices.size() - 1;

	for (int i = someStacks * (someSlices - 2); i < someStacks * (someSlices - 1); i++)
	{
		mesh.myIndices.push_back(botVertice);
		mesh.myIndices.push_back(i + 1);
		if (i < someStacks - 1)
		{
			mesh.myIndices.push_back(botVertice);
			mesh.myIndices.push_back(i + 2);

			mesh.myIndices.push_back(i + 1);
			mesh.myIndices.push_back(i + 2);
		}
	}
	//mesh.myIndices.push_back(mesh.myIndices.back());
	//mesh.myIndices.push_back(botVertice + 1);

	mySphereData[aLOD] = mesh;
}

void KE::SpherePrimitive::ContructAxisSphere(const int aPolyCount)
{
	SphereData data;

	const float radius = 1.0f;
	float angle = 360.0f / aPolyCount;

	// x
	{
		for (int i = 0; i < aPolyCount; i++)
		{
			float currentAngle = angle * i;
			float x = radius * cos(KE::DegToRad(currentAngle));
			float y = radius * sin(KE::DegToRad(currentAngle));
			float z = 0.0f;

			data.myVertices.push_back({ x,y,z,1.0f });
			data.myIndices.push_back(i);
			if (i != 0)
			{
				data.myIndices.push_back(i);
			}
		}
		data.myIndices.push_back(aPolyCount * 0);
	}

	// y
	{
		data.myIndices.push_back(aPolyCount);

		for (int i = 0; i < aPolyCount; i++)
		{
			float currentAngle = angle * i;
			float x = radius * cos(KE::DegToRad(currentAngle));
			float y = 0.0f;
			float z = radius * sin(KE::DegToRad(currentAngle));

			data.myVertices.push_back({ x,y,z,1.0f });
			data.myIndices.push_back(aPolyCount + i);
			if (i != aPolyCount * 1)
			{
				data.myIndices.push_back(aPolyCount + i);
			}
		}
		data.myIndices.push_back(aPolyCount * 1);
	}


	// z
	{
		int step = 2;

		data.myIndices.push_back(aPolyCount * step);

		for (int i = 0; i < aPolyCount; i++)
		{
			float currentAngle = angle * i;
			float x = 0.0f;
			float y = radius * sin(KE::DegToRad(currentAngle));
			float z = radius * cos(KE::DegToRad(currentAngle));

			data.myVertices.push_back({ x,y,z,1.0f });
			data.myIndices.push_back(aPolyCount * step + i);
			if (i != aPolyCount * step)
			{
				data.myIndices.push_back(aPolyCount * step + i);
			}
		}
		data.myIndices.push_back(aPolyCount * step);
	}

	// xz
	{
		int step = 3;

		data.myIndices.push_back(aPolyCount * step);

		for (int i = 0; i < aPolyCount; i++)
		{
			float currentAngle = angle * i;
			float x = radius * cos(KE::DegToRad(currentAngle)) * 0.7055f;
			float y = radius * sin(KE::DegToRad(currentAngle));
			float z = radius * cos(KE::DegToRad(currentAngle)) * 0.7055f;

			data.myVertices.push_back({ x,y,z,1.0f });
			data.myIndices.push_back(aPolyCount * step + i);
			if (i != aPolyCount * step)
			{
				data.myIndices.push_back(aPolyCount * step + i);
			}
		}
		data.myIndices.push_back(aPolyCount * step);
	}


	// xz
	{
		int step = 4;

		data.myIndices.push_back(aPolyCount * step);

		for (int i = 0; i < aPolyCount; i++)
		{
			float currentAngle = angle * i;
			float x = radius * cos(KE::DegToRad(currentAngle)) * 0.706f;
			float y = radius * sin(KE::DegToRad(currentAngle));
			float z = -radius * cos(KE::DegToRad(currentAngle)) * 0.706f;

			data.myVertices.push_back({ x,y,z,1.0f });
			data.myIndices.push_back(aPolyCount * step + i);
			if (i != aPolyCount * step)
			{
				data.myIndices.push_back(aPolyCount * step + i);
			}
		}
		data.myIndices.push_back(aPolyCount * step);
	}


	mySphereData[KE::SphereLOD::axisSphere] = data;
}
