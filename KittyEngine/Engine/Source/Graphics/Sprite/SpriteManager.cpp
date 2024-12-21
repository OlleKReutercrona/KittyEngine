#include "stdafx.h"
#include "Engine/Source/Graphics/Camera.h"

#include "SpriteManager.h"

#include "Engine/Source/Graphics/Graphics.h"
#include "Engine/Source/Graphics/ShaderLoader.h"
#include "Engine/Source/Utility/Logging.h"
#include "Engine/Source/Graphics/CBuffer.h"

namespace KE
{
	SpriteManager::SpriteManager() { }

	SpriteManager::~SpriteManager() { }

	void SpriteManager::Init(Graphics* aGraphics)
	{
		myGraphics = aGraphics;

		//create vertex buffer
		SpriteVertex vertices[4] = {
			{-1.0f, -1.0f, 0.0f, 1.0f},
			{-1.0f, 1.0f, 0.0f, 1.0f},
			{1.0f, -1.0f, 0.0f, 1.0f},
			{1.0f, 1.0f, 0.0f, 1.0f}
		};

		vertices[0].u = 0;
		vertices[0].v = 1;
		vertices[1].u = 0;
		vertices[1].v = 0;
		vertices[2].u = 1;
		vertices[2].v = 1;
		vertices[3].u = 1;
		vertices[3].v = 0;

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(SpriteVertex) * 4;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA subresourceData = {};
		subresourceData.pSysMem = vertices;

		ID3D11Device* device = myGraphics->GetDevice().Get();
		HRESULT result = device->CreateBuffer(&bufferDesc, &subresourceData, &myVertexBuffer);
		if (FAILED(result))
		{
			KE_ERROR("Failed to create sprite vertex buffer!");
			return;
		}

		//create instance buffer

		D3D11_BUFFER_DESC instanceBufferDesc = {};

		instanceBufferDesc.ByteWidth = sizeof(Sprite) * 8192;
		instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		result = device->CreateBuffer(&instanceBufferDesc, nullptr, &myInstanceBuffer);
		if (FAILED(result))
		{
			KE_ERROR("Failed to create sprite instance buffer!");
			return;
		}

		D3D11_BUFFER_DESC spriteRenderBufferDesc = {};

		spriteRenderBufferDesc.ByteWidth = sizeof(SpriteRenderBuffer);
		spriteRenderBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		spriteRenderBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		spriteRenderBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		result = device->CreateBuffer(&spriteRenderBufferDesc, nullptr, &mySpriteRenderBuffer);
		if (FAILED(result))
		{
			KE_ERROR("Failed to create sprite render buffer!");
			return;
		}

		//
		D3D11_BUFFER_DESC textRenderBufferDesc = {};

		textRenderBufferDesc.ByteWidth = sizeof(TextStylingBufferData);
		textRenderBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		textRenderBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		textRenderBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		result = device->CreateBuffer(&textRenderBufferDesc, nullptr, &myTextBuffer);
		if (FAILED(result))
		{
			KE_ERROR("Failed to create text render buffer!");
			return;
		}
		//

		//create index buffer

		unsigned int indices[6] = {0, 1, 2, 2, 1, 3};


		bufferDesc = {};
		bufferDesc.ByteWidth = sizeof(unsigned int) * 6;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		subresourceData = {};
		subresourceData.pSysMem = indices;

		result = device->CreateBuffer(&bufferDesc, &subresourceData, &myIndexBuffer);
		if (FAILED(result))
		{
			KE_ERROR("Failed to create sprite index buffer!");
			return;
		}
	}

	//

	void SpriteManager::BindBuffers(SpriteBatch& aSpriteBatch, KE::Camera* aCamera)
	{
		//set topology
		ID3D11DeviceContext* context = myGraphics->GetContext().Get();
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		aSpriteBatch;
		unsigned int instanceCount = (unsigned int)aSpriteBatch.myInstances.size();
		instanceCount;

		unsigned int strides[2];
		unsigned int offsets[2];
		ID3D11Buffer* bufferPointers[2];


		strides[0] = sizeof(SpriteVertex);
		strides[1] = sizeof(Sprite);

		offsets[0] = 0;
		offsets[1] = 0;

		bufferPointers[0] = myVertexBuffer;
		bufferPointers[1] = myInstanceBuffer;


		//

		SpriteRenderBuffer spriteRenderBuffer = {};
		spriteRenderBuffer.displayMode = (int)aSpriteBatch.myData.myMode;
		spriteRenderBuffer.uvRegion.min.u = aSpriteBatch.myData.myUVs.uvMin[0];
		spriteRenderBuffer.uvRegion.min.v = aSpriteBatch.myData.myUVs.uvMin[1];
		spriteRenderBuffer.uvRegion.max.u = aSpriteBatch.myData.myUVs.uvMax[0];
		spriteRenderBuffer.uvRegion.max.v = aSpriteBatch.myData.myUVs.uvMax[1];
		spriteRenderBuffer.myEffectType = (int)aSpriteBatch.myData.myEffectType;
		spriteRenderBuffer.flipX = aSpriteBatch.myData.flipX;
		spriteRenderBuffer.flipY = aSpriteBatch.myData.flipY;

		switch (aSpriteBatch.myData.myMode)
		{
			case SpriteBatchMode::Default:
			{
				spriteRenderBuffer.completeTransform = myGraphics->myView * myGraphics->myProjection;
				break;
			}
			case SpriteBatchMode::Billboard:
			{
				spriteRenderBuffer.completeTransform = aCamera->transform.GetMatrix();
				break;
			}
			case SpriteBatchMode::Screen:
			{
				spriteRenderBuffer.completeTransform = DirectX::XMMatrixIdentity();

				spriteRenderBuffer.completeTransform *= DirectX::XMMatrixScaling(
					1.0f / myGraphics->GetRenderWidth(),
					1.0f / myGraphics->GetRenderHeight(),
					1.0f
				);

				break;
			}
			case SpriteBatchMode::ScreenText:
			{
				const float rWidth = (float)myGraphics->GetRenderWidth(), rHeight = (float)myGraphics->GetRenderHeight();
				Camera camera;
				camera.transform.SetPosition({ rWidth / 2.0f, rHeight / 2.0f, 0.0f });

				camera.SetOrthographic(rWidth, rHeight, 0.0f, 1.0f);
				spriteRenderBuffer.completeTransform = camera.GetViewMatrix() * camera.GetProjectionMatrix();

				TextStylingBufferData textStylingBufferData = {};
				textStylingBufferData.textColour = aSpriteBatch.myTextStyling.text.colour;
				textStylingBufferData.strokeColour = aSpriteBatch.myTextStyling.stroke.colour;
				textStylingBufferData.strokeSize = aSpriteBatch.myTextStyling.stroke.width;
				textStylingBufferData.strokeSoftness = aSpriteBatch.myTextStyling.stroke.softness;



				D3D11_MAPPED_SUBRESOURCE textStylingMappedResource = {};
				context->Map(myTextBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &textStylingMappedResource);
				memcpy(
					textStylingMappedResource.pData,
					&textStylingBufferData,
					sizeof(TextStylingBufferData)
				);
				context->Unmap(myTextBuffer, 0);

				myGraphics->GetContext()->PSSetConstantBuffers(8, 1, &myTextBuffer);
				myGraphics->GetContext()->VSSetConstantBuffers(8, 1, &myTextBuffer);

				break;
			}
		}

		D3D11_MAPPED_SUBRESOURCE spriteRenderMappedResource = {};
		context->Map(mySpriteRenderBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &spriteRenderMappedResource);
		memcpy(
			spriteRenderMappedResource.pData,
			&spriteRenderBuffer,
			sizeof(SpriteRenderBuffer)
		);
		context->Unmap(mySpriteRenderBuffer, 0);


		//TODO! define slot somewhere else :3
		myGraphics->GetContext()->PSSetConstantBuffers(3, 1, &mySpriteRenderBuffer);
		myGraphics->GetContext()->VSSetConstantBuffers(3, 1, &mySpriteRenderBuffer);


		//now, actually make my instancebuffer data the data from the spritebatch.

		D3D11_MAPPED_SUBRESOURCE mappedResource = {};
		context->Map(myInstanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		memcpy(
			mappedResource.pData,
			aSpriteBatch.myInstances.data(),
			sizeof(Sprite) * instanceCount
		);
		context->Unmap(myInstanceBuffer, 0);

		myGraphics->GetContext()->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
		myGraphics->GetContext()->IASetIndexBuffer(myIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		myGraphics->GetContext()->IASetInputLayout(myGraphics->GetShaderLoader().GetVertexShader(SHADER_LOAD_PATH "Sprite_VS.cso")->GetInputLayout());
		myGraphics->GetContext()->VSSetShader(myGraphics->GetShaderLoader().GetVertexShader(SHADER_LOAD_PATH "Sprite_VS.cso")->GetShader(), nullptr, 0);
		myGraphics->GetContext()->PSSetShader(myGraphics->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH "Sprite_PS.cso")->GetShader(), nullptr, 0);
	}

	void SpriteManager::RenderBatch(SpriteBatch& aSpriteBatch)
	{
		ID3D11ShaderResourceView* srv = aSpriteBatch.myData.myTexture->myShaderResourceView.Get();
		myGraphics->GetContext()->PSSetShaderResources(0, 1, &srv);

		if (aSpriteBatch.myCustomVS)
		{
			myGraphics->GetContext()->VSSetShader(aSpriteBatch.myCustomVS->GetShader(), nullptr, 0);
		}
		if (aSpriteBatch.myCustomPS)
		{
			myGraphics->GetContext()->PSSetShader(aSpriteBatch.myCustomPS->GetShader(), nullptr, 0);
		}

		myGraphics->GetContext()->DrawIndexedInstanced(6, (unsigned int)aSpriteBatch.myInstances.size(), 0, 0, 0);
	}

	void SpriteManager::Render(KE::Camera* aCamera, eRenderLayers aLayer)
	{
		for (SpriteBatch* batch : mySpriteBatches)
		{
			if (batch->myRenderLayer != aLayer) { continue; }
			BindBuffers(*batch, aCamera);
			RenderBatch(*batch);
		}
	}

	void SpriteManager::BeginFrame()
	{
		mySpriteBatches.clear();
		myScreenSpriteBatches.clear();
	}

	void SpriteManager::EndFrame() { }

	void SpriteManager::QueueSpriteBatch(SpriteBatch* aSpriteBatch)
	{

		mySpriteBatches.push_back(aSpriteBatch);

	}

}
