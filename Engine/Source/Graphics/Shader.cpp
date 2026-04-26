#include "stdafx.h"
#include "Shader.h"

#include <fstream>

#include <d3d11.h>

namespace KE
{
	PixelShader::PixelShader()
	{
		myPixelShaderSlots[0] = nullptr;
		myPixelShaderSlots[1] = nullptr;
		myShaderType = ShaderType::ePixelShader;
	}

	void PixelShader::Init(const std::string& aFilePath, ID3D11Device* aDevice)
	{
		// Load Pixel Shader
		HRESULT result;

		std::ifstream psFile;
		psFile.open(aFilePath, std::ios::binary);

		if (psFile.fail())
		{
			psFile.close();
			psFile.open("../x64/" + aFilePath, std::ios::binary);
		}

		std::string psData = { std::istreambuf_iterator<char>(psFile), std::istreambuf_iterator<char>() };
		result = aDevice->CreatePixelShader(psData.data(), psData.size(), nullptr, &myPixelShaderSlots[0]);
		psFile.close();
		myShaderData = psData;

		if (FAILED(result))
		{
			myPixelShaderSlots[0] = nullptr;
			return;
		}

		SetName(aFilePath);
	}

	void PixelShader::Clear()
	{
		myPixelShaderSlots[0] = nullptr;
		myPixelShaderSlots[1] = nullptr;
	}

	void PixelShader::AssignRecompiledShader(ID3D11PixelShader* aShader, bool aSwitchSlot)
	{
		myPixelShaderSlots[1] = aShader;
		if (aSwitchSlot)
		{
			myActiveSlot = 1;
		}
	}

	VertexShader::VertexShader()
	{
		myVertexShaderSlots[0] = nullptr;
		myVertexShaderSlots[1] = nullptr;

		myShaderType = ShaderType::eVertexShader;
	}

	void VertexShader::Init(const std::string& aFilePath, ID3D11Device* aDevice)
	{
		HRESULT result;

		std::string vsData;
		std::ifstream vsFile;

		vsFile.open(aFilePath, std::ios::binary);

		if (vsFile.fail())
		{
			vsFile.close();
			vsFile.open("../x64/" + aFilePath, std::ios::binary);
		}

		vsData = { std::istreambuf_iterator<char>(vsFile), std::istreambuf_iterator<char>() };
		result = aDevice->CreateVertexShader(vsData.data(), vsData.size(), nullptr, &myVertexShaderSlots[0]);
		vsFile.close();
		myShaderData = vsData;
		if (FAILED(result))
		{
			myVertexShaderSlots[0] = nullptr;
			return;
		}

		// create Input Layout

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			}
		};

		D3D11_INPUT_ELEMENT_DESC instancedLayout[] =
		{
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},

			{"INSTANCE_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INSTANCE_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INSTANCE_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INSTANCE_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INSTANCE_ATTRIBUTES", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
		};

		D3D11_INPUT_ELEMENT_DESC lineLayout[] =
		{
			{
				"SV_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
		};

		D3D11_INPUT_ELEMENT_DESC spriteLayout[] =
		{
			// Data from the vertex buffer
			{"SV_POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0}, // 3 floats
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}, // 2 floats
			//{ "GARBAGE", 0, DXGI_FORMAT_R32G32_FLOAT,		 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 }, // 2 floats


			// Data from the instance buffer
			{"INSTANCE_TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INSTANCE_TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INSTANCE_TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INSTANCE_TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INSTANCE_COLOUR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INSTANCE_TEX_BOUNDS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INSTANCE_TEX_REGION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1}
		};

		D3D11_INPUT_ELEMENT_DESC skeletalAnimLayout[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BONEINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BONEWEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		D3D11_INPUT_ELEMENT_DESC shellLayout[] =
		{
			{
				"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},
			{
				"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
				D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0
			},

			{"INSTANCE_UV_MIN", 0, DXGI_FORMAT_R32G32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},
			{"INSTANCE_UV_MAX", 0, DXGI_FORMAT_R32G32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1},

		};

		if (aFilePath.find("Line") != std::string::npos)
		{
			result = aDevice->CreateInputLayout(
				lineLayout,
				static_cast<UINT>(std::size(lineLayout)),
				vsData.data(),
				vsData.size(),
				&myInputLayout
			);
		}
		else if (aFilePath.find("Sprite") != std::string::npos)
		{
			result = aDevice->CreateInputLayout(
				spriteLayout,
				static_cast<UINT>(std::size(spriteLayout)),
				vsData.data(),
				vsData.size(),
				&myInputLayout
			);
		}
		else if (aFilePath.find("Skeletal") != std::string::npos)
		{
			result = aDevice->CreateInputLayout(
				skeletalAnimLayout,
				(UINT)std::size(skeletalAnimLayout),
				vsData.data(),
				vsData.size(),
				&myInputLayout
			);
		}
		else if (aFilePath.find("Instanced") != std::string::npos)
		{
			result = aDevice->CreateInputLayout(
				instancedLayout,
				(UINT)std::size(instancedLayout),
				vsData.data(),
				vsData.size(),
				&myInputLayout
			);
		}
		else if (aFilePath.find("Water") != std::string::npos)
		{
			result = aDevice->CreateInputLayout(
				instancedLayout,
				(UINT)std::size(instancedLayout),
				vsData.data(),
				vsData.size(),
				&myInputLayout
			);
		}
		else if (aFilePath.find("Shell") != std::string::npos)
		{
			result = aDevice->CreateInputLayout(
				shellLayout,
				(UINT)std::size(shellLayout),
				vsData.data(),
				vsData.size(),
				&myInputLayout
			);
		}
		else
		{
			result = aDevice->CreateInputLayout(
				layout,
				static_cast<UINT>(std::size(layout)),
				vsData.data(),
				vsData.size(),
				&myInputLayout
			);
		}
		if (FAILED(result))
		{
			myVertexShaderSlots[0] = nullptr;
			return;
		}

		SetName(aFilePath);
	}

	void VertexShader::Clear()
	{
		myVertexShaderSlots[0] = nullptr;
		myVertexShaderSlots[1] = nullptr;
	}

	void VertexShader::AssignRecompiledShader(ID3D11VertexShader* aShader, bool aSwitchSlot)
	{
		myVertexShaderSlots[1] = aShader;
		if (aSwitchSlot)
		{
			myActiveSlot = 1;
		}
	}

	ComputeShader::ComputeShader()
	{
		myComputeShaderSlots[0] = nullptr;
		myComputeShaderSlots[1] = nullptr;
	}

	void ComputeShader::Init(const std::string& aFilePath, ID3D11Device* aDevice)
	{
		// Load Compute Shader
		HRESULT result;

		std::ifstream csFile;
		csFile.open(aFilePath, std::ios::binary);

		if (csFile.fail())
		{
			csFile.close();
			csFile.open("../x64/" + aFilePath, std::ios::binary);
		}

		std::string csData = { std::istreambuf_iterator<char>(csFile), std::istreambuf_iterator<char>() };
		result = aDevice->CreateComputeShader(csData.data(), csData.size(), nullptr, &myComputeShaderSlots[0]);
		csFile.close();
		myShaderData = csData;

		if (FAILED(result))
		{
			myComputeShaderSlots[0] = nullptr;
			return;
		}

		SetName(aFilePath);
	}

	void ComputeShader::Clear()
	{
		myComputeShaderSlots[0] = nullptr;
		myComputeShaderSlots[1] = nullptr;
	}

	void ComputeShader::AssignRecompiledShader(ID3D11ComputeShader* aShader, bool aSwitchSlot)
	{
		myComputeShaderSlots[1] = aShader;
		if (aSwitchSlot)
		{
			myActiveSlot = 1;
		}
	}
}
