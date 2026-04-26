#include "stdafx.h"
#include "DeferredLightManager.h"

#include "Engine/Source/Graphics/ShaderLoader.h"
#include "Graphics/Primitives/SpherePrimitive.h"
#include "Utility/Logging.h"
#include "Engine/Source/Graphics/Graphics.h"
#include "Engine\Source\Graphics\Texture\TextureLoader.h"
#include "Engine\Source\Graphics\Texture\Texture.h"

namespace KE
{
	DeferredLightManager::DeferredLightManager()
	{
		myPointLights.reserve(MAX_LIGHT_VECTOR_CAPACITY);
		mySpotLights.reserve(MAX_LIGHT_VECTOR_CAPACITY);
	}

	void DeferredLightManager::InitShadowCamera(const Vector2i aSize, const float aNear, const float aFar)
	{		
		//												aLeft,  aRight, aTop,  aBottom, aNear, aFar
		myDirectionalLightCamera.SetOrthographic((float)aSize.x, (float)aSize.y, aNear, aFar);
	}

	HRESULT DeferredLightManager::Init(ID3D11Device* aDevice, ShaderLoader* aShaderLoader, const Vector2i aSize)
	{
		myLightVS = aShaderLoader->GetVertexShader(SHADER_LOAD_PATH + myLightVSName);

		myDirectionalLightPS = aShaderLoader->GetPixelShader(SHADER_LOAD_PATH + myDirectionalLightPSName);
		myPointLightPS = aShaderLoader->GetPixelShader(SHADER_LOAD_PATH + myPointLightPSName);
		mySpotLightPS = aShaderLoader->GetPixelShader(SHADER_LOAD_PATH + mySpotLightPSName);

		myDevice = aDevice;

		// Shadow Stuff
		//myDepthBuffer.Init(aDevice, aSize);
		myDepthBuffer.Init(aDevice, {16384, 16384});

		InitShadowCamera(Vector2i( 425, 275), -1000.0f, 500.0f);
		myDirectionalLightCamera.transform.SetPosition({0.0f, 0.0f, 0.0f});
		 
		


		// Setup buffers
		D3D11_BUFFER_DESC cbd = {};
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.StructureByteStride = 0u;

		// ------ Directional Light ------ //
		{
			cbd.ByteWidth = sizeof(DirectionalLightData);

			const HRESULT result = myDirectionalLightBuffer.Init(aDevice, &cbd);
			if (FAILED(result))
			{
				KE_ERROR("Failed to create Directional Light constant buffer!");
				return result;
			}
		}


		// ------ Point Light ------ //
		{
			cbd.ByteWidth = sizeof(PointLightData);

			const HRESULT result = myPointLightBuffer.Init(aDevice, &cbd);
			if (FAILED(result))
			{
				KE_ERROR("Failed to create Point Light constant buffer!");
				return result;
			}
		}

		// ------ Spot Light ------ //
		{
			cbd.ByteWidth = sizeof(SpotLightData);

			const HRESULT result = mySpotLightbuffer.Init(aDevice, &cbd);
			if (FAILED(result))
			{
				KE_ERROR("Failed to create Spot Light constant buffer!");
				return result;
			}
		}

		{
			cbd.ByteWidth = sizeof(ConstantBuffer);

			const HRESULT result = myTransformBuffer.Init(aDevice, &cbd);
			if (FAILED(result))
			{
				KE_ERROR("Failed to create Transform constant buffer!");
				return result;
			}
		}


		// ------ Initiate Meshes ------ //
		{
			// Quad
			{
				// --- Vertices --- //

				Vertex& v1 = myQuad.myVertices.emplace_back();
				v1.x = -1.0f;
				v1.y = -1.0f;
				v1.w = 1.0f;

				Vertex& v2 = myQuad.myVertices.emplace_back();
				v2.x = -1.0f;
				v2.y = +1.0f;
				v2.w = 1.0f;

				Vertex& v3 = myQuad.myVertices.emplace_back();
				v3.x = +1.0f;
				v3.y = -1.0f;
				v3.w = 1.0f;

				Vertex& v4 = myQuad.myVertices.emplace_back();
				v4.x = 1.0f;
				v4.y = 1.0f;
				v4.w = 1.0f;


				// --- Indices --- //

				// 0----1
				// |  / |
				// | /  |
				// 2----3

				myQuad.myIndices.push_back(0);
				myQuad.myIndices.push_back(1);
				myQuad.myIndices.push_back(2);

				myQuad.myIndices.push_back(2);
				myQuad.myIndices.push_back(1);
				myQuad.myIndices.push_back(3);


				// Buffer creation
				HRESULT result;
				{
					// Create vertex buffer
					D3D11_BUFFER_DESC vertexBufferDesc = {};
					vertexBufferDesc.ByteWidth = sizeof(Vertex) * (int)myQuad.myVertices.size();
					vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
					vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
					vertexBufferDesc.MiscFlags = 0;
					D3D11_SUBRESOURCE_DATA vertexData = {0};
					vertexData.pSysMem = myQuad.myVertices.data();
					result = aDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &myQuad.myVertexBuffer);
					if (FAILED(result))
					{
						return result;
					}
				}

				{
					// Create index buffer
					D3D11_BUFFER_DESC indexBufferDesc = {};
					indexBufferDesc.ByteWidth = sizeof(unsigned int) * (int)myQuad.myIndices.size();
					indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
					indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
					D3D11_SUBRESOURCE_DATA indexData = {0};
					indexData.pSysMem = myQuad.myIndices.data();
					// Create the index buffer.
					result = aDevice->CreateBuffer(&indexBufferDesc, &indexData,
					                               &myQuad.myIndexBuffer);
					if (FAILED(result))
					{
						return result;
					}
				}
			}


			// Sphere
			{
				SpherePrimitive sphere;
				mySphere.myIndices = sphere.GetIndices();

				auto& vertices = sphere.GetVertices();
				for (size_t i = 0; i < vertices.size(); i++)
				{
					// This is ASS, primitives should use the same vertex type as Mesh - Olle
					Vertex v;
					v.x = vertices[i].x;
					v.y = vertices[i].y;
					v.z = vertices[i].z;
					v.w = 1.0f;

					mySphere.myVertices.push_back(v);
				}

				// Buffer creation
				HRESULT result;
				{
					// Create vertex buffer
					D3D11_BUFFER_DESC vertexBufferDesc = {};
					vertexBufferDesc.ByteWidth = sizeof(Vertex) * (int)mySphere.myVertices.size();
					vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
					vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					vertexBufferDesc.CPUAccessFlags = 0;
					vertexBufferDesc.MiscFlags = 0;
					D3D11_SUBRESOURCE_DATA vertexData = {0};
					vertexData.pSysMem = mySphere.myVertices.data();
					vertexData.SysMemPitch = 0;
					vertexData.SysMemSlicePitch = 0;

					result = aDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &mySphere.myVertexBuffer);
					if (FAILED(result))
					{
						return result;
					}
				}

				{
					// Create index buffer
					D3D11_BUFFER_DESC indexBufferDesc = {};
					indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
					indexBufferDesc.ByteWidth = sizeof(unsigned int) * (int)mySphere.myIndices.size();
					indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
					D3D11_SUBRESOURCE_DATA indexData = {0};
					indexBufferDesc.CPUAccessFlags = 0;
					indexBufferDesc.MiscFlags = 0;

					indexData.pSysMem = mySphere.myIndices.data();
					// Create the index buffer.
					result = aDevice->CreateBuffer(&indexBufferDesc, &indexData,
					                               &mySphere.myIndexBuffer);
					if (FAILED(result))
					{
						return result;
					}
				}
			}
		}

		return S_OK;
	}


	void DeferredLightManager::PrepareShadowPass(Graphics* aGraphics)
	{
		// Set Camera
		// Maybe not needed
		
		myDepthBuffer.Clear(aGraphics->GetContext().Get());
		float arr[4] = { 0.0f,0.0f,0.0f,0.0f };
		aGraphics->GetRenderTarget(6)->Clear(arr);

		aGraphics->SetView(myDirectionalLightCamera.GetViewMatrix());
		aGraphics->SetProjection(myDirectionalLightCamera.GetProjectionMatrix());
		myDirectionalLightCamera.transform.SetDirection({ -myDirectionalLight.myDirection.x, -myDirectionalLight.myDirection.y, -myDirectionalLight.myDirection.z });


		ID3D11ShaderResourceView* const nullSRV[1] = { NULL };
		aGraphics->GetContext()->PSSetShaderResources(14, 1, nullSRV);

		myDepthBuffer.SetAsActiveTarget(aGraphics->GetContext().Get());
		//myDepthBuffer.SetAsActiveTarget(aGraphics->GetContext().Get(), aGraphics->GetRenderTarget(6));
	}

	int DeferredLightManager::Render(Graphics* aGraphics)
	{
		int drawCalls = 0;

		ComPtr<ID3D11DeviceContext> context = aGraphics->GetContext();

		// Shadow Directional Light
		myDepthBuffer.SetAsResourceOnSlot(14, aGraphics->GetContext().Get());

		// TODO Maybe won't be needed

		constexpr UINT stride = sizeof(Vertex);
		constexpr UINT offset = 0u;

		// Draw Mesh
		context->IASetVertexBuffers(0u, 1u, myQuad.myVertexBuffer.GetAddressOf(), &stride, &offset);
		context->IASetIndexBuffer(myQuad.myIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);

		// --- Set Vertex Shader --- //
		context->VSSetShader(myLightVS->GetShader(), nullptr, 0u);
		context->IASetInputLayout(myLightVS->GetInputLayout());

		// ------ Directional Light ------ //
		{
			// Setup transform buffer
			ConstantBuffer transformBuffer;
			transformBuffer.positionAndRange[0] = 0.0f;
			transformBuffer.positionAndRange[1] = 0.0f;
			transformBuffer.positionAndRange[2] = 0.0f;
			transformBuffer.positionAndRange[3] = 1.0f;
			transformBuffer.isDirectional = TRUE;

			// Update Buffer and bind
			myTransformBuffer.MapBuffer(&transformBuffer, sizeof(ConstantBuffer), context.Get());
			myTransformBuffer.BindForVS(DEFERRED_LIGHT_BUFFER_SLOT, context.Get());

			//myDirectionalLight.myDirectionalLightCamera = myDirectionalLightCamera.transform.GetMatrix();
			myDirectionalLight.myDirectionalLightCamera = myDirectionalLightCamera.GetViewMatrix() * myDirectionalLightCamera.GetProjectionMatrix();
			myDirectionalLightBuffer.MapBuffer(&myDirectionalLight, sizeof(DirectionalLightData), context.Get());
			myDirectionalLightBuffer.BindForPS(DEFERRED_LIGHT_BUFFER_SLOT, context.Get());

			// Set Pixel Shader
			context->PSSetShader(myDirectionalLightPS->GetShader(), nullptr, 0u);

			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			context->DrawIndexed(myQuad.GetIndexCount(), 0u, 0u);
			drawCalls++;
		}

		aGraphics->SetBlendState(eBlendStates::AdditiveBlend);
		aGraphics->SetRasterizerState(eRasterizerStates::FrontfaceCulling);
		aGraphics->SetDepthStencilState(eDepthStencilStates::ReadOnlyGreater);

		// ------ Point Lights ------ //

		context->PSSetShader(myPointLightPS->GetShader(), nullptr, 0u);

		for (PointLightData lightData : myPointLights)
		{
			// Setup transform buffer
			ConstantBuffer transformBuffer;

			transformBuffer.positionAndRange[0] = lightData.myPosition.x;
			transformBuffer.positionAndRange[1] = lightData.myPosition.y;
			transformBuffer.positionAndRange[2] = lightData.myPosition.z;
			transformBuffer.positionAndRange[3] = lightData.myRange;
			transformBuffer.isDirectional = FALSE;

			// Update Buffer and bind
			myTransformBuffer.MapBuffer(&transformBuffer, sizeof(ConstantBuffer), context.Get());
			myTransformBuffer.BindForVS(DEFERRED_LIGHT_BUFFER_SLOT, context.Get());

			myPointLightBuffer.MapBuffer(&lightData, sizeof(PointLightData), context.Get());
			myPointLightBuffer.BindForPS(DEFERRED_LIGHT_BUFFER_SLOT, context.Get());

			// Draw Mesh
			context->IASetIndexBuffer(mySphere.myIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
			context->IASetVertexBuffers(0u, 1u, mySphere.myVertexBuffer.GetAddressOf(), &stride, &offset);

			context->DrawIndexed(mySphere.GetIndexCount(), 0u, 0u);
			drawCalls++;
		}

		// ------ Spot Lights ------ //

		context->PSSetShader(mySpotLightPS->GetShader(), nullptr, 0u);

		for (SpotLightData lightData : mySpotLights)
		{
			// Setup transform buffer
			ConstantBuffer transformBuffer;
			transformBuffer.positionAndRange[0] = lightData.myPosition.x;
			transformBuffer.positionAndRange[1] = lightData.myPosition.y;
			transformBuffer.positionAndRange[2] = lightData.myPosition.z;
			transformBuffer.positionAndRange[3] = lightData.myRange;
			transformBuffer.isDirectional = FALSE;

			// Update Buffer and bind
			myTransformBuffer.MapBuffer(&transformBuffer, sizeof(ConstantBuffer), context.Get());
			myTransformBuffer.BindForVS(DEFERRED_LIGHT_BUFFER_SLOT, context.Get());

			mySpotLightbuffer.MapBuffer(&lightData, sizeof(SpotLightData), context.Get());
			mySpotLightbuffer.BindForPS(LIGHT_BUFFER_PS_SLOT, context.Get());

			// Draw Mesh
			context->IASetIndexBuffer(mySphere.myIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0u);
			context->IASetVertexBuffers(0u, 1u, mySphere.myVertexBuffer.GetAddressOf(), &stride, &offset);

			context->DrawIndexed(mySphere.GetIndexCount(), 0u, 0u);
			drawCalls++;
		}

		aGraphics->SetBlendState(eBlendStates::Disabled);
		aGraphics->SetDepthStencilState(eDepthStencilStates::Write);
		aGraphics->SetRasterizerState(eRasterizerStates::BackfaceCulling);

		// Reset
		aGraphics->GetContext().Get()->PSSetShaderResources(14, 0, nullptr);

		return drawCalls;
	}

	void DeferredLightManager::Reset()
	{
		myPointLights.clear();
		mySpotLights.clear();

		myFreePointLights.clear();
		myFreeSpotLights.clear();
	}

	// TODO Importer needs to recieve a LightData& instead of a LightComponentData&
	LightData* DeferredLightManager::CreateLightData(const eLightType aLightType)
	{
		switch (aLightType)
		{
			case eLightType::Directional:
			{
				return &myDirectionalLight;
			}
			case eLightType::Point:
			{
				if (myFreePointLights.size() > 0)
				{
					auto* light = myFreePointLights.back();
					myFreePointLights.pop_back();
					return light;
				}
				return &myPointLights.emplace_back();
			}
			case eLightType::Spot:
			{
				if (myFreeSpotLights.size() > 0)
				{
					auto* light = myFreeSpotLights.back();
					myFreeSpotLights.pop_back();
					return light;
				}
				return &mySpotLights.emplace_back();
			}
		}

		return nullptr;
	}

	void DeferredLightManager::RemoveLightData(eLightType aType, LightData* aLightData)
	{
		switch(aType)
		{
		case eLightType::Point:
		{
			auto* light = static_cast<PointLightData*>(aLightData);
			light->isActive = false;
			myFreePointLights.push_back(light);
			break;
		}
		case eLightType::Spot:
		{
			auto* light = static_cast<SpotLightData*>(aLightData);
			light->isActive = false;
			myFreeSpotLights.push_back(light);
			break;
		}
		default: break;
		}
	}

	void DeferredLightManager::AssignCubemap(Cubemap* aCubemap)
	{
		//myCubemap = aTextureLoader->GetCubemapFromPath("Data/Assets/Materials/CubeMaps/Cubemap_test.dds");
		myCubemap = aCubemap;
	}
}
