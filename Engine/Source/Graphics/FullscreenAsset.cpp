#include "stdafx.h"
#include "FullscreenAsset.h"
#include "Graphics/Graphics.h"

#include "Engine/Source/Utility/Logging.h"


void KE::FullscreenAsset::MakeUVTransform(Graphics* aGraphics,Vector2f aUVMin, Vector2f aUVMax)
{
	UVTransform uvT;
	uvT.uvMin = aUVMin;
	uvT.uvMax = aUVMax;

	myCBuffer.MapBuffer(&uvT, sizeof(UVTransform), aGraphics->GetContext().Get());
	myCBuffer.BindForPS(9, aGraphics->GetContext().Get());
}

KE::FullscreenAsset::FullscreenAsset()
{
}

KE::FullscreenAsset::~FullscreenAsset()
{
	
}

bool KE::FullscreenAsset::Init(Graphics* aGraphics)
{
	//construct a 1x1 quad

	//setup the quad
	Vertex _vertices[4] = {
		{ -1.0f, -1.0f, 0.0f, 1.0f },
		{ -1.0f, 1.0f, 0.0f, 1.0f },
		{ 1.0f, -1.0f, 0.0f, 1.0f },
		{ 1.0f, 1.0f, 0.0f, 1.0f }
	};

	_vertices[0].u = 0; _vertices[0].v = 1;
	_vertices[1].u = 0; _vertices[1].v = 0;
	_vertices[2].u = 1; _vertices[2].v = 1;
	_vertices[3].u = 1; _vertices[3].v = 0;

	unsigned int _indices[6] = { 0, 1, 2, 2, 1, 3 };

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	for (int i = 0; i < 4; i++)
	{
		vertices.push_back(_vertices[i]);
	}

	for (int i = 0; i < 6; i++)
	{
		indices.push_back(_indices[i]);
	}


	{
		//assign vertex buffer
		D3D11_BUFFER_DESC vertexBufferDesc = {};
		vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		vertexBufferDesc.ByteWidth = sizeof(Vertex) * (UINT)vertices.size();
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vertexData = { 0 };
		vertexData.pSysMem = vertices.data();
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		HRESULT result = aGraphics->GetDevice()->CreateBuffer(&vertexBufferDesc, &vertexData,
			&myMesh.myVertexBuffer);

		if (FAILED(result))
		{
			KE_ERROR("Failed to create vertex buffer for fullscreenasset.");
			return false;

			//todo: error handling
		}
	}

	{
		//assign index buffer
		D3D11_BUFFER_DESC indexBufferDesc = { 0 };
		indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		indexBufferDesc.ByteWidth = sizeof(unsigned int) * (UINT)indices.size();
		indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexBufferDesc.CPUAccessFlags = 0;
		indexBufferDesc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA indexData = { 0 };
		indexData.pSysMem = indices.data();

		HRESULT result = aGraphics->GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &myMesh.myIndexBuffer);

		if (FAILED(result))
		{
			KE_ERROR("Failed to create index buffer for fullscreenasset.");
			return false;
			//todo: error handling
		}
	}

	{
		//create cbuffer
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.ByteWidth = sizeof(UVTransform);
		bufferDesc.StructureByteStride = 0u;

		myCBuffer.Init(aGraphics->GetDevice(), &bufferDesc);
	}

	myMesh.myVertices = vertices;
	myMesh.myIndices = indices;

	return true;
}


void KE::FullscreenAsset::Render(Graphics* aGraphics, const Vector2f& uvMin, const Vector2f& uvMax)
{
	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0u;

	MakeUVTransform(aGraphics, uvMin, uvMax);

	aGraphics->GetContext()->PSSetShaderResources(0u, 1u, myTexture->myShaderResourceView.GetAddressOf());
	// ----------------- END TEXTURE -----------------


	// ----------------- VERTEX BUFFER -----------------
	aGraphics->GetContext()->IASetVertexBuffers(0u, 1u, myMesh.myVertexBuffer.GetAddressOf(), &stride, &offset);
	// ----------------- END VERTEX BUFFER -----------------


	// ----------------- INDEX BUFFER -----------------
	aGraphics->GetContext()->IASetIndexBuffer(myMesh.myIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
	// ----------------- END INDEX BUFFER -----------------

	// ----------------- VERTEX SHADER -----------------
	aGraphics->GetContext()->VSSetShader(myVertexShader->GetShader(), nullptr, 0u);
	// ----------------- END VERTEX SHADER -----------------


	// ----------------- INPUT VERTEX LAYOUT -----------------
	aGraphics->GetContext()->IASetInputLayout(myVertexShader->GetInputLayout());
	// ----------------- END INPUT VERTEX LAYOUT -----------------


	// ----------------- PIXEL SHADER -----------------
	aGraphics->GetContext()->PSSetShader(myPixelShader->GetShader(), nullptr, 0u);
	// ----------------- END PIXEL SHADER -----------------





	// ----------------- TOPOLOGY -----------------
	// Set primitive topology to triangle list (groups of 3 vertices)
	aGraphics->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// ----------------- END TOPOLOGY -----------------

	//
	aGraphics->DrawIndexed(myMesh.GetIndexCount(), 0u, 0u);
}

void KE::FullscreenAsset::Render(Graphics* aGraphics, ID3D11ShaderResourceView* aShaderResourceView, VertexShader* aVS, PixelShader* aPS, const Vector2f& uvMin, const Vector2f& uvMax)
{
	MakeUVTransform(aGraphics, uvMin, uvMax);

	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0u;

	aGraphics->GetContext()->PSSetShaderResources(0u, 1u, &aShaderResourceView);
	aGraphics->GetContext()->IASetVertexBuffers(0u, 1u, myMesh.myVertexBuffer.GetAddressOf(), &stride, &offset);
	aGraphics->GetContext()->IASetIndexBuffer(myMesh.myIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
	aGraphics->GetContext()->VSSetShader(aVS ? aVS->GetShader() : myVertexShader->GetShader(), nullptr, 0u);
	aGraphics->GetContext()->IASetInputLayout(aVS ? aVS->GetInputLayout() : myVertexShader->GetInputLayout());
	aGraphics->GetContext()->PSSetShader(aPS ? aPS->GetShader() : myPixelShader->GetShader(), nullptr, 0u);
	aGraphics->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	aGraphics->DrawIndexed(myMesh.GetIndexCount(), 0u, 0u);
}

void KE::FullscreenAsset::RenderWithoutSRV(Graphics* aGraphics, VertexShader* aVS, PixelShader* aPS, const Vector2f& uvMin, const Vector2f& uvMax)
{
	MakeUVTransform(aGraphics, uvMin, uvMax);

	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0u;

	aGraphics->GetContext()->IASetVertexBuffers(0u, 1u, myMesh.myVertexBuffer.GetAddressOf(), &stride, &offset);
	aGraphics->GetContext()->IASetIndexBuffer(myMesh.myIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
	aGraphics->GetContext()->VSSetShader(aVS ? aVS->GetShader() : myVertexShader->GetShader(), nullptr, 0u);
	aGraphics->GetContext()->IASetInputLayout(aVS ? aVS->GetInputLayout() : myVertexShader->GetInputLayout());
	aGraphics->GetContext()->PSSetShader(aPS ? aPS->GetShader() : myPixelShader->GetShader(), nullptr, 0u);
	aGraphics->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	aGraphics->DrawIndexed(myMesh.GetIndexCount(), 0u, 0u);
}
