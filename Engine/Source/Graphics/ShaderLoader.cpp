#include "stdafx.h"
#include "ShaderLoader.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#include <filesystem>

#include <WinBase.h>

#include "Script/CodeScriptParser.h"
#include "Engine/Source/Script/Script.h"
#include "Engine/Source/Script/ScriptManager.h"
#include "Utility/Timer.h"


#define HLSL_PATH "../Engine/Source/Graphics/Shaders/"
//#define HLSL_PATH "Shaders/"

#define SHADER_RECOMPILATION_SLEEP 5	// milliseconds to wait between a file change and recompilation
// without sleeping recompilation fails, presumably 
// because the async file watcher owns it for that frame?
// 1 ms is enough for my machine, but might not be for others

#define toWstr(str) std::wstring(str.begin(), str.end())

//a better include handler that originates from the parent path
namespace fs = std::filesystem;
class RecompileHandler : public ID3DInclude
{
public:
	STDMETHOD(Open)(
		D3D_INCLUDE_TYPE IncludeType,
		LPCSTR            pFileName,
		LPCVOID           pParentData,
		LPCVOID* ppData,
		UINT* pBytes) override
	{
		std::string shaderPath = HLSL_PATH;
		std::string fileName(pFileName);

		// Check if the file exists in the default directory
		if (fs::exists(shaderPath + fileName))
		{
			// Read the contents of the file
			std::ifstream file(shaderPath + fileName);
			std::string fileContents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

			// Allocate memory for the include data
			*pBytes = static_cast<UINT>(fileContents.size());
			*ppData = new char[*pBytes];
			memcpy((void*)*ppData, fileContents.c_str(), *pBytes);

			file.close();
			return S_OK;
		}

		// If the file was not found in the default directory, try other directories within "Shaders"
		for (const auto& directoryEntry : fs::directory_iterator(shaderPath))
		{
			if (directoryEntry.is_directory())
			{
				std::string additionalDirectory = directoryEntry.path().string() + "/";
				if (fs::exists(additionalDirectory + fileName))
				{
					// Read the contents of the file
					std::ifstream additionalFile(additionalDirectory + fileName);
					std::string fileContents((std::istreambuf_iterator<char>(additionalFile)), std::istreambuf_iterator<char>());

					// Allocate memory for the include data
					*pBytes = static_cast<UINT>(fileContents.size());
					*ppData = new char[*pBytes];
					memcpy((void*)*ppData, fileContents.c_str(), *pBytes);

					additionalFile.close();
					return S_OK;
				}
			}
		}

		// File not found
		return E_FAIL;
	}

	STDMETHOD(Close)(LPCVOID pData) override
	{
		// Free memory allocated for include data
		delete[] pData;
		return S_OK;
	}
};

namespace KE
{
	void ShaderLoader::FindAndLoadShaders()
	{
		for (auto& p : std::filesystem::directory_iterator(std::filesystem::current_path() / "Shaders"))
		{
			std::string filePath = p.path().string();
			std::string fileName = p.path().filename().string();

			//if the file is a pixel shader
			if (fileName.find("PS") != std::string::npos)
			{
				//load the pixel shader
				GetPixelShader(fileName);
			}
			//if the file is a vertex shader
			else if (fileName.find("VS") != std::string::npos)
			{
				//load the vertex shader
				GetVertexShader(fileName);
			}
			else if (fileName.find("CS") != std::string::npos)
			{
				GetComputeShader(fileName);
			}
		}
	}

	void ShaderLoader::ActivateFileWatcher() {}

	void ShaderLoader::Init(ID3D11Device* aDevice)
	{
		myDevice = aDevice;

		//

		GetPixelShader(DEFAULT_PIXEL_SHADER);
		GetVertexShader(DEFAULT_VERTEX_SHADER);

	}

	void ShaderLoader::Clear()
	{
		for (auto& shader : myPixelShaders)
		{
			shader.second.Clear();
		}

		for (auto& shader : myVertexShaders)
		{
			shader.second.Clear();
		}

		for (auto& shader : myComputeShaders)
		{
			shader.second.Clear();
		}
	}

	VertexShader* ShaderLoader::CreateVertexShaderFromParsedScript(const std::string& aShaderName, const std::string& aParsedCode)
	{
		std::ofstream file;
		file.open("_VertexShaderStaging.txt");
		file << "#include \"Data/EngineAssets/LanguageDefinitions/hlslFunctions.hlsli\" \n";
		file << aParsedCode;
		file.close();

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

		// Compile the shader from file
		ID3DBlob* compiledShader = nullptr;
		ID3DBlob* errorBlob = nullptr;

		HRESULT hr = D3DCompileFromFile(
			L"_VertexShaderStaging.txt", // Path to the shader file
			nullptr, // Macro definitions (can be nullptr)
			D3D_COMPILE_STANDARD_FILE_INCLUDE, // Include handler (can be nullptr)
			"main", // Entry point function name
			"vs_5_0", // Pixel shader model
			0, // Compile flags (can be 0)
			0, // Additional compile flags (can be 0)
			&compiledShader, // Compiled shader blob
			&errorBlob // Error blob (used for error messages)
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			return nullptr;
		}

		if (errorBlob)
		{
			errorBlob->Release();
		}

		// Create the pixel shader
		ID3D11VertexShader* vertexShader = nullptr;
		hr = myDevice->CreateVertexShader(
			compiledShader->GetBufferPointer(),
			compiledShader->GetBufferSize(),
			nullptr,
			&vertexShader
		);

		if (FAILED(hr))
		{
			compiledShader->Release();
			return nullptr;
		}

		myVertexShaders[aShaderName] = VertexShader();
		auto& vs = myVertexShaders[aShaderName];

		vs.myVertexShaderSlots[0] = vertexShader;
		vs.myShaderType = Shader::eVertexShader;
		vs.myName = aShaderName;
		vs.myShaderData = "huh";

		hr = myDevice->CreateInputLayout(
			instancedLayout,
			(UINT)std::size(instancedLayout),
			compiledShader->GetBufferPointer(),
			compiledShader->GetBufferSize(),
			&vs.myInputLayout
		);


		// Release the compiled shader blob, as it's no longer needed
		compiledShader->Release();

		return &vs;
	}

	VertexShader* ShaderLoader::CreateVertexShaderFromScript(const std::string& aShaderName, Script* aShaderScript)
	{

		auto* res = new ParsingOutput{ aShaderScript };
		auto& result = *res;
		CodeScriptParser::Parse(aShaderScript, res);

		VertexShader* vs = CreateVertexShaderFromParsedScript(aShaderName, result.code);

		delete res;
		return vs;
	}

	PixelShader* ShaderLoader::CreatePixelShaderFromParsedScript(const std::string& aShaderName, const std::string& aParsedCode, const std::string& aEntryName)
	{
		Timer compileTimer;
		myPixelShaders[aShaderName] = PixelShader();
		myPixelShaders[aShaderName].myPixelShaderSlots[0] = CreatePixelShaderFromCode(
			std::format("{}\n{}\n{}\n{}",
				"#include \"Data/EngineAssets/LanguageDefinitions/hlslFunctions.hlsli\"",
				"#include \"Data/EngineAssets/LanguageDefinitions/hlslMath.hlsli\"",
				"#include \"Data/EngineAssets/LanguageDefinitions/hlslNoise.hlsli\"",
				aParsedCode
			),

			aEntryName
		);
		myPixelShaders[aShaderName].myShaderType = Shader::ePixelShader;
		myPixelShaders[aShaderName].myName = aShaderName;
		const float compileTime = compileTimer.UpdateDeltaTime();		

		//std::cout << "Compile Time: " << compileTime << std::endl;

		return &myPixelShaders[aShaderName];
	}

	PixelShader* ShaderLoader::CreatePixelShaderFromScript(const std::string& aShaderName, Script* aShaderScript, const std::string& aEntryName)
	{
		auto* res = new ParsingOutput{ aShaderScript };
		bool success = CodeScriptParser::Parse(aShaderScript, res);
		auto& result = *res;

		PixelShader* ps = CreatePixelShaderFromParsedScript(aShaderName, result.code, aEntryName);
		delete res;
		return ps;
	}

	PixelShader* ShaderLoader::GetPixelShader(std::string aFilePath)
	{

		if (myPixelShaders.find(aFilePath) == myPixelShaders.end())
		{
			myPixelShaders[aFilePath] = PixelShader();

			if (aFilePath.find("ScriptPS_") != std::string::npos)
			{
				const std::string scriptName = aFilePath.substr(aFilePath.find("ScriptPS_") + sizeof("ScriptPS_")-1);


				auto* scriptMgr = KE_GLOBAL::blackboard.Get<ScriptManager>("scriptManager");
				CreatePixelShaderFromScript(
					aFilePath,
					scriptMgr->GetOrLoadScript(scriptName),
					"main"
				);

			}
			else
			{
				myPixelShaders[aFilePath].Init(aFilePath, myDevice);
			}

		}

		if (myPixelShaders[aFilePath].Failed())
		{
			KE_ERROR("Shader not found: %s", aFilePath.c_str());
			return GetDefaultPixelShader();
		}

		return &myPixelShaders[aFilePath];
	}

	VertexShader* ShaderLoader::GetVertexShader(std::string aFilePath)
	{
		if (myVertexShaders.find(aFilePath) == myVertexShaders.end())
		{
			myVertexShaders[aFilePath] = VertexShader();
			myVertexShaders[aFilePath].Init(aFilePath, myDevice);
		}

		if (myVertexShaders[aFilePath].Failed())
		{
			return GetDefaultVertexShader();
		}

		return &myVertexShaders[aFilePath];
	}

	ComputeShader* ShaderLoader::GetComputeShader(std::string aFilePath)
	{
		if (myComputeShaders.find(aFilePath) == myComputeShaders.end())
		{
			myComputeShaders[aFilePath] = ComputeShader();
			myComputeShaders[aFilePath].Init(aFilePath, myDevice);
		}

		if (myComputeShaders[aFilePath].Failed())
		{
			return nullptr;
		}

		return &myComputeShaders[aFilePath];
	}

	std::vector<PixelShader*> ShaderLoader::GetPixelShadersVector() const
	{
		std::vector<PixelShader*> pixelShaders;
		for (auto& shader : myPixelShaders)
		{
			pixelShaders.push_back((PixelShader*)&shader.second);
		}
		return pixelShaders;
	}

	std::vector<ComputeShader*> ShaderLoader::GetComputeShadersVector() const
	{
		std::vector<ComputeShader*> computeShaders;
		for (auto& shader : myComputeShaders)
		{
			computeShaders.push_back((ComputeShader*)&shader.second);
		}
		return computeShaders;

		return computeShaders;
	}

	ID3D11VertexShader* ShaderLoader::CreateVertexShaderFromFile(const wchar_t* aFilePath)
	//, const char* anEntryPoint)
	{
		// Compile the shader from file
		ID3DBlob* compiledShader = nullptr;
		ID3DBlob* errorBlob = nullptr;

		RecompileHandler handler;

		HRESULT hr = D3DCompileFromFile(
			aFilePath, // Path to the shader file
			nullptr, // Macro definitions (can be nullptr)
			&handler, // Include handler (can be nullptr)
			"main", // Entry point function name
			"vs_5_0", // Pixel shader model
			0, // Compile flags (can be 0)
			0, // Additional compile flags (can be 0)
			&compiledShader, // Compiled shader blob
			&errorBlob // Error blob (used for error messages)
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			return nullptr;
		}

		if (errorBlob)
		{
			errorBlob->Release();
		}

		// Create the pixel shader
		ID3D11VertexShader* vertexShader = nullptr;
		hr = myDevice->CreateVertexShader(
			compiledShader->GetBufferPointer(),
			compiledShader->GetBufferSize(),
			nullptr,
			&vertexShader
		);

		if (FAILED(hr))
		{
			compiledShader->Release();
			return nullptr;
		}

		// Release the compiled shader blob, as it's no longer needed
		compiledShader->Release();

		return vertexShader;
	}

	ID3D11PixelShader* ShaderLoader::CreatePixelShaderFromFile(const wchar_t* aFilePath) //, const char* anEntryPoint)
	{
		// Compile the shader from file
		ID3DBlob* compiledShader = nullptr;
		ID3DBlob* errorBlob = nullptr;

		RecompileHandler handler;

		HRESULT hr = D3DCompileFromFile(
			aFilePath, // Path to the shader file
			nullptr, // Macro definitions (can be nullptr)
			&handler, // Include handler (can be nullptr)
			"main", // Entry point function name
			"ps_5_0", // Pixel shader model
			0, // Compile flags (can be 0)
			0, // Additional compile flags (can be 0)
			&compiledShader, // Compiled shader blob
			&errorBlob // Error blob (used for error messages)
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			return nullptr;
		}

		if (errorBlob)
		{
			errorBlob->Release();
		}

		// Create the pixel shader
		ID3D11PixelShader* pixelShader = nullptr;
		hr = myDevice->CreatePixelShader(
			compiledShader->GetBufferPointer(),
			compiledShader->GetBufferSize(),
			nullptr,
			&pixelShader
		);

		if (FAILED(hr))
		{
			compiledShader->Release();
			return nullptr;
		}

		// Release the compiled shader blob, as it's no longer needed
		compiledShader->Release();

		return pixelShader;
	}

	ID3D11ComputeShader* ShaderLoader::CreateComputeShaderFromFile(const wchar_t* aFilePath)
	{
		// Compile the shader from file
		ID3DBlob* compiledShader = nullptr;
		ID3DBlob* errorBlob = nullptr;
		RecompileHandler handler;
		HRESULT hr = D3DCompileFromFile(
			aFilePath, // Path to the shader file
			nullptr, // Macro definitions (can be nullptr)
			&handler, // Include handler (can be nullptr)
			"main", // Entry point function name
			"ps_5_0", // Pixel shader model
			0, // Compile flags (can be 0)
			0, // Additional compile flags (can be 0)
			&compiledShader, // Compiled shader blob
			&errorBlob // Error blob (used for error messages)
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			return nullptr;
		}

		if (errorBlob)
		{
			errorBlob->Release();
		}

		// Create the compute shader
		ID3D11ComputeShader* computeShader = nullptr;
		hr = myDevice->CreateComputeShader(
			compiledShader->GetBufferPointer(),
			compiledShader->GetBufferSize(),
			nullptr,
			&computeShader
		);

		if (FAILED(hr))
		{
			compiledShader->Release();
			return nullptr;
		}

		// Release the compiled shader blob, as it's no longer needed
		compiledShader->Release();

		return computeShader;
	}

	ID3D11PixelShader* ShaderLoader::CreatePixelShaderFromCode(const std::string& aShaderCode, const std::string& aEntryName)
	{
		// Compile the shader from file
		ID3DBlob* compiledShader = nullptr;
		ID3DBlob* errorBlob = nullptr;

		//HRESULT hr = D3DCompileFromFile(
		//	aFilePath, // Path to the shader file
		//	nullptr, // Macro definitions (can be nullptr)
		//	D3D_COMPILE_STANDARD_FILE_INCLUDE, // Include handler (can be nullptr)
		//	"main", // Entry point function name
		//	"ps_5_0", // Pixel shader model
		//	0, // Compile flags (can be 0)
		//	0, // Additional compile flags (can be 0)
		//	&compiledShader, // Compiled shader blob
		//	&errorBlob // Error blob (used for error messages)
		//);

		HRESULT hr = D3DCompile(
			aShaderCode.c_str(),
			aShaderCode.size(),
			nullptr,
			nullptr,
			D3D_COMPILE_STANDARD_FILE_INCLUDE,
			aEntryName.c_str(),
			"ps_5_0",
			0,
			0,
			&compiledShader,
			&errorBlob
		);

		if (FAILED(hr))
		{
			if (errorBlob)
			{
				OutputDebugStringA((char*)errorBlob->GetBufferPointer());
				errorBlob->Release();
			}
			return nullptr;
		}

		if (errorBlob)
		{
			errorBlob->Release();
		}

		// Create the pixel shader
		ID3D11PixelShader* pixelShader = nullptr;
		hr = myDevice->CreatePixelShader(
			compiledShader->GetBufferPointer(),
			compiledShader->GetBufferSize(),
			nullptr,
			&pixelShader
		);

		if (FAILED(hr))
		{
			compiledShader->Release();
			return nullptr;
		}

		// Release the compiled shader blob, as it's no longer needed
		compiledShader->Release();

		return pixelShader;
	}

	bool ShaderLoader::RecompileShader(VertexShader* aVertexShader)
	{
		std::string filePath = aVertexShader->GetName();
		//turn this path from a .cso (compiled file) into a .hlsl (source file)
		filePath.replace(filePath.find(".cso"), 4, ".hlsl");
		//strip everything except file name
		std::string fileName = filePath.substr(filePath.find_last_of("/") + 1);

		for (const auto& entry : std::filesystem::recursive_directory_iterator(HLSL_PATH))
		{
			if (entry.path().filename().string() == fileName)
			{
				filePath = entry.path().string();
				break;
			}
		}
		//filePath = HLSL_PATH + fileName;

		//recompile the shader
		ID3D11VertexShader* vertexShader = CreateVertexShaderFromFile(toWstr(filePath).c_str());

		if (vertexShader == nullptr)
		{
			return false;
		}

		aVertexShader->AssignRecompiledShader(vertexShader);

		return true;
	}

	bool ShaderLoader::RecompileShader(PixelShader* aPixelShader)
	{
		std::string filePath = aPixelShader->GetName();
		//turn this path from a .cso (compiled file) into a .hlsl (source file)
		filePath.replace(filePath.find(".cso"), 4, ".hlsl");
		//strip everything except file name
		std::string fileName = filePath.substr(filePath.find_last_of("/") + 1);

		//recursively iterate through the HLSL_PATH folder to find the correct file path
		for (const auto& entry : std::filesystem::recursive_directory_iterator(HLSL_PATH))
		{
			if (entry.path().filename().string() == fileName)
			{
				filePath = entry.path().string();
				break;
			}
		}


		//filePath = HLSL_PATH + fileName;


		//recompile the shader
		ID3D11PixelShader* pixelShader = CreatePixelShaderFromFile(toWstr(filePath).c_str());

		if (pixelShader == nullptr)
		{
			return false;
		}

		aPixelShader->AssignRecompiledShader(pixelShader);

		return true;
	}

	bool ShaderLoader::RecompileShader(ComputeShader* aComputeShader)
	{
		std::string filePath = aComputeShader->GetName();
		//turn this path from a .cso (compiled file) into a .hlsl (source file)
		filePath.replace(filePath.find(".cso"), 4, ".hlsl"); 
		//strip everything except file name
		std::string fileName = filePath.substr(filePath.find_last_of("/") + 1);

		filePath = HLSL_PATH + fileName;

		//recompile the shader
		ID3D11ComputeShader* computeShader = CreateComputeShaderFromFile(toWstr(filePath).c_str());

		if (computeShader == nullptr)
		{
			return false;
		}

		aComputeShader->AssignRecompiledShader(computeShader);

		return true;
	}

	bool ShaderLoader::RecompileShader(const std::string& aFilePath)
	{
		std::string path = SHADER_LOAD_PATH + aFilePath;

		Sleep(SHADER_RECOMPILATION_SLEEP);

		if (path.find(VERTEX_SHADER_KEY) != std::string::npos)
		{
			path.replace(path.find(".hlsl"), 5, ".cso");
			VertexShader& vertexShader = myVertexShaders[path];
			return RecompileShader(&vertexShader);
		}
		else if (path.find(PIXEL_SHADER_KEY) != std::string::npos)
		{
			path.replace(path.find(".hlsl"), 5, ".cso");
			PixelShader& pixelShader = myPixelShaders[path];
			return RecompileShader(&pixelShader);
		}
		else
		{
			return false;
		}
	}



	std::vector<VertexShader*> ShaderLoader::GetVertexShadersVector() const
	{
		std::vector<VertexShader*> vertexShaders;
		for (auto& shader : myVertexShaders)
		{
			vertexShaders.push_back((VertexShader*)&shader.second);
		}
		return vertexShaders;
	}
}
