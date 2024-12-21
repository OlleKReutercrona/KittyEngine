#include "stdafx.h"
#include "DebugLine.h"
#include <fstream>
#include <d3d11.h>
#include <assert.h>

#include "Shader.h"



namespace KE
{

	DebugLine::DebugLine()
	{
	}

	DebugLine::~DebugLine()
	{
	}

	bool DebugLine::Initialize(LineRenderData aRenderData, ID3D11Device* aDevice, bool isStatic)
	{
		myDevice = aDevice;

		myIsStatic = isStatic;

		myIndices = (int)aRenderData.myIndices.size();
		myVertices = aRenderData.myVertices;

		if (myIndices > 0)
		{
			HRESULT result;
			{
				// Create vertex buffer
				D3D11_BUFFER_DESC vertexBufferDesc = {};
				vertexBufferDesc.ByteWidth = sizeof(LineVertex) * (int)aRenderData.myVertices.size();
				vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
				vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				vertexBufferDesc.MiscFlags = 0;
				D3D11_SUBRESOURCE_DATA vertexData = { 0 };
				vertexData.pSysMem = aRenderData.myVertices.data();
				result = myDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &myVertexBuffer);
				if (FAILED(result))
				{
					return false;
				}
			}

			{
				myIndicesVector = aRenderData.myIndices;

				// Create index buffer
				D3D11_BUFFER_DESC indexBufferDesc = {};
				indexBufferDesc.ByteWidth = sizeof(unsigned int) * (int)aRenderData.myIndices.size();
				indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
				indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				D3D11_SUBRESOURCE_DATA indexData = { 0 };
				indexData.pSysMem = aRenderData.myIndices.data();
				// Create the index buffer.
				result = myDevice->CreateBuffer(&indexBufferDesc, &indexData,
					&myIndexBuffer);
				if (FAILED(result))
				{
					return false;
				}
			}
		}
		

		myPixelShader = aRenderData.myPS;
		myVertexShader = aRenderData.myVS;

		aRenderData.myLayout =
		{
				{ "SV_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		return true;
	}

	void DebugLine::Render(ID3D11DeviceContext* context, VertexShader* aVSOverride, PixelShader* aPSOverride)
	{
		if (myVertices.size() <= 0) { return; }

		if (!myIsStatic)
		{
			SetVertexes();
			SetIndices();
		}

		VertexShader* vs = aVSOverride ? aVSOverride : myVertexShader;
		PixelShader* ps = aPSOverride ? aPSOverride : myPixelShader;

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		context->IASetInputLayout(myVertexShader->GetInputLayout());
		unsigned int stride = sizeof(LineVertex);
		unsigned int offset = 0;
		context->IASetVertexBuffers(0, 1, myVertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(myIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		context->VSSetShader(vs->GetShader(), nullptr, 0);
		context->PSSetShader(ps->GetShader(), nullptr, 0);
		context->DrawIndexed(myIndices, 0, 0);
	}

	void DebugLine::SetVertexes()
	{
		// Create vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.ByteWidth = sizeof(LineVertex) * (int)myVertices.size();
		vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexBufferDesc.MiscFlags = 0;
		D3D11_SUBRESOURCE_DATA vertexData = { 0 };
		vertexData.pSysMem = myVertices.data();
		HRESULT result = myDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &myVertexBuffer);
		if (FAILED(result))
		{
			assert(false);
			OutputDebugString(L"SetVertex Failed");
		}
	}

	void DebugLine::SetIndices()
	{
		// Create index buffer
		D3D11_BUFFER_DESC indexBufferDesc = {};
		indexBufferDesc.ByteWidth = sizeof(int) * (int)myIndicesVector.size();
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		D3D11_SUBRESOURCE_DATA indexData = { 0 };
		indexData.pSysMem = myIndicesVector.data();
		// Create the index buffer.
		HRESULT result = myDevice->CreateBuffer(&indexBufferDesc, &indexData, &myIndexBuffer);
		if (FAILED(result))
		{
			assert(false);
			return;
		}
	}

	LineVertex* DebugLine::GetVertex(const int anID)
	{
		if (anID >= myVertices.size() || anID < 0) return nullptr;

		return &myVertices[anID];

	}

	void DebugLine::AddLine(const LineVertex aStartPoint, const LineVertex anEndPoint)
	{
		myIndicesVector.push_back((int)myVertices.size());
		++myIndices;
		myVertices.push_back(aStartPoint);

		myIndicesVector.push_back((int)myVertices.size());
		++myIndices;
		myVertices.push_back(anEndPoint);

		if (!isReady)
		{
			CreateBuffers();
		}

	}

	void DebugLine::DrawCube(const Vector3f& aPosition, const Vector3f& aDimensions, const Vector4f& aColour)
	{
		auto vertices = myCube.GetVertices();
		auto indices = myCube.GetIndices();

		int startIndex = static_cast<int>(myVertices.size());
		for (size_t i = 0; i < 8; i++)
		{
			myVertices.emplace_back(LineVertex({ vertices[i].x, vertices[i].y, vertices[i].z, vertices[i].w }, aColour) * aDimensions + aPosition);
		}


		for (size_t i = 0; i < 24; i++)
		{
			myIndicesVector.push_back(indices[i] + startIndex);
			myIndices++;
		}
	}

	void DebugLine::DrawCube(const Transform& aTransform, const Vector3f& aDimensions, const Vector4f& aColour)
	{
		auto vertices = myCube.GetVertices();
		auto indices = myCube.GetIndices();

		int startIndex = static_cast<int>(myVertices.size());
		for (size_t i = 0; i < 8; i++)
		{
			Vector4f position = { vertices[i].x, vertices[i].y, vertices[i].z, vertices[i].w };
			position.x *= aDimensions.x;
			position.y *= aDimensions.y;
			position.z *= aDimensions.z;

			position = position * aTransform.GetCUMatrix();
			myVertices.emplace_back(LineVertex(position, aColour));
		}


		for (size_t i = 0; i < 24; i++)
		{
			myIndicesVector.push_back(indices[i] + startIndex);
			myIndices++;
		}
	}

	void DebugLine::DrawSphere(const Vector3f& aPosition, const float aRadius, const Vector4f& aColour)
	{
		auto vertices = mySphere.GetVertices(KE::SphereLOD::axisSphere);
		auto indices = mySphere.GetIndices(KE::SphereLOD::axisSphere);

		int startIndex = static_cast<int>(myVertices.size());
		for (size_t i = 0; i < vertices.size(); i++)
		{
			myVertices.push_back((LineVertex({vertices[i].x, vertices[i].y, vertices[i].z, vertices[i].w}, aColour) * aRadius) + aPosition);
		}

		for (size_t i = 0; i < indices.size(); i++)
		{
			myIndicesVector.push_back(indices[i] + startIndex);
			myIndices++;
		}
	}

	void DebugLine::DrawSphere(const Transform& aTransform, const float aRadius, const Vector4f& aColour)
	{
		auto vertices = mySphere.GetVertices(KE::SphereLOD::axisSphere);
		auto indices = mySphere.GetIndices(KE::SphereLOD::axisSphere);

		Transform t = aTransform;
		t.SetPosition({0.0f, 0.0f, 0.0f});

		int startIndex = static_cast<int>(myVertices.size());
		for (size_t i = 0; i < vertices.size(); i++)
		{
			

			Vector4f position = { vertices[i].x, vertices[i].y, vertices[i].z, vertices[i].w };
			position *= aRadius;
			position = t.GetCUMatrix() * position;

			position += aTransform.GetPosition();

			myVertices.push_back((LineVertex(position, aColour)));
		}

		for (size_t i = 0; i < indices.size(); i++)
		{
			myIndicesVector.push_back(indices[i] + startIndex);
			myIndices++;
		}


	}

	void DebugLine::DrawCone(const Vector3f& aPosition, const Vector3f& aDirection, const float aLength, const float anOuterRadius, const float anInnerRadius, const Vector4f& aColour)
	{
		int sourceIndex = static_cast<int>(myVertices.size());

		myIndicesVector.push_back(sourceIndex);
		myVertices.push_back(LineVertex(aPosition.x, aPosition.y, aPosition.z, 1.0f));
		myIndices++;

		LineVertex to(aPosition.x, aPosition.y, aPosition.z, 1.0f);
		auto dir = aDirection * aLength;
		to =  to + dir;
		myIndicesVector.push_back(static_cast<int>(myVertices.size()));
		myVertices.push_back(to);
		myIndices++;

		myIndicesVector.push_back(sourceIndex);
		int v0 = static_cast<int>(myVertices.size());
		myIndicesVector.push_back(v0);
		myVertices.push_back(LineVertex(to.position.x + anOuterRadius, to.position.y, to.position.z, 1.0f));

		myIndicesVector.push_back(sourceIndex);
		int v1 = static_cast<int>(myVertices.size());
		myIndicesVector.push_back(v1);
		myVertices.push_back(LineVertex(to.position.x - anOuterRadius, to.position.y, to.position.z, 1.0f));

		myIndicesVector.push_back(sourceIndex);
		int v2 = static_cast<int>(myVertices.size());
		myIndicesVector.push_back(v2);
		myVertices.push_back(LineVertex(to.position.x, to.position.y, to.position.z + anOuterRadius, 1.0f));

		myIndicesVector.push_back(sourceIndex);
		int v3 = static_cast<int>(myVertices.size());
		myIndicesVector.push_back(v3);
		myVertices.push_back(LineVertex(to.position.x, to.position.y, to.position.z - anOuterRadius, 1.0f));

		myIndices += 8;

		myIndicesVector.push_back(v0);
		myIndicesVector.push_back(v2);

		myIndicesVector.push_back(v2);
		myIndicesVector.push_back(v1);

		myIndicesVector.push_back(v1);
		myIndicesVector.push_back(v3);

		myIndicesVector.push_back(v3);
		myIndicesVector.push_back(v0);

		myIndices += 8;

		//myIndicesVector.push_back(sourceIndex);
		//myIndicesVector.push_back(static_cast<int>(myVertices.size()));
		//myVertices.push_back(LineVertex(dir.x + anOuterRadius, dir.y, dir.z + anOuterRadius, 1.0f));

		//myIndicesVector.push_back(sourceIndex);
		//myIndicesVector.push_back(static_cast<int>(myVertices.size()));
		//myVertices.push_back(LineVertex(dir.x - anOuterRadius, dir.y, dir.z + anOuterRadius, 1.0f));

		//myIndicesVector.push_back(sourceIndex);
		//myIndicesVector.push_back(static_cast<int>(myVertices.size()));
		//myVertices.push_back(LineVertex(dir.x + anOuterRadius, dir.y, dir.z + anOuterRadius, 1.0f));

		//myIndicesVector.push_back(sourceIndex);
		//myIndicesVector.push_back(static_cast<int>(myVertices.size()));
		//myVertices.push_back(LineVertex(dir.x - anOuterRadius, dir.y, dir.z + anOuterRadius, 1.0f));

		//myIndices += 8;

		//myIndicesVector.push_back(sourceIndex);
		//myIndicesVector.push_back(static_cast<int>(myVertices.size()));
		//myVertices.push_back(LineVertex(dir.x + anOuterRadius, dir.y, dir.z - anOuterRadius, 1.0f));

		//myIndicesVector.push_back(sourceIndex);
		//myIndicesVector.push_back(static_cast<int>(myVertices.size()));
		//myVertices.push_back(LineVertex(dir.x - anOuterRadius, dir.y, dir.z - anOuterRadius, 1.0f));

		//myIndicesVector.push_back(sourceIndex);
		//myIndicesVector.push_back(static_cast<int>(myVertices.size()));
		//myVertices.push_back(LineVertex(dir.x + anOuterRadius, dir.y, dir.z - anOuterRadius, 1.0f));

		//myIndicesVector.push_back(sourceIndex);
		//myIndicesVector.push_back(static_cast<int>(myVertices.size()));
		//myVertices.push_back(LineVertex(dir.x - anOuterRadius, dir.y, dir.z - anOuterRadius, 1.0f));

		//myIndices += 8;
	}

	void DebugLine::DrawCapsule(const Vector3f& aPosition, const float aRadius, const float aLength, const Vector4f& aColour)
	{
		auto mesh = myCapsule.GetMesh();

		auto vertices = mesh.myVertices;
		auto indices = mesh.myIndices;

		int startIndex = static_cast<int>(myVertices.size());
		float heightModifier = 0;
		for (int i = 0; i < vertices.size(); i++)
		{
			if (i >= mesh.halfWayIndex) heightModifier = aLength;

			myVertices.push_back((LineVertex({ vertices[i].x, vertices[i].y + heightModifier, vertices[i].z, vertices[i].w }, aColour) * aRadius) + aPosition);
		}

		for (int i = 0; i < indices.size(); i++)
		{
			myIndicesVector.push_back(indices[i] + startIndex);
			myIndices++;
		}

		for (int i = 0; i < 4; i++)
		{
			myIndicesVector.push_back(mesh.lengthIndices[i].x);
			myIndicesVector.push_back(mesh.lengthIndices[i].y);
			myIndices++;
			myIndices++;
		}
	}

	void DebugLine::DrawCapsule(const Transform& aTransform, const float aRadius, const float aLength, const Vector4f& aColour)
	{
		auto mesh = myCapsule.GetMesh();

		Matrix4x4f matrix = aTransform.GetCUMatrix();
		matrix(4, 1) = 0.0f;
		matrix(4, 2) = 0.0f;
		matrix(4, 3) = 0.0f;


		float height = aLength >= 0.0f ? aLength : 0.0f;

		auto vertices = mesh.myVertices;
		auto indices = mesh.myIndices;

		int startIndex = static_cast<int>(myVertices.size());
		float heightModifier = 0;
		for (int i = 0; i < vertices.size(); i++)
		{
			if (i >= mesh.halfWayIndex) heightModifier = aLength;

			Vector4f position({ vertices[i].x, vertices[i].y + heightModifier, vertices[i].z, vertices[i].w });

			position *= aRadius;

			position = position * matrix;

			position += aTransform.GetPosition();

			myVertices.push_back((LineVertex(position, aColour)));
		}

		for (int i = 0; i < indices.size(); i++)
		{
			myIndicesVector.push_back(indices[i] + startIndex);
			myIndices++;
		}

		for (int i = 0; i < 4; i++)
		{
			myIndicesVector.push_back(mesh.lengthIndices[i].x  + startIndex);
			myIndicesVector.push_back(mesh.lengthIndices[i].y  + startIndex);
			myIndices++;
			myIndices++;
		}
	}

	void DebugLine::EndFrame()
	{
		myIndicesVector.clear();
		myVertices.clear();
		isReady = false;
		myIndices = 0;
	}

	void DebugLine::CreateBuffers()
	{
		{
			// Create vertex buffer
			D3D11_BUFFER_DESC vertexBufferDesc = {};
			vertexBufferDesc.ByteWidth = sizeof(LineVertex) * (int)myVertices.size();
			vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			vertexBufferDesc.MiscFlags = 0;
			D3D11_SUBRESOURCE_DATA vertexData = { 0 };
			vertexData.pSysMem = myVertices.data();
			HRESULT result = myDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &myVertexBuffer);
			if (FAILED(result))
			{
				return;
			}
		}

		{
			// Create index buffer
			D3D11_BUFFER_DESC indexBufferDesc = {};
			indexBufferDesc.ByteWidth = sizeof(unsigned int) * (int)myIndicesVector.size();
			indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			D3D11_SUBRESOURCE_DATA indexData = { 0 };
			indexData.pSysMem = myIndicesVector.data();
			// Create the index buffer.
			HRESULT result = myDevice->CreateBuffer(&indexBufferDesc, &indexData,
				&myIndexBuffer);
			if (FAILED(result))
			{
				return;
			}
		}

		isReady = true;
	}
}