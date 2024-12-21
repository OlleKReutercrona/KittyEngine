#pragma once
#include <d3d11.h>

#include "Shader.h"

#include <string>
#include <unordered_map>

struct ID3D11Device;

#define VERTEX_SHADER_KEY "VS"
#define PIXEL_SHADER_KEY "PS"
#define COMPUTE_SHADER_KEY "CS"


#define SHADER_LOAD_PATH "Shaders/" //this is fuck

#define DEFAULT_VERTEX_SHADER SHADER_LOAD_PATH "Model_Deferred_Instanced_VS.cso"
#define DEFAULT_PIXEL_SHADER SHADER_LOAD_PATH "Model_Deferred_PS.cso"

namespace KE
{
	struct ParsingOutput;
	class Script;

	class ShaderLoader
	{
	private:
		std::unordered_map<std::string, VertexShader> myVertexShaders;
		std::unordered_map<std::string, PixelShader> myPixelShaders;
		std::unordered_map<std::string, ComputeShader> myComputeShaders;

		ID3D11Device* myDevice = {};

		void FindAndLoadShaders();

	public:
		void ActivateFileWatcher();

		void Init(ID3D11Device* aDevice);
		void Clear();

		VertexShader* CreateVertexShaderFromParsedScript(const std::string& aShaderName, const std::string& aParsedCode);
		VertexShader* CreateVertexShaderFromScript(const std::string& aShaderName, Script* aShaderScript);
		
		PixelShader* CreatePixelShaderFromParsedScript(const std::string& aShaderName, const std::string& aParsedCode, const std::string& aEntryName = "main");
		PixelShader* CreatePixelShaderFromScript(const std::string& aShaderName, Script* aShaderScript, const std::string& aEntryName = "main");

		VertexShader* GetVertexShader(std::string aFilePath);
		PixelShader* GetPixelShader(std::string aFilePath);
		ComputeShader* GetComputeShader(std::string aFilePath);


		const std::unordered_map<std::string, VertexShader>& GetVertexShaders() const { return myVertexShaders; }
		const std::unordered_map<std::string, PixelShader>& GetPixelShaders() const { return myPixelShaders; }
		const std::unordered_map<std::string, ComputeShader>& GetComputeShaders() const { return myComputeShaders; }

		std::vector<VertexShader*> GetVertexShadersVector() const;
		std::vector<PixelShader*> GetPixelShadersVector() const;
		std::vector<ComputeShader*> GetComputeShadersVector() const;

		ID3D11VertexShader* CreateVertexShaderFromFile(const wchar_t* aFilePath);
		ID3D11PixelShader* CreatePixelShaderFromFile(const wchar_t* aFilePath);
		ID3D11ComputeShader* CreateComputeShaderFromFile(const wchar_t* aFilePath);

		ID3D11PixelShader* CreatePixelShaderFromCode(const std::string& aShaderCode, const std::string& aEntryName = "main");

		inline VertexShader* GetDefaultVertexShader() { return GetVertexShader(DEFAULT_VERTEX_SHADER); }
		inline PixelShader* GetDefaultPixelShader() { return GetPixelShader(DEFAULT_PIXEL_SHADER); }

		bool RecompileShader(VertexShader* aVertexShader);
		bool RecompileShader(PixelShader* aVertexShader);
		bool RecompileShader(ComputeShader* aComputeShader);

		bool RecompileShader(const std::string& aFilePath);
	};
}
