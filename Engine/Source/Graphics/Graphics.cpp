#include "stdafx.h"
#include "Graphics.h"

#include "Engine/Source/Graphics/Shader.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/ModelComponent.h"
#include "Engine/Source/Graphics/Texture/Texture.h"
#include "Engine/Source/Graphics/GraphicsConstants.h"
#include "Engine/Source/Graphics/FullscreenAsset.h"
#include "Engine/Source/Graphics/AssetLoader/AssetLoader.h"

#include "Engine/Source/Graphics/Particle/ParticleEmitter.h"
#include "Engine\Source\Utility\Logging.h"
#include "Engine/Source/Utility/DebugTimeLogger.h"

#include <dxgidebug.h>
#include <filesystem>

#include "ComponentSystem/GameObjectManager.h"
#include "Editor/Source/ImGui/ImGuiHandler.h"
#include "Editor/Source/ImGui/ImGuiHandler.h"
#include "imgui/imgui.h"
#include "Text/Text.h"
#include "Utility/EventSystem.h"
#include "Windows/Window.h"
#pragma comment(lib, "dxguid.lib")


namespace WRL = Microsoft::WRL;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

#pragma message("----------------------------")
#pragma message("Kitty Graphics Best Graphics")
#pragma message("----------------------------")
#pragma message("           /|, _")
#pragma message("          (',. /")
#pragma message("          |,   \\")
#pragma message("          l.l__,)/")
#pragma message("----------------------------")

ID3D11Debug* debugInterface = nullptr;

namespace KE
{
	Graphics::Graphics(HWND aHWnd, const UINT aWidth, const UINT aHeight)
		:
		myWidth(aWidth), myHeight(aHeight), myAssetLoader(this)
	{
		HRESULT hr;

		// Swapchain setup
		{
			DXGI_SWAP_CHAIN_DESC scd = {};
			scd.BufferDesc.Width = myWidth;
			scd.BufferDesc.Height = myHeight;
			scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			scd.BufferDesc.RefreshRate.Numerator = 0u;
			scd.BufferDesc.RefreshRate.Denominator = 0u;
			scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			scd.SampleDesc.Count = 1u; // Anti-aliasing
			scd.SampleDesc.Quality = 0u; // Anti-aliasing
			scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			scd.BufferCount = 1u; // 1 back buffer and 1 front buffer //huh?
			scd.OutputWindow = aHWnd;
			scd.Windowed = true;
			scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			scd.Flags = 0u;

			UINT swapCreateFlags = D3D11_CREATE_DEVICE_DISABLE_GPU_TIMEOUT;
#ifdef KITTYENGINE_DEBUG
			swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif


			HRESULT swapResult = D3D11CreateDeviceAndSwapChain(
				nullptr,
				D3D_DRIVER_TYPE_HARDWARE,
				nullptr,
				swapCreateFlags,
				nullptr,
				0,
				D3D11_SDK_VERSION,
				&scd,
				&mySwap,
				&myDevice,
				nullptr,
				&myContext
			);

			//see error reason from did
			if (FAILED(swapResult))
			{
				throw std::runtime_error("Failed to create device and swap chain.");
			}
		}

		// Disable DirectX's ALT-ENTER fullscreen, so we can handle it ourselves
		{
			Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
			myDevice->QueryInterface(__uuidof(IDXGIDevice), &dxgiDevice);

			Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
			dxgiDevice->GetAdapter(&dxgiAdapter);

			Microsoft::WRL::ComPtr<IDXGIFactory> dxgiFactory;
			dxgiAdapter->GetParent(__uuidof(IDXGIFactory), &dxgiFactory);

			dxgiFactory->MakeWindowAssociation(aHWnd, DXGI_MWA_NO_ALT_ENTER);
		}

		//myDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugInterface));

		////setup object reporting
		//debugInterface->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

		// Gain access to texture subresource in swap chains (back buffer)
		WRL::ComPtr<ID3D11Resource> pBackBuffer;
		mySwap->GetBuffer(0u, __uuidof(ID3D11Resource), &pBackBuffer);
		myDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &myRenderTarget);

		CreateDepthStencilStates();
		SetDepthStencilState(eDepthStencilStates::Write);

		// Create depth stencil texture and view
		{
			WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
			D3D11_TEXTURE2D_DESC descDepth = {};
			descDepth.Width = myWidth;
			descDepth.Height = myHeight;
			descDepth.MipLevels = 1u;
			descDepth.ArraySize = 1u;
			descDepth.Format = DXGI_FORMAT_D32_FLOAT;
			descDepth.SampleDesc.Count = 1u;
			descDepth.SampleDesc.Quality = 0u;
			descDepth.Usage = D3D11_USAGE_DEFAULT;
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			myDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil);

			// Create view of depth stencil texture
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
			descDSV.Format = DXGI_FORMAT_D32_FLOAT;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice = 0u;
			myDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &myDepthStencil);

			// Bind depth stencil view to OM
			myContext->OMSetRenderTargets(1u, myRenderTarget.GetAddressOf(), myDepthStencil.Get());
		}


		// Configure viewport
		{
			D3D11_VIEWPORT vp = {};
			vp.Width = static_cast<FLOAT>(myWidth);
			vp.Height = static_cast<FLOAT>(myHeight);
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0.0f;
			vp.TopLeftY = 0.0f;
			myContext->RSSetViewports(1u, &vp);
		}

		CreateBlendStates();
		SetBlendState(eBlendStates::Disabled);

		// Default sampler state
		{
			D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samplerDesc.AddressU = samplerDesc.AddressW = samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
			myDevice->CreateSamplerState(&samplerDesc, &mySampler);
		}

		// Fullscreen effect sampler state
		{
			D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samplerDesc.AddressU = samplerDesc.AddressW = samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
			myDevice->CreateSamplerState(&samplerDesc, &myFullscreenEffectSampler);
		}

		// Shadow sampler state
		{
			D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = samplerDesc.AddressW = samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
			samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
			samplerDesc.MipLODBias = 0.0f;
			samplerDesc.BorderColor[0] = 0.0f;
			samplerDesc.BorderColor[1] = 0.0f;
			samplerDesc.BorderColor[2] = 0.0f;
			samplerDesc.BorderColor[3] = 0.0f;
			samplerDesc.MinLOD = 0; 
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			myDevice->CreateSamplerState(&samplerDesc, &myShadowSampler);
		}

		// Shadow sampler state
		{
			D3D11_SAMPLER_DESC samplerCompDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
			samplerCompDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
			samplerCompDesc.AddressU = samplerCompDesc.AddressW = samplerCompDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
			samplerCompDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
			samplerCompDesc.MipLODBias = 0.0f;
			samplerCompDesc.BorderColor[0] = 0.0f;
			samplerCompDesc.BorderColor[1] = 0.0f;
			samplerCompDesc.BorderColor[2] = 0.0f;
			samplerCompDesc.BorderColor[3] = 0.0f;
			samplerCompDesc.MinLOD = 0;
			samplerCompDesc.MaxLOD = D3D11_FLOAT32_MAX;
			myDevice->CreateSamplerState(&samplerCompDesc, &myShadowCompSampler);
		}

		// SSAO Sampler State
		{
			D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samplerDesc.AddressU = samplerDesc.AddressW = samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
			myDevice->CreateSamplerState(&samplerDesc, &mySSAOSampler);
		}

		// Wrapping Sampler
		{
			D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
			samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
			samplerDesc.AddressU = samplerDesc.AddressW = samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
			myDevice->CreateSamplerState(&samplerDesc, &myWrappingSampler);
		}

		//Bilinear Sampler
		{
			D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = samplerDesc.AddressW = samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			myDevice->CreateSamplerState(&samplerDesc, &myBilinearSampler);
		}

		//Point Sampler
		{
			D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			samplerDesc.AddressU = samplerDesc.AddressW = samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.MaxAnisotropy = 1;
			samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			myDevice->CreateSamplerState(&samplerDesc, &myPointSampler);
		}

		// Bind samplers!
		myContext->PSSetSamplers(0, 1, mySampler.GetAddressOf());
		// Slot 1 is cursed and changed somewhere externally
		myContext->PSSetSamplers(2, 1, myFullscreenEffectSampler.GetAddressOf());

		myContext->PSSetSamplers(3, 1, myShadowSampler.GetAddressOf());

		myContext->PSSetSamplers(4, 1, mySSAOSampler.GetAddressOf());

		myContext->PSSetSamplers(5, 1, myWrappingSampler.GetAddressOf());

		myContext->PSSetSamplers(6, 1, myShadowCompSampler.GetAddressOf());

		myContext->PSSetSamplers(7, 1, myBilinearSampler.GetAddressOf());

		myContext->PSSetSamplers(8, 1, myPointSampler.GetAddressOf());

		//init factories/loaders/idkwhattonamethings :3


		myModelLoader.Init(myDevice.Get());
		myTextureLoader.Init(myDevice.Get(), myContext.Get());
		myShaderLoader.Init(myDevice.Get());
		myVFXManager.Init(this);

		// Create Render Targets, TODO -> should probably use a member variable or a #define instead of a random magic number
		for (int i = 0; i < 12; i++)
		{
			myRenderTargets.emplace_back();
			myRenderTargets.back().Init(myDevice.Get(), myContext.Get(), myWidth, myHeight);
		}

		myRenderTargets[10].SetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);
		myRenderTargets[11].SetFormat(DXGI_FORMAT_R16G16B16A16_FLOAT);

		myDistanceFog.Init(myDevice.Get(), myContext.Get(), 
			myWidth, myHeight,
			myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "DistanceFog_PS.cso"), 
			myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "FullscreenAsset_VS.cso"));

		hr = myDeferredLightManager.Init(myDevice.Get(), &myShaderLoader, { (int)myWidth, (int)myHeight });
		if (FAILED(hr))
		{
			assert(hr && "Failed to init DeferredLightManager");
		}

		myModelData.reserve(16384); // this way of doing stuff isnt very good, but it works for now
		mySkeletalModelData.reserve(1024);



		// <+--+> CREATE GBUFFER <+--+>
		myGBuffer = GBuffer::Create({ static_cast<int32_t>(myWidth), static_cast<int32_t>(myHeight) }, myDevice.Get());
		mySecondaryGBuffer = GBuffer::Create({ static_cast<int32_t>(myWidth), static_cast<int32_t>(myHeight) }, myDevice.Get());


		// <+--+> COMMON BUFFER SETUP <+--+>
		{
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));

			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = sizeof(CommonBuffer);
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			hr = myCommonbuffer.Init(myDevice, &bd);
			if (FAILED(hr))
			{
				return;
			}

			CommonBuffer data = {};
			myCommonbuffer.MapBuffer(&data, sizeof(CommonBuffer), myContext.Get());
		}

		//___ GRID LINES ___ //
		{
			LineRenderData lineData;

			lineData.myPS = myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "DebugLine_PS.cso");
			lineData.myVS = myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "DebugLine_VS.cso");

			if (!myDebugLines.Initialize(lineData, myDevice.Get()))
			{
				return;
			}

			int gridHalfSize = 50;
			Vector4f gridcolour = { 0.3525f, 1.3525f, 0.3525f, 1.0f };
			Vector4f xAxis = { 1.0f, 0.0f, 0.0f, 1.0f };
			Vector4f zAxis = { 0.0f, 0.0f, 1.0f, 1.0f };
			Vector4f tenthColour = { 1.0f, 1.0f, 0.15f, 1.0f };
			Vector4f colour = gridcolour;

			for (int i = -gridHalfSize; i <= gridHalfSize; i++)
			{
				if (i % 10 == 0)
				{
					colour = tenthColour;
					if (i == 0)
					{
						colour = zAxis;
						//colour = xAxis;
					}
				}

				///____ X ____
				lineData.myIndices.push_back((int)lineData.myVertices.size());
				lineData.myVertices.emplace_back();
				lineData.myVertices.back().position = { (float)i, 0, (float)-gridHalfSize, 1.0f };
				lineData.myVertices.back().colour = colour;

				lineData.myIndices.push_back((int)lineData.myVertices.size());
				lineData.myVertices.emplace_back();
				lineData.myVertices.back().position = { (float)i, 0, (float)gridHalfSize, 1.0f };
				lineData.myVertices.back().colour = colour;


				if (i == 0)
				{
					colour = xAxis;
					//colour = zAxis;
				}


				/// ____ Z _____
				lineData.myIndices.push_back((int)lineData.myVertices.size());
				lineData.myVertices.emplace_back();
				lineData.myVertices.back().position = { (float)-gridHalfSize, 0, (float)i, 1.0f };
				lineData.myVertices.back().colour = colour;

				lineData.myIndices.push_back((int)lineData.myVertices.size());
				lineData.myVertices.emplace_back();
				lineData.myVertices.back().position = { (float)gridHalfSize, 0, (float)i, 1.0f };
				lineData.myVertices.back().colour = colour;

				if (i % 10 == 0)
				{
					colour = gridcolour;
				}
			}

			if (!myLineGrid.Initialize(lineData, myDevice.Get(), true))
			{
				return;
			}
		}

		// <+--+> FULLSCREEN LINES <+--+>
		{
			LineRenderData lineData;

			lineData.myVS = myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "FullscreenDebugLine_VS.cso");
			lineData.myPS = myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "DebugLine_PS.cso");

			if (!myFullscreenLines.Initialize(lineData, myDevice.Get()))
			{
				return;
			}
		}

		(Vector2i*&)KE_GLOBAL::resolution = &myRenderResolution;

		CreateRasterizerStates();
	}

	Graphics::~Graphics()
	{
		myTextureLoader.Clear();
		myModelLoader.Clear();
		myShaderLoader.Clear();

		//clear all myRenderTargets
		for (auto& renderTarget : myRenderTargets)
		{
			renderTarget.myShaderResourceView = nullptr;
			renderTarget.myRenderTargetView = nullptr;
			renderTarget.myTexture = nullptr;

			renderTarget.myDepthStencilView = nullptr;
			renderTarget.myDepthStencilTexture = nullptr;
		}

		for (auto& blendState : myBlendStates)
		{
			blendState = nullptr;
		}

		for (auto& depthStencilState : myDepthStencilStates)
		{
			depthStencilState = nullptr;
		}

		for (auto& rasterizerState : myRasterizerStates)
		{
			rasterizerState = nullptr;
		}

		myGBuffer.myDepthStencilTexture = nullptr;
		myGBuffer.myDepthStencilShaderResourceView = nullptr;
		myGBuffer.myDepthStencilView = nullptr;
		for (auto& rtv : myGBuffer.myRenderTargetViews)
		{
			rtv = nullptr;
		}
		for (auto& srv : myGBuffer.myShaderResourceViews)
		{
			srv = nullptr;
		}
		for (auto& tex : myGBuffer.myTextures)
		{
			tex = nullptr;
		}

		mySecondaryGBuffer.myDepthStencilTexture = nullptr;
		mySecondaryGBuffer.myDepthStencilShaderResourceView = nullptr;
		mySecondaryGBuffer.myDepthStencilView = nullptr;
		for (auto& rtv : mySecondaryGBuffer.myRenderTargetViews)
		{
			rtv = nullptr;
		}
		for (auto& srv : mySecondaryGBuffer.myShaderResourceViews)
		{
			srv = nullptr;
		}
		for (auto& tex : mySecondaryGBuffer.myTextures)
		{
			tex = nullptr;
		}

		myContext->Flush();


	}

	void Graphics::Init()
	{
		KE_GLOBAL::blackboard.Register("graphics", this);

		mySpriteManager.Init(this);
		myDebugRenderer.Init(*this);
		myGUIHandler.Init(this);

		// needs more work to be used properly
		//myAssetLoader.Init();

		// <+--+> FullScreen Asset <+--+>
		{
			myFullscreenAsset.Init(this);
			myFullscreenAsset.myVertexShader = myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "OutlineRenderer_VS.cso");
			myFullscreenAsset.myPixelShader = myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "OutlineRenderer_PS.cso");
		}

		// <+--+> Post Process <+--+>
		{
			myPostProcess.Init(
				myDevice.Get(),
				this,
				myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "UpDownSample_VS.cso"),
				myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "DownSample_PS.cso"),
				myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "UpSample_PS.cso"),
				myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "Gaussian_PS.cso")
			);


			myPostProcess.SetPSShader(myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "PostProcessing_PS.cso"));
			myPostProcess.SetVSShader(myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "PostProcessing_VS.cso"));

			myPostProcess.ConfigureDownSampleRTs(this, myWidth, myHeight);
		}

		// SSAO
		{
			HRESULT hr = mySSAO.Init(this);
			if (FAILED(hr))
			{
				KE_ERROR("SSAO FAILED TO INIT");
				return;
			}
		}

		//

		myDecalManager.Init(this);

		myDefaultRenderer.Init(this);
		mySkeletalRenderer.Init(this);
		myInstancedRenderer.Init(this);
		myDecalRenderer.Init(this);
		myShellTexturedRenderer.Init(this);

		for (int i = 0; i < static_cast<int>(eRenderLayers::Count); i++)
		{
			myRenderLayers[i].Init(this);

			myRenderLayers[i].AssignRenderers(
				&myDefaultRenderer, &mySkeletalRenderer, &myInstancedRenderer, &myDecalRenderer
			);

			myRenderLayers[i].SetDepthStencilState(KE::eDepthStencilStates::Write);
			myRenderLayers[i].SetBlendState(KE::eBlendStates::Disabled);
			myRenderLayers[i].SetRasterizerState(eRasterizerStates::BackfaceCulling);
		}

		myRenderLayers[static_cast<int>(eRenderLayers::Back)].SetDepthStencilState(KE::eDepthStencilStates::ReadOnlyLessEqual);
		myRenderLayers[static_cast<int>(eRenderLayers::Back)].SetRasterizerState(eRasterizerStates::FrontfaceCulling);

		myRenderLayers[static_cast<int>(eRenderLayers::Back)].flags = 0
																	| RenderLayerFlags_Active
																	//| RenderLayerFlags_ReceiveShadows
																	//| RenderLayerFlags_CastShadows
																	//| RenderLayerFlags_Lit
																	| RenderLayerFlags_DrawModels
																	//| RenderLayerFlags_DrawSkeletal
																	| RenderLayerFlags_DrawDecals
																	//| RenderLayerFlags_DrawSprites
																	//| RenderLayerFlags_DrawVFX
																	//| RenderLayerFlags_SSAO
																	;							

		myRenderLayers[static_cast<int>(eRenderLayers::Main)].flags = 0
																	| RenderLayerFlags_Active
																	| RenderLayerFlags_ReceiveShadows
																	| RenderLayerFlags_CastShadows
																	| RenderLayerFlags_Lit
																	| RenderLayerFlags_DrawModels
																	| RenderLayerFlags_DrawSkeletal
																	| RenderLayerFlags_DrawDecals
																	| RenderLayerFlags_DrawSprites
																	| RenderLayerFlags_DrawVFX
																	//| RenderLayerFlags_SSAO
																	;

		myRenderLayers[static_cast<int>(eRenderLayers::Front)].flags = 0
																	| RenderLayerFlags_Active
																	//| RenderLayerFlags_ReceiveShadows
																	| RenderLayerFlags_CastShadows 
																	| RenderLayerFlags_Lit
																	| RenderLayerFlags_DrawModels
																	| RenderLayerFlags_DrawSkeletal
																	//| RenderLayerFlags_DrawDecals
																	| RenderLayerFlags_DrawSprites
																	| RenderLayerFlags_DrawVFX
																	//| RenderLayerFlags_SSAO
																	;

		myRenderLayers[static_cast<int>(eRenderLayers::UI)].flags = 0
																	| RenderLayerFlags_Active
																	//| RenderLayerFlags_ReceiveShadows
																	//| RenderLayerFlags_CastShadows
																	//| RenderLayerFlags_Lit
																	//| RenderLayerFlags_DrawModels
																	//| RenderLayerFlags_DrawSkeletal
																	//| RenderLayerFlags_DrawDecals
																	| RenderLayerFlags_DrawSprites
																	//| RenderLayerFlags_DrawVFX
																	//| RenderLayerFlags_SSAO
																	;


		//font = new SpriteFont(FontLoader::GenerateMSDFFont("Data/InternalAssets/Dirga.ttf", 64.0f, 3.0f, 2.0f, 1.0f));

		//
		for (int i = 1; i < 6; i++)
		{

			std::vector<std::string> loadPaths = {
				std::format("Data/Assets/Players/Player{}/sk_player0{}.fbx", i, i),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_idle.fbx", i, i),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_run.fbx", i, i),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_dash.fbx", i, i),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_throw.fbx", i, i),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_death.fbx", i, i),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_taunt.fbx", i, i),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_select.fbx", i, i),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_victoryDance.fbx", i, i),
				std::format("Data/Assets/Players/Player{}/anim_player0{}_podium.fbx", i, i),
			};

			myModelLoader.Preload(loadPaths, PreloadData::Type::AnimatedMesh);
		}

		//myModelLoader.Preload({"Data\\EngineAssets\\SmoothSphere.fbx"}, PreloadData::Type::Mesh);
		//myModelLoader.Preload({"Data/EngineAssets/SmoothSphere.fbx"}, PreloadData::Type::Mesh);
	}


#ifndef KITTYENGINE_SHIP

	void Graphics::SetRenderResolution(int aWidth, int aHeight)
	{
		if (aWidth <= 0 || aHeight <= 0)
		{
			return;
		}


		{
			myWidth = aWidth;
			myHeight = aHeight;

			myRenderResolution = { aWidth, aHeight };
		}

		myRenderTarget->Release();
		mySwap->ResizeBuffers(0, aWidth, aHeight, DXGI_FORMAT_UNKNOWN, 0);
		ID3D11Texture2D* pBackBuffer;
		mySwap->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		pBackBuffer->Release();
		myDevice->CreateRenderTargetView(pBackBuffer, NULL, myRenderTarget.GetAddressOf());

		// Create depth stencil texture
		WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
		D3D11_TEXTURE2D_DESC descDepth = {};
		descDepth.Width = aWidth;
		descDepth.Height = aHeight;
		descDepth.MipLevels = 1u;
		descDepth.ArraySize = 1u;
		descDepth.Format = DXGI_FORMAT_D32_FLOAT;
		descDepth.SampleDesc.Count = 1u;
		descDepth.SampleDesc.Quality = 0u;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		myDevice->CreateTexture2D(&descDepth, nullptr, pDepthStencil.ReleaseAndGetAddressOf());

		// Create view of depth stencil texture
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
		descDSV.Format = DXGI_FORMAT_D32_FLOAT;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0u;
		myDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, myDepthStencil.ReleaseAndGetAddressOf());


		for (auto& renderTarget : myRenderTargets)
		{
			renderTarget.Resize(aWidth, aHeight);
		}
		myRenderTargets[10].Resize(1024, 1024);
		myRenderTargets[11].Resize(1024, 1024);

		for (auto& renderLayer : myRenderLayers)
		{
			renderLayer.Resize({ aWidth, aHeight });
		}

		myRenderTargets[9].Resize(4096, 4096);

		myDistanceFog.Resize(aWidth, aHeight);

		auto windowSize = DirectX::XMINT2(aWidth, aHeight);
		myGBuffer.ResizeViewport(myDevice.Get(), windowSize);
		mySecondaryGBuffer.ResizeViewport(myDevice.Get(), windowSize);
		mySSAO.Resize(this, windowSize);

		//go through all cameras and resize them
		for (int i = 0; i < KE_CAMERA_MAX; i++)
		{
			const KE::ProjectionData& data = myCameraManager.GetCamera(i)->GetProjectionData();
			if (myCameraManager.GetCamera(i)->GetType() == KE::ProjectionType::Orthographic) { continue; }
			myCameraManager.GetCamera(i)->SetPerspective(
				(float)aWidth,
				(float)aHeight,
				data.perspective.fov,
				data.perspective.nearPlane,
				data.perspective.farPlane
			);
		}


		myPostProcess.ConfigureDownSampleRTs(this, aWidth, aHeight);

		myVFXManager.Resize(aWidth, aHeight);

		// Set up the viewport.
		SetViewport(aWidth, aHeight);
	}

#else

	void Graphics::SetRenderResolution(int aWidth, int aHeight)
	{
		if (aWidth <= 0 || aHeight <= 0)
		{
			return;
		}

		std::cout << "Graphics::SetRenderResolution CALLED with: " << aWidth << ", " << aHeight << ", " << (isFullscreen ? "true" : "false") << std::endl;


		int useWidth, useHeight;


		if (!isFullscreen) //if windowed, we actually need to resize our swap chain as the window resizes
		{
			myFullscreenRenderHeight = -1;
			myFullscreenRenderWidth = -1;

			{
				myWidth = aWidth;
				myHeight = aHeight;

				useWidth = aWidth;
				useHeight = aHeight;
			}

			myRenderTarget->Release();
			mySwap->ResizeBuffers(0, aWidth, aHeight, DXGI_FORMAT_UNKNOWN, 0);
			ID3D11Texture2D* pBackBuffer;
			mySwap->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
			pBackBuffer->Release();
			myDevice->CreateRenderTargetView(pBackBuffer, NULL, myRenderTarget.GetAddressOf());

			// Create depth stencil texture
			WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
			D3D11_TEXTURE2D_DESC descDepth = {};
			descDepth.Width = aWidth;
			descDepth.Height = aHeight;
			descDepth.MipLevels = 1u;
			descDepth.ArraySize = 1u;
			descDepth.Format = DXGI_FORMAT_D32_FLOAT;
			descDepth.SampleDesc.Count = 1u;
			descDepth.SampleDesc.Quality = 0u;
			descDepth.Usage = D3D11_USAGE_DEFAULT;
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
			myDevice->CreateTexture2D(&descDepth, nullptr, pDepthStencil.ReleaseAndGetAddressOf());

			// Create view of depth stencil texture
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
			descDSV.Format = DXGI_FORMAT_D32_FLOAT;
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			descDSV.Texture2D.MipSlice = 0u;
			myDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, myDepthStencil.ReleaseAndGetAddressOf());
		}
		else //otherwise, we don't :)
		{
			myFullscreenRenderHeight = aHeight;
			myFullscreenRenderWidth = aWidth;

			useWidth = myFullscreenRenderWidth;
			useHeight = myFullscreenRenderHeight;
		}

		myRenderResolution = { useWidth, useHeight };
		

		for (int i = 0; i < myRenderTargets.size(); i++)
		{
			if (i == 10 || i == 11) { continue; } //don't resize the shell texturing RTs!
			auto& renderTarget = myRenderTargets[i];
			renderTarget.Resize(useWidth, useHeight);
		}

		for (auto& renderLayer : myRenderLayers)
		{
			renderLayer.Resize({ useWidth, useHeight });
		}

		myRenderTargets[9].Resize(4096, 4096);
		myRenderTargets[10].Resize(1024, 1024);
		myRenderTargets[11].Resize(1024, 1024);

		myDistanceFog.Resize(useWidth, useHeight);

		auto windowSize = DirectX::XMINT2(useWidth, useHeight);
		myGBuffer.ResizeViewport(myDevice.Get(), windowSize);
		mySecondaryGBuffer.ResizeViewport(myDevice.Get(), windowSize);
		mySSAO.Resize(this, windowSize);

		//go through all cameras and resize them
		for (int i = 0; i < KE_CAMERA_MAX; i++)
		{
			const KE::ProjectionData& data = myCameraManager.GetCamera(i)->GetProjectionData();
			myCameraManager.GetCamera(i)->SetPerspective(
				(float)useWidth,
				(float)useHeight,
				data.perspective.fov,
				data.perspective.nearPlane,
				data.perspective.farPlane
			);
		}


		myPostProcess.ConfigureDownSampleRTs(this, useWidth, useHeight);

		myVFXManager.Resize(useWidth, useHeight);

		// Set up the viewport.
		SetViewport(useWidth, useHeight);


	}

#endif

	void Graphics::SetViewport(int aWidth, int aHeight, float aTopLeftX, float aTopLeftY)
	{
		D3D11_VIEWPORT vp;
		vp.Width = (float)aWidth;
		vp.Height = (float)aHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = aTopLeftX;
		vp.TopLeftY = aTopLeftY;
		myContext->RSSetViewports(1, &vp);
	}

#ifndef KITTYENGINE_SHIP
	int Graphics::GetRenderWidth() const { return myWidth; }
	int Graphics::GetRenderHeight() const { return myHeight; }
	Vector2i Graphics::GetRenderSize() const { return { (int)myWidth, (int)myHeight }; }
#else

	int Graphics::GetRenderWidth() const
	{
		return myFullscreenRenderWidth > 0 ? myFullscreenRenderWidth : myWidth;
	}

	int Graphics::GetRenderHeight() const
	{
		return myFullscreenRenderHeight > 0 ? myFullscreenRenderHeight : myHeight;
	}

	Vector2i Graphics::GetRenderSize() const
	{
		return { (int)GetRenderWidth(), (int)GetRenderHeight() };
	}

#endif


	void Graphics::BeginFrame()
	{
		myDrawCalls = 0;
		myLightDrawCalls = 0;
		

		// Forward rendering
		{
			constexpr float colour[] = KITTY_CLEAR_COLOUR;
			myContext->ClearRenderTargetView(myRenderTarget.Get(), colour);
			myContext->ClearDepthStencilView(myDepthStencil.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
			myContext->OMSetRenderTargets(1u, myRenderTarget.GetAddressOf(), myDepthStencil.Get());
		}

		for (auto& layer : myRenderLayers)
		{
			layer.NewFrame();
		}


		mySpriteManager.BeginFrame();
	}

	void Graphics::SetCommonBuffer(Camera& aCamera)
	{
		CommonBuffer commonBuffer;
		commonBuffer.worldToClipMatrix = myView * myProjection;
		commonBuffer.clipToWorldMatrix = DirectX::XMMatrixInverse(nullptr, commonBuffer.worldToClipMatrix);

		commonBuffer.viewMatrix = myView;
		commonBuffer.projectionMatrix = myProjection;

		commonBuffer.projMatrixInverse = DirectX::XMMatrixInverse(nullptr, myProjection);
		commonBuffer.viewMatrixInverse = DirectX::XMMatrixInverse(nullptr, myView);

		Matrix4x4f camTransform = DirectX::XMMatrixInverse(nullptr, myView);


		commonBuffer.cameraPosition = {
			camTransform(4, 1),
			camTransform(4, 2),
			camTransform(4, 3),
			camTransform(4, 4)
		};


#ifndef KITTYENGINE_SHIP
		commonBuffer.resolution = { (unsigned int)myWidth, (unsigned int)myHeight };
#else
		commonBuffer.resolution = {
			static_cast<unsigned int>(GetRenderWidth()),
			static_cast<unsigned int>(GetRenderHeight())
		};
#endif

		commonBuffer.timePassed = KE_GLOBAL::totalTime;
		commonBuffer.deltaTime = KE_GLOBAL::deltaTime;

		commonBuffer.camNearFar = { aCamera.GetProjectionData().perspective.nearPlane, aCamera.GetProjectionData().perspective.farPlane };

		myCommonbuffer.MapBuffer(&commonBuffer, sizeof(CommonBuffer), myContext.Get());
		myCommonbuffer.BindForPS(COMMON_BUFFER_PS_SLOT, myContext.Get());
		myCommonbuffer.BindForVS(COMMON_BUFFER_PS_SLOT, myContext.Get());
	}

	void Graphics::EndFrame()
	{
		mySpriteManager.EndFrame();
		myVFXManager.EndFrame();
		myDebugLines.EndFrame();
		myFullscreenLines.EndFrame();

		myDrawCallsLastFrame = myDrawCalls;
		myLightDrawCallsLastFrame = myLightDrawCalls;

		myRenderTargetQueue.clear();


		constexpr UINT syncInterval = 0; // No VSync
		constexpr UINT presentFlags = 0; // Present immediately

		HRESULT hr = mySwap->Present(syncInterval, presentFlags);
		if (FAILED(hr))
		{
			assert(hr == DXGI_ERROR_DEVICE_REMOVED && "Device removed");
		}
	}

	void Graphics::RenderDebug()
	{
		SetRasterizerState(eRasterizerStates::AALines);
		static PixelShader* behindLinePS = myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "DebugLine_Behind_PS.cso");
		SetDepthStencilState(KE::eDepthStencilStates::ReadOnlyGreater);
		myDebugLines.Render(myContext.Get(), nullptr, behindLinePS);
		SetDepthStencilState(KE::eDepthStencilStates::ReadOnlyLess);
		myLineGrid.Render(myContext.Get());
		myDebugLines.Render(myContext.Get());
		myFullscreenLines.Render(myContext.Get());
		SetDepthStencilState(KE::eDepthStencilStates::Write);
		myDrawCalls += 4;
		SetRasterizerState(eRasterizerStates::BackfaceCulling);
	}

	void Graphics::DrawLayer(Camera* camera, eRenderLayers layer, bool debugDraw)
	{
		const int layerIndex = static_cast<int>(layer);
		if (!(myRenderLayers[layerIndex].flags & RenderLayerFlags_Active)) { return; }

#pragma region MainRenderPass
		KE::DebugTimeLogger::BeginLogVar("Layer Render Pass");
		if (mySkybox) { myContext->PSSetShaderResources(15, 1, mySkybox->myShaderResourceView.GetAddressOf()); }

		myRenderLayers[layerIndex].NewFrame();
		//myGBuffer.ClearTextures(myContext.Get(),false);
		myGBuffer.SetAsActiveTarget(myContext.Get(), myGBuffer.GetDepthStencilView());

		myRenderLayers[layerIndex].Render(camera);

		myGBuffer.UnbindTarget(myContext.Get());

		if (myCubemap) { myContext->PSSetShaderResources(15, 1, myCubemap->myShaderResourceView.GetAddressOf()); }

		KE::DebugTimeLogger::EndLogVar("Layer Render Pass");
#pragma endregion

#pragma region ShadowPass
		if (myRenderLayers[layerIndex].flags & RenderLayerFlags_CastShadows)
		{
			//SetRasterizerState(eRasterizerStates::FrontfaceCulling);
			KE::DebugTimeLogger::BeginLogVar("Shadow Pass");
			myDeferredLightManager.PrepareShadowPass(this);

			const auto stateOld = myRenderLayers[layerIndex].mySettings.rasterizerState;
			myRenderLayers[layerIndex].SetRasterizerState(eRasterizerStates::FrontfaceCulling);
			myRenderLayers[layerIndex].Render(&myDeferredLightManager.myDirectionalLightCamera, nullptr, myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "Model_Shadow_PS.cso"));
			myRenderLayers[layerIndex].SetRasterizerState(stateOld);

			SetView(camera->GetViewMatrix());
			SetProjection(camera->GetProjectionMatrix());
			SetViewport(myWidth, myHeight);
			KE::DebugTimeLogger::EndLogVar("Shadow Pass");
		}
#pragma endregion

#pragma region DecalPass
		KE::DebugTimeLogger::BeginLogVar("Decal Pass");
		if (myRenderLayers[layerIndex].flags & RenderLayerFlags_DrawDecals)
		{
			myRenderLayers[layerIndex].RenderDecals(camera, &myGBuffer, &mySecondaryGBuffer, &myDecalManager);
		}
		else
		{
			myGBuffer.SetAllAsResources(myContext.Get(), 0u);
		}
		KE::DebugTimeLogger::EndLogVar("Decal Pass");
#pragma endregion

#pragma region LightingPass
		KE::DebugTimeLogger::BeginLogVar("Lighting Pass");
		myRenderLayers[layerIndex].GetProcessedRender()->MakeActive(false);
		SetRasterizerState(eRasterizerStates::BackfaceCulling);
		if (myActivePass == eDeferredPasses::Camera)
		{
			if (myRenderLayers[layerIndex].flags & RenderLayerFlags_SSAO)
			{
				myGBuffer.SetAllAsResources(myContext.Get(), 0u);
				mySSAO.Render(this);

				myRenderLayers[layerIndex].GetProcessedRender()->MakeActive(false);
				myGBuffer.SetAllAsResources(myContext.Get(), 0u);
			}

			if (myRenderLayers[layerIndex].flags & RenderLayerFlags_Lit)
			{
				myDeferredLightManager.Render(this);
			}
			else
			{
				//copy the albedo gbuffer to the bound render target
				myRenderLayers[layerIndex].GetProcessedRender()->CopyFrom(myGBuffer.GetTexture(static_cast<int>(GBuffer::GBufferTexture::Albedo)));
			}

			if (myRenderLayers[layerIndex].flags & RenderLayerFlags_SSAO)
			{
				mySSAO.Unbind(myContext.Get());
			}
		}
		else
		{
			myFullscreenAsset.Render(
				this, myGBuffer.GetShaderResourceViews()[(int)myActivePass - 1],
				myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "FullscreenAsset_VS.cso"),
				myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "FullscreenAsset_PS.cso")
			); 
		}
		KE::DebugTimeLogger::EndLogVar("Lighting Pass");
#pragma endregion

#pragma region SpritePass
		KE::DebugTimeLogger::BeginLogVar("Sprite Pass");
		if (myRenderLayers[layerIndex].flags & RenderLayerFlags_DrawSprites)
		{
			myGBuffer.ClearAllAsResourcesSlots(myContext.Get(), 0u);
			myRenderLayers[layerIndex].GetProcessedRender()->MakeActive(true, myGBuffer.GetDepthStencilView());
			mySpriteManager.Render(camera, layer);
		}
		KE::DebugTimeLogger::EndLogVar("Sprite Pass");
#pragma endregion

#pragma region DebugDraw
		if (layer == eRenderLayers::Main && debugDraw) //until we have per-layer debug lines, this will do
		{
			KE::DebugTimeLogger::BeginLogVar("Debug Render Pass");

			myRenderLayers[layerIndex].GetProcessedRender()->MakeActive(true, myGBuffer.GetDepthStencilView());
			RenderDebug();

			KE::DebugTimeLogger::EndLogVar("Debug Render Pass");
		}
#pragma endregion

#pragma region VFX
		{
			KE::DebugTimeLogger::BeginLogVar("VFX Pass");
			if (myRenderLayers[layerIndex].flags & RenderLayerFlags_DrawVFX)
			{
				mySecondaryGBuffer.SetAllAsResources(myContext.Get(), 4u);
				

				ID3D11RenderTargetView* rtvs[2] = {
					myRenderLayers[layerIndex].GetProcessedRender()->GetRenderTargetView(),
					myRenderTargets[4].GetRenderTargetView()
				};
				myContext->OMSetRenderTargets(2, rtvs, myGBuffer.GetDepthStencilView());
				myVFXManager.Render(layer, myGBuffer.GetDepthStencilView());
				mySecondaryGBuffer.ClearAllAsResourcesSlots(myContext.Get(), 4u);

			}

			KE::DebugTimeLogger::EndLogVar("VFX Pass");
		}
#pragma endregion

		myGBuffer.ClearAllAsResourcesSlots(myContext.Get(), 0u);
	}

	void Graphics::CompositeLayers(KE::RenderTarget& workingRenderTarget, eRenderLayers aHighestLayer)
	{
		KE::DebugTimeLogger::BeginLogVar("Layer Composition Pass");
		workingRenderTarget.MakeActive(false);
		for (int i = 0; i < static_cast<int>(eRenderLayers::Count); i++)
		{
			if (i == (int)aHighestLayer) { break; }

			auto& layer = myRenderLayers[i];
			if (!(layer.flags & RenderLayerFlags_Active)) { continue; }
			myFullscreenAsset.Render(
				this,
				layer.GetProcessedRender()->GetShaderResourceView(), 
				myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "FullscreenAsset_VS.cso"),
				myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "RenderLayerCompositor_PS.cso")
			);
		}
		KE::DebugTimeLogger::EndLogVar("Layer Composition Pass");
	}

	void Graphics::PerformPostProcess(KE::RenderTarget& workingRenderTarget)
	{
		//we post process the composited image, currently no reason to do it per
		//layer


		myPostProcess.PreProcessBloom(this, &workingRenderTarget);
		myVFXManager.GetPostProcessing().MockPreProcessBloom(&myRenderTargets[4]);
		if (workingRenderTarget.ShouldRenderPostProcessing() && myActivePass == eDeferredPasses::Camera)
		{
			KE::DebugTimeLogger::BeginLogVar("PostProcess Pass");
			myPostProcess.Render(this, &workingRenderTarget);
			myVFXManager.GetPostProcessing().Render(this, &workingRenderTarget);

			workingRenderTarget.MakeActive(false);
			KE::DebugTimeLogger::EndLogVar("PostProcess Pass");
		}
	}

	bool Graphics::DetermineBackbufferRender()
	{
#ifdef KITTYENGINE_DEBUG
		if (myRenderTargetQueue.size() == 0)
		{
			return false;
		}
		if (myRenderTargetQueue[0].first == -1)
		{
			myRenderTargetQueue[0].first = 0;
			return true;
		}
#else
		if (myRenderTargetQueue.size() == 0)
		{
			AppendRenderTargetQueue(0, 0);
			myRenderTargets[0].SetShouldRenderDebug(false);
			myRenderTargets[0].SetShouldRenderPostProcessing(true);
			return true;
		}
		if (myRenderTargetQueue[0].first == -1)
		{
			myRenderTargetQueue[0].first = 0;
			return true;
		}
#endif

		return false;
	}

#ifndef KITTYENGINE_SHIP

	void Graphics::PerformBackBufferRender()
	{
		myContext->OMSetRenderTargets(1u, myRenderTarget.GetAddressOf(), nullptr);

		KE::FullscreenAsset asset;
		asset.Init(this);
		asset.myVertexShader = myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "FullscreenAsset_VS.cso");
		asset.myPixelShader = myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "FullscreenAsset_PS.cso");

		asset.Render(this, myRenderTargets[0].GetShaderResourceView());
	}

#else

	void Graphics::PerformBackBufferRender()
	{
		myContext->OMSetRenderTargets(1u, myRenderTarget.GetAddressOf(), nullptr);

		SetViewport(static_cast<int>(myWidth), static_cast<int>(myHeight));

		myFullscreenAsset.Render(
			this,
			myRenderTargets[0].GetShaderResourceView(), 
			myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "FullscreenAsset_VS.cso"),
			myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "FullscreenAsset_PS.cso")
		);
	}

#endif

	void Graphics::Render()
	{
		constexpr float vfxClearColour[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		constexpr float mainClearColour[] = { 0.0f, 0.0f, 0.0f, 1.0f };

		myGUIHandler.UpdateGUI();
		myGUIHandler.RenderGUI(this);

		myGBuffer.ClearAllAsResourcesSlots(myContext.Get(), 0u);

		mySecondaryGBuffer.ClearTextures(myContext.Get());
		myGBuffer.ClearTextures(myContext.Get());
		myRenderTargets[4].Clear((float*)vfxClearColour, true); //this is the render target used for vfx blooming

		const bool renderToBack = DetermineBackbufferRender();

		//this shouldn't live here..
		myVFXManager.Update(KE_GLOBAL::deltaTime);
		//

		SetViewport(myWidth,myHeight);

		for (size_t RTVIndex = 0; RTVIndex < myRenderTargetQueue.size(); RTVIndex++)
		{
			KE::RenderTarget& workingRenderTarget = myRenderTargets[myRenderTargetQueue[RTVIndex].first];
			workingRenderTarget.Clear(mainClearColour);

			Camera* camera = myCameraManager.GetCamera(myRenderTargetQueue[RTVIndex].second);

			myGBuffer.ClearTextures(myContext.Get());
			myGBuffer.SetAsActiveTarget(myContext.Get(), myGBuffer.GetDepthStencilView());

			SetView(camera->GetViewMatrix());
			SetProjection(camera->GetProjectionMatrix());
			SetCommonBuffer(*camera);

			if (isFullscreenRenderQueued)
			{
				workingRenderTarget.MakeActive(false);
				myFullscreenAsset.Render(
					this,
					myFullscreenAsset.myTexture->myShaderResourceView.Get(),
					myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "FullscreenAsset_VS.cso"),
					myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "SplashScreen_PS.cso")
				);
				isFullscreenRenderQueued = false;
				continue;
			}

			//main geometry render

			if (mySkybox) { myContext->PSSetShaderResources(15, 1, mySkybox->myShaderResourceView.GetAddressOf()); }

			myRenderLayers[(int)eRenderLayers::Main].Render(camera);

			SetRasterizerState(eRasterizerStates::FrontfaceCulling);
			SetDepthStencilState(KE::eDepthStencilStates::ReadOnlyLessEqual);
			myRenderLayers[(int)eRenderLayers::Back].Render(camera);
			SetDepthStencilState(KE::eDepthStencilStates::Write);


#define DEFERRED_GRASS
#ifdef DEFERRED_GRASS
			SetRasterizerState(eRasterizerStates::BackfaceCulling);

			auto& shells = myShellTexturedRenderer.GetShellModels();
			if (shells.Size() > 0)
			{
				for (const auto& shell : shells)
				{
					if (shell.effectsRT)
					{
						shell.effectsRT->SetAsShaderResource(4);
					}
					if (shell.displacementTexture)
					{
						myContext->PSSetShaderResources(
							5, 
							1,
							shell.displacementTexture->myShaderResourceView.GetAddressOf()
						);
					}


					ShellTexturingRenderInput shellIn{
						camera->GetViewMatrix(),
						camera->GetProjectionMatrix(),
						shell.attributes,
						nullptr,
						myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "DeferredShellTexturing_PS.cso")
					};

					myShellTexturedRenderer.Render(shellIn, shell.modelData);
				}
			}
			SetRasterizerState(eRasterizerStates::FrontfaceCulling);
#endif

			//shadow rendering

			myDeferredLightManager.PrepareShadowPass(this);
			myRenderLayers[(int)eRenderLayers::Main].Render(&myDeferredLightManager.myDirectionalLightCamera, nullptr, myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "Model_Shadow_PS.cso"));
			SetRasterizerState(eRasterizerStates::BackfaceCulling);


			//front layer rendering

			myGBuffer.SetAsActiveTarget(myContext.Get(), workingRenderTarget.GetDepthStencilView());
			myRenderLayers[(int)eRenderLayers::Front].Render(camera);


			//decal rendering

			myRenderLayers[(int)eRenderLayers::Main].RenderDecals(camera, &myGBuffer, &mySecondaryGBuffer, &myDecalManager);

			//SSAO START
			if (myRenderLayers[(int)eRenderLayers::Main].flags & RenderLayerFlags_SSAO)
			{
				KE::DebugTimeLogger::BeginLogVar("SSAO");
				workingRenderTarget.MakeActive(false);
				mySSAO.Render(this);
				KE::DebugTimeLogger::EndLogVar("SSAO");

			}

			//lighting
			workingRenderTarget.MakeActive(false);
			myGBuffer.SetAllAsResources(myContext.Get(), 0u);
			if (myCubemap) { myContext->PSSetShaderResources(15, 1, myCubemap->myShaderResourceView.GetAddressOf()); }
			myDeferredLightManager.Render(this);
			SetView(camera->GetViewMatrix());
			SetProjection(camera->GetProjectionMatrix());
			SetCommonBuffer(*camera);

			//SSAO END
			if (myRenderLayers[(int)eRenderLayers::Main].flags & RenderLayerFlags_SSAO)
			{
				mySSAO.Unbind(myContext.Get());
			}


#ifndef DEFERRED_GRASS
			auto& shells = myShellTexturedRenderer.GetShellModels();
			if (shells.Size() > 0)
			{
				constexpr ID3D11ShaderResourceView* const nullSRV[1] = { NULL };
				myContext->PSSetShaderResources(6, 1, nullSRV);

				workingRenderTarget.MakeActive(true, myGBuffer.GetDepthStencilView());
				for (const auto& shell : shells)
				{
					if (shell.effectsRT)
					{
						shell.effectsRT->SetAsShaderResource(4);
					}

					ShellTexturingRenderInput shellIn {
						camera->GetViewMatrix(),
						camera->GetProjectionMatrix(),
						shell.attributes,
						nullptr,
						nullptr
					};
												
					myShellTexturedRenderer.Render(shellIn, shell.modelData);
				}
			}
#endif

			if (myActivePass == eDeferredPasses::Camera)
			{

				//water rendering :O

				if (!myWaterModels.empty())
				{
					myGBuffer.ClearAllAsResourcesSlots(myContext.Get(), 0u);
					myRenderTargets[5].CopyFrom(&workingRenderTarget);
					myGBuffer.SetAsResourceOnSlot(myContext.Get(), GBuffer::GBufferTexture::WorldPosition, 6u);
					myRenderTargets[5].SetAsShaderResource(5);
					if (mySkybox) { myContext->PSSetShaderResources(15, 1, mySkybox->myShaderResourceView.GetAddressOf()); }
					workingRenderTarget.MakeActive(true, myGBuffer.GetDepthStencilView());
				}

				for (auto& water : myWaterModels)
				{
					BasicRenderInput in;
					in.viewMatrix = camera->GetViewMatrix(); 
					in.projectionMatrix = camera->GetProjectionMatrix(); 

					myDefaultRenderer.RenderModel(in, *water);
				}


				//vfx rendering
				myGBuffer.ClearAllAsResourcesSlots(myContext.Get(), 0u);
				SetBlendState(eBlendStates::VFXBlend);


				ID3D11RenderTargetView* rtvs[2] = {
					workingRenderTarget.GetRenderTargetView(),
					myRenderTargets[4].GetRenderTargetView()
				};
				myContext->OMSetRenderTargets(2, rtvs, myGBuffer.GetDepthStencilView());
				myVFXManager.Render(eRenderLayers::Main, myGBuffer.GetDepthStencilView());
				myVFXManager.Render(eRenderLayers::Front, myGBuffer.GetDepthStencilView());
				SetBlendState(eBlendStates::AlphaBlend);


				//

				//post processing
				if (workingRenderTarget.ShouldRenderPostProcessing())
				{
					PerformPostProcess(workingRenderTarget);
				}


				// Distance Fog
				{
					SetBlendState(eBlendStates::AlphaBlend);

					workingRenderTarget.MakeActive(false);

					ID3D11ShaderResourceView* const nullSRV[1] = { NULL };
					myContext->PSSetShaderResources(1, 1, nullSRV);

					myGBuffer.SetDepthAsResourceOnSlot(myContext.Get(), 1);
					myDistanceFog.BindResources(&workingRenderTarget, myContext.Get());

					myDistanceFog.Render(this, myFullscreenAsset);

					SetBlendState(eBlendStates::Disabled);
				}
			}
			else
			{
				myFullscreenAsset.Render(
					this, myGBuffer.GetShaderResourceViews()[(int)myActivePass - 1],
					myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "FullscreenAsset_VS.cso"),
					myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "FullscreenAsset_PS.cso")
				);
			}

			if (myDrawSSAOPass > 0)
			{
				myFullscreenAsset.Render(
					this, myDrawSSAOPass == 1 ? mySSAO.GetBlurSRV() : mySSAO.GetSSAOSRV(),
					myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "FullscreenAsset_VS.cso"),
					myShaderLoader.GetPixelShader(SHADER_LOAD_PATH "FullscreenAsset_PS.cso")
				);
			}

			SetDepthStencilState(eDepthStencilStates::ReadOnlyLessEqual);

			SetBlendState(eBlendStates::TransparencyBlend);
			mySpriteManager.Render(camera, eRenderLayers::UI);
			SetBlendState(eBlendStates::Disabled);
			SetDepthStencilState(eDepthStencilStates::Write);

			//debug rendering
			if (workingRenderTarget.ShouldRenderDebug())
			{
				workingRenderTarget.MakeActive(true, myGBuffer.GetDepthStencilView());
				RenderDebug();
			}

			//workingRenderTarget.MakeActive(true);
		}

		if (renderToBack)
		{
			PerformBackBufferRender();
		}

		

		//set backbuffer as active
		myContext->OMSetRenderTargets(1u, myRenderTarget.GetAddressOf(), myDepthStencil.Get());
		myGBuffer.SetAllAsResources(myContext.Get(), 0u);
	}

	//void Graphics::Render()
	//{
	//	constexpr float vfxClearColour[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	//	constexpr float mainClearColour[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	//	myGUIHandler.UpdateGUI();
	//	myGUIHandler.RenderGUI(this);

	//	myGBuffer.ClearAllAsResourcesSlots(myContext.Get(), 0u);

	//	mySecondaryGBuffer.ClearTextures(myContext.Get());
	//	myGBuffer.ClearTextures(myContext.Get());
	//	myRenderTargets[4].Clear((float*)vfxClearColour, true); //this is the render target used for vfx blooming

	//	const bool renderToBack = DetermineBackbufferRender();

	//	//this shouldn't live here..
	//	myVFXManager.Update(KE_GLOBAL::deltaTime);
	//	//

	//	for (size_t RTVIndex = 0; RTVIndex < myRenderTargetQueue.size(); RTVIndex++)
	//	{
	//		KE::RenderTarget& workingRenderTarget = myRenderTargets[myRenderTargetQueue[RTVIndex].first];
	//		workingRenderTarget.Clear(mainClearColour);
	//		workingRenderTarget.MakeActive(true);

	//		Camera* camera = myCameraManager.GetCamera(myRenderTargetQueue[RTVIndex].second);

	//		myGBuffer.ClearTextures(myContext.Get());

	//		SetView(camera->GetViewMatrix());
	//		SetProjection(camera->GetProjectionMatrix());
	//		SetCommonBuffer(*camera);

	//		// 
	//		KE::DebugTimeLogger::BeginLogVar("GBuffer Pass");
	//		//for (int i = static_cast<int>(eRenderLayers::Back); i < static_cast<int>(eRenderLayers::Count); i++)
	//		for (int i = static_cast<int>(eRenderLayers::Count) - 1; i >= static_cast<int>(eRenderLayers::Back); i--)
	//		{
	//			DrawLayer(camera, static_cast<eRenderLayers>(i), workingRenderTarget.ShouldRenderDebug());
	//		}
	//		KE::DebugTimeLogger::EndLogVar("GBuffer Pass");


	//		CompositeLayers(workingRenderTarget, eRenderLayers::Count);
	//		if (workingRenderTarget.ShouldRenderPostProcessing())
	//		{
	//			PerformPostProcess(workingRenderTarget);
	//		}


	//	}

	//	if (renderToBack)
	//	{
	//		PerformBackBufferRender();
	//	}

	//	myGBuffer.SetAllAsResources(myContext.Get(), 0u);

	//	//set backbuffer as active
	//	myContext->OMSetRenderTargets(1u, myRenderTarget.GetAddressOf(), myDepthStencil.Get());
	//}

	void Graphics::AssignCubemap(const std::string& aPath)
	{
		Cubemap* cubemap = myTextureLoader.GetCubemapFromPath(aPath);
		if (cubemap == nullptr)
		{
			KE_WARNING("Cubemap missing %s", aPath.c_str());
			return;
		}

		myCubemap = cubemap;
		//myDeferredLightManager.AssignCubemap(cubemap);
	}

	void Graphics::AssignSkybox(const std::string& aPath)
	{
		Cubemap* skybox = myTextureLoader.GetCubemapFromPath(aPath);
		if (skybox == nullptr)
		{
			KE_WARNING("Skybox missing %s", aPath.c_str());
			return;
		}

		mySkybox = skybox;
	}

	ModelData* Graphics::CreateModelData(const std::string& aModelName)
	{
		KE::ModelData& workingData = (KE::ModelData&)myModelData.emplace_back();

		workingData.myMeshList = &myModelLoader.Load(aModelName);
		workingData.myRenderResources.resize(workingData.myMeshList->myMeshes.size());


		for (size_t i = 0; i < workingData.myMeshList->myMeshes.size(); i++)
		{
			workingData.myRenderResources[i].myVertexShader = myShaderLoader.GetDefaultVertexShader();
			workingData.myRenderResources[i].myPixelShader = myShaderLoader.GetDefaultPixelShader();

			std::string texturePath = workingData.myMeshList->myMaterialNames[i];
			KE::Material* material = myTextureLoader.GetMaterialFromPath(texturePath);
			workingData.myRenderResources[i].myMaterial = material;
		}

		return &myModelData.back();
	}

	SkeletalModelData* Graphics::CreateSkeletalModelData(const std::string& aModelName, KE::SkeletalModelData* aModelData)
	{

		KE::SkeletalModelData* workingData;// = &mySkeletalModelData.emplace_back();
		if (aModelData)
		{
			workingData = aModelData;
		}
		else
		{
			workingData = &mySkeletalModelData.emplace_back();
		}


		// TODO Fix nullcheck if bad path
		// TODO Should then give us a default cube and log a warning
		myModelLoader.LoadSkeletalModel(*workingData, aModelName);
		workingData->myRenderResources.resize(workingData->myMeshList->myMeshes.size());

		for (size_t i = 0; i < workingData->myMeshList->myMeshes.size(); i++)
		{
			workingData->myRenderResources[i].myVertexShader = myShaderLoader.GetVertexShader(SHADER_LOAD_PATH "Model_Deferred_Skeletal_VS.cso");
			workingData->myRenderResources[i].myPixelShader = myShaderLoader.GetDefaultPixelShader();

			std::string texturePath = workingData->myMeshList->myMaterialNames[i];
			KE::Material* material = myTextureLoader.GetMaterialFromPath(texturePath);
			workingData->myRenderResources[i].myMaterial = material;
		}

		return &mySkeletalModelData.back();
	}

	void Graphics::ClearModelData()
	{
		myModelData.clear();
		mySkeletalModelData.clear();
		for (auto& layer : myRenderLayers)
		{
			layer.Reset();
		}
	}

	void Graphics::SetProjection(const DirectX::XMMATRIX& aProjectionMatrix)
	{
		myProjection = aProjectionMatrix;
	}

	void Graphics::SetView(const DirectX::XMMATRIX& aViewMatrix)
	{
		myView = aViewMatrix;
	}

	void Graphics::SetBlendState(eBlendStates aBlendState)
	{
		myContext->OMSetBlendState(myBlendStates[(int)aBlendState].Get(), nullptr, 0xffffffff);
	}

	void Graphics::SetDepthStencilState(eDepthStencilStates aDepthStencilState)
	{
		myContext->OMSetDepthStencilState(myDepthStencilStates[(int)aDepthStencilState].Get(), 0);
	}

	void Graphics::SetRasterizerState(eRasterizerStates aRasterizerState)
	{
		myContext->RSSetState(myRasterizerStates[(int)aRasterizerState].Get());
	}

	void Graphics::SetRenderTarget(RenderTarget* aRenderTarget, bool aUseDepth)
	{
		if (!aRenderTarget)
		{
			myContext->OMSetRenderTargets(1u, myRenderTarget.GetAddressOf(), myDepthStencil.Get());
		}
		else
		{
			aRenderTarget->MakeActive(aUseDepth);
		}
	}

	void Graphics::AddLine(LineVertex& aFirstLine, LineVertex& aSecondLine)
	{
		myDebugLines.AddLine(aFirstLine, aSecondLine);
	}

	void Graphics::DrawLineCube(const Vector3f& aPosition, const Vector3f& aSize, const Vector4f& aColour)
	{
		myDebugLines.DrawCube(aPosition, aSize, aColour);
	}

	void Graphics::DrawLineCube(const Vector3f& aPosition, const Vector3f& aSize, const Transform& aTransform, const Vector4f& aColour)
	{
		myDebugLines.DrawCube(aTransform, aSize, aColour);
	}

	void Graphics::DrawLineSphere(const Vector3f& aPosition, const float aRadius, const Vector4f& aColour)
	{
		myDebugLines.DrawSphere(aPosition, aRadius, aColour);
	}

	void Graphics::DrawCone(const Vector3f& aPosition, const Vector3f& aDirection, const float aLength, const float anOuterRadius, const float anInnerRadius, const Vector4f& aColour)
	{
		myDebugLines.DrawCone(aPosition, aDirection, aLength, anOuterRadius, anInnerRadius, aColour);
	}

	void Graphics::AddScreenSpaceLine(const Vector2f& aFrom, const Vector2f& aTo, const Vector4f& aColour)
	{
		LineVertex p1;
		p1.position = { 2.0f * (aFrom.x / myWidth) - 1.0f, 2.0f * ((myHeight - aFrom.y) / myHeight) - 1.0f, 0.0f, 1.0f };
		p1.colour = aColour;

		LineVertex p2;
		p2.position = { 2.0f * (aTo.x / myWidth) - 1.0f, 2.0f * ((myHeight - aTo.y) / myHeight) - 1.0f, 0.0f, 1.0f };
		p2.colour = aColour;

		myFullscreenLines.AddLine(p1, p2);
	}

	void Graphics::RenderFullscreen(Texture* aTexture)
	{
		myFullscreenAsset.AssignTexture(aTexture);

		myPostProcess.AssignTexture(aTexture);

		isFullscreenRenderQueued = true;
	}

	void Graphics::AppendRenderTargetQueue(int aRenderTargetIndex, int aCameraIndex)
	{
		//loop through queue and check if the render target is already in the queue
		for (size_t i = 0; i < myRenderTargetQueue.size(); i++)
		{
			if (myRenderTargetQueue[i].first == aRenderTargetIndex &&
				myRenderTargetQueue[i].second == aCameraIndex)
			{
				return;
			}
		}

		myRenderTargetQueue.push_back({ aRenderTargetIndex, aCameraIndex });
	}

	RenderTarget* Graphics::GetRenderTarget(int aIndex)
	{
		return &myRenderTargets[aIndex];
	}

	void Graphics::DrawIndexed(UINT aIndexCount, UINT aStartIndexLocation, UINT aBaseVertexLocation)
	{
		myContext->DrawIndexed(aIndexCount, aStartIndexLocation, aBaseVertexLocation);
		myDrawCalls++;
	}

	void Graphics::IncrementDeferredPass()
	{
		int current = static_cast<int>(myActivePass);
		current = ++current < static_cast<int>(eDeferredPasses::Count) - 1 ? current : 0;

		std::cout << current << std::endl;

		myActivePass = static_cast<eDeferredPasses>(current);
	}


	void Graphics::BindTexture(const Texture* aTexture, unsigned aStartSlot)
	{
		ID3D11ShaderResourceView* srv = aTexture->myShaderResourceView.Get();
		myContext->PSSetShaderResources(aStartSlot, 1u, &srv);
	}

	void Graphics::BindMaterial(const Material* aMaterial, unsigned aStartSlot)
	{
		ID3D11ShaderResourceView* shaderResourceViews[] = {
			aMaterial->myTextures[0]->myShaderResourceView.Get(),
			aMaterial->myTextures[1]->myShaderResourceView.Get(),
			aMaterial->myTextures[2]->myShaderResourceView.Get(),
			aMaterial->myTextures[3]->myShaderResourceView.Get()
		};

		myContext->PSSetShaderResources(aStartSlot, 4u, shaderResourceViews);
	}

	//

	RenderLayer* Graphics::GetRenderLayer(eRenderLayers aLayer)
	{
		return &myRenderLayers[static_cast<int>(aLayer)];
	}

	void Graphics::ResetRenderLayer(eRenderLayers aLayer)
	{
		myRenderLayers[static_cast<int>(aLayer)].Reset();
	}

	void Graphics::ResetRenderLayers()
	{
		for (auto& layer : myRenderLayers)
		{
			layer.Reset();
		}
	}

	ID3D11ShaderResourceView* Graphics::GetBoundResource(unsigned int aSlot)
	{
		ID3D11ShaderResourceView* srv;
		myContext->PSGetShaderResources(aSlot, 1u, &srv);
		return srv;
	}

	void Graphics::ToggleDrawSSAOAsResult()
	{
		myDrawSSAOPass = ++myDrawSSAOPass < 3 ? myDrawSSAOPass : 0;
	}

	//

#pragma region State Creation
	bool Graphics::CreateBlendStates()
	{
		HRESULT hr = S_OK;
		D3D11_BLEND_DESC blendStateDesc = {};


		////////////////////////////////////////////////////////////////
		// DISABLED BLEND STATE -> DEFAULT
		blendStateDesc.RenderTarget[0].BlendEnable = FALSE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = myDevice->CreateBlendState(&blendStateDesc, myBlendStates[(int)eBlendStates::Disabled].ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		////////////////////////////////////////////////////////////////
		// ALPHA BLEND
		blendStateDesc = {};
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = myDevice->CreateBlendState(&blendStateDesc, myBlendStates[(int)eBlendStates::AlphaBlend].ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		////////////////////////////////////////////////////////////////
		// VFX BLEND whatever you wanna call it
		blendStateDesc = {};
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;

		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = myDevice->CreateBlendState(&blendStateDesc,
			myBlendStates[(int)eBlendStates::VFXBlend].ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		////////////////////////////////////////////////////////////////
		// ADDITIVE BLEND
		blendStateDesc = {};
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = myDevice->CreateBlendState(&blendStateDesc, myBlendStates[(int)eBlendStates::AdditiveBlend].ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		////////////////////////////////////////////////////////////////
		// TRANSPARENCY BLEND
		blendStateDesc = {};
		blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
		blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
		blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		hr = myDevice->CreateBlendState(&blendStateDesc, myBlendStates[(int)eBlendStates::TransparencyBlend].ReleaseAndGetAddressOf());
		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}

	bool Graphics::CreateDepthStencilStates()
	{
		HRESULT hr = S_OK;

		// WRITE
		D3D11_DEPTH_STENCIL_DESC dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

		hr = myDevice->CreateDepthStencilState(&dsDesc, myDepthStencilStates[(int)eDepthStencilStates::Write].ReleaseAndGetAddressOf());

		if (FAILED(hr))
		{
			return false;
		}

		// READ ONLY GREATER
		dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_GREATER;
		dsDesc.StencilEnable = FALSE;

		hr = myDevice->CreateDepthStencilState(&dsDesc, myDepthStencilStates[(int)eDepthStencilStates::ReadOnlyGreater].ReleaseAndGetAddressOf());

		if (FAILED(hr))
		{
			return false;
		}

		// READ ONLY LESS
		dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		dsDesc.StencilEnable = FALSE;

		hr = myDevice->CreateDepthStencilState(
			&dsDesc, myDepthStencilStates[(int)eDepthStencilStates::ReadOnlyLess].ReleaseAndGetAddressOf());

		if (FAILED(hr))
		{
			return false;
		}

		// READ ONLY EMPTY
		dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_NEVER;
		dsDesc.StencilEnable = FALSE;

		hr = myDevice->CreateDepthStencilState(&dsDesc, myDepthStencilStates[(int)eDepthStencilStates::ReadOnlyEmpty].ReleaseAndGetAddressOf());

		if (FAILED(hr))
		{
			return false;
		}

		// ALWAYS
		dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		dsDesc.StencilEnable = FALSE;

		hr = myDevice->CreateDepthStencilState(&dsDesc, myDepthStencilStates[(int)eDepthStencilStates::Always].ReleaseAndGetAddressOf());

		if (FAILED(hr))
		{
			return false;
		}

		// ReadOnlyLessEqual
		dsDesc = {};
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		dsDesc.StencilEnable = FALSE;

		hr = myDevice->CreateDepthStencilState(&dsDesc, myDepthStencilStates[(int)eDepthStencilStates::ReadOnlyLessEqual].ReleaseAndGetAddressOf());

		if (FAILED(hr))
		{
			return false;
		}

		return true;
	}

	bool Graphics::CreateRasterizerStates()
	{
		HRESULT hr = S_OK;

		D3D11_RASTERIZER_DESC rasterizerDesc = {};

		myRasterizerStates[(int)eRasterizerStates::BackfaceCulling] = nullptr;

		rasterizerDesc.AntialiasedLineEnable = false;
		rasterizerDesc.CullMode = D3D11_CULL_FRONT;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.MultisampleEnable = true;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		hr = myDevice->CreateRasterizerState(&rasterizerDesc, &myRasterizerStates[(int)eRasterizerStates::FrontfaceCulling]);
		if (FAILED(hr))
		{
			return false;
		}

		rasterizerDesc.AntialiasedLineEnable = false;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.MultisampleEnable = true;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		hr = myDevice->CreateRasterizerState(&rasterizerDesc, &myRasterizerStates[(int)eRasterizerStates::NoCulling]);
		if (FAILED(hr))
		{
			return false;
		}

		//wireframe

		rasterizerDesc.AntialiasedLineEnable = false;
		rasterizerDesc.CullMode = D3D11_CULL_NONE;
		rasterizerDesc.DepthBias = 0;
		rasterizerDesc.DepthBiasClamp = 0.0f;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.MultisampleEnable = true;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.SlopeScaledDepthBias = 0.0f;
		hr = myDevice->CreateRasterizerState(&rasterizerDesc, &myRasterizerStates[(int)eRasterizerStates::Wireframe]);
		if (FAILED(hr))
		{
			return false;
		}

		// AA Lines
		{
			CD3D11_RASTERIZER_DESC desc;
			desc.FillMode = D3D11_FILL_SOLID;
			desc.CullMode = D3D11_CULL_NONE;
			desc.FrontCounterClockwise = FALSE;
			desc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
			desc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
			desc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			desc.DepthClipEnable = TRUE;
			desc.ScissorEnable = FALSE;
			desc.MultisampleEnable = FALSE;
			desc.AntialiasedLineEnable = TRUE;

			hr = myDevice->CreateRasterizerState(&desc, &myRasterizerStates[(int)eRasterizerStates::AALines]);
			if (FAILED(hr))
			{
				return false;
			}
		}

		return true;
	}

	Graphics::FrustumPlanes Graphics::ExtractFrustumPlanes() const
	{
		FrustumPlanes frustum;

		// Extract the rows of the view-projection matrix
		DirectX::XMFLOAT4X4 VP;
		const DirectX::XMMATRIX viewProjectionMatrix = myView * myProjection;
		DirectX::XMStoreFloat4x4(&VP, viewProjectionMatrix);

		// Extract the frustum planes from the view-projection matrix
		frustum.planes[0] = DirectX::XMFLOAT4(VP._14 + VP._11, VP._24 + VP._21, VP._34 + VP._31, VP._44 + VP._41);
		// Left plane
		frustum.planes[1] = DirectX::XMFLOAT4(VP._14 - VP._11, VP._24 - VP._21, VP._34 - VP._31, VP._44 - VP._41);
		// Right plane
		frustum.planes[2] = DirectX::XMFLOAT4(VP._14 - VP._12, VP._24 - VP._22, VP._34 - VP._32, VP._44 - VP._42);
		// Top plane
		frustum.planes[3] = DirectX::XMFLOAT4(VP._14 + VP._12, VP._24 + VP._22, VP._34 + VP._32, VP._44 + VP._42);
		// Bottom plane
		frustum.planes[4] = DirectX::XMFLOAT4(VP._13, VP._23, VP._33, VP._43); // Near plane
		frustum.planes[5] = DirectX::XMFLOAT4(VP._14 - VP._13, VP._24 - VP._23, VP._34 - VP._33, VP._44 - VP._43);
		// Far plane

		// Normalize the frustum planes
		for (auto& plane : frustum.planes)
		{
			const float length = std::sqrt(plane.x * plane.x +
				plane.y * plane.y +
				plane.z * plane.z);
			plane = DirectX::XMFLOAT4(plane.x / length,
				plane.y / length,
				plane.z / length,
				plane.w / length);
		}

		return frustum;
	}

	bool Graphics::IsModelBoundsInFrustum(const Vector3f& aPosition, const ModelBounds& aModelBounds) const
	{
		const Vector3f min = aPosition + aModelBounds.min;
		const Vector3f max = aPosition + aModelBounds.max;
		const FrustumPlanes frustum = ExtractFrustumPlanes();
		for (int i = 0; i < 6; ++i)
		{
			if (frustum.planes[i].x * min.x + frustum.planes[i].y * min.y + frustum.planes[i].z * min.z + frustum.
				planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * max.x + frustum.planes[i].y * min.y + frustum.planes[i].z * min.z + frustum.
				planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * min.x + frustum.planes[i].y * max.y + frustum.planes[i].z * min.z + frustum.
				planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * max.x + frustum.planes[i].y * max.y + frustum.planes[i].z * min.z + frustum.
				planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * min.x + frustum.planes[i].y * min.y + frustum.planes[i].z * max.z + frustum.
				planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * max.x + frustum.planes[i].y * min.y + frustum.planes[i].z * max.z + frustum.
				planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * min.x + frustum.planes[i].y * max.y + frustum.planes[i].z * max.z + frustum.
				planes[i].w > 0.0f)
				continue;
			if (frustum.planes[i].x * max.x + frustum.planes[i].y * max.y + frustum.planes[i].z * max.z + frustum.
				planes[i].w > 0.0f)
				continue;

			// If the bounding box is completely outside any frustum plane, it is not visible
			return false;
		}

		// If the bounding box is not completely outside any frustum plane, it is visible
		return true;
	}

#pragma endregion
}