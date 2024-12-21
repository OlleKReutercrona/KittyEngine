#pragma once
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11ComputeShader;
struct ID3D11InputLayout;
struct ID3D11Device;

namespace KE
{
	class Shader
	{
		KE_EDITOR_FRIEND
		friend class ShaderLoader;
	public:
		inline void SetName(std::string aName) { myName = aName; }
		inline const std::string& GetName() const { return myName; }


		enum ShaderType
		{
			eVertexShader,
			ePixelShader
		};

		virtual void Clear() = 0;

		inline ShaderType GetType() { return myShaderType; }

		inline void SetSlot(char aSlot) { myActiveSlot = aSlot; }

		inline const std::string& GetShaderData() { return myShaderData; }

	protected:
		std::string myName;
		ShaderType myShaderType;
		std::string myShaderData;

		char myActiveSlot = 0;
	};

	class PixelShader : public Shader
	{
		friend class ShaderLoader;

	private:
		ComPtr<ID3D11PixelShader> myPixelShaderSlots[2];

	public:
		//Use ShaderLoader for this instead of constructing it here!

		PixelShader();
		void Init(const std::string& aFilePath, ID3D11Device* aDevice);
		void Clear() override;

		bool Failed() const { return myPixelShaderSlots[0] == nullptr; }
		ID3D11PixelShader* GetShader() const { return myPixelShaderSlots[myActiveSlot].Get(); }

		//This should never be manually called!
		void AssignRecompiledShader(ID3D11PixelShader* aShader, bool aSwitchSlot = true);
	};

	class VertexShader : public Shader
	{
		friend class ShaderLoader;

	private:
		ComPtr<ID3D11VertexShader> myVertexShaderSlots[2];
		ComPtr<ID3D11InputLayout> myInputLayout;

	public:
		//Use ShaderLoader for this instead of constructing it here!
		VertexShader();
		void Init(const std::string& aFilePath, ID3D11Device* aDevice);
		void Clear() override;


		bool Failed() const { return myVertexShaderSlots[0] == nullptr; }

		ID3D11VertexShader* GetShader() const { return myVertexShaderSlots[myActiveSlot].Get(); }
		ID3D11InputLayout* GetInputLayout() const { return myInputLayout.Get(); }

		void AssignRecompiledShader(ID3D11VertexShader* aShader, bool aSwitchSlot = true);
	};

	class ComputeShader : public Shader
	{
	private:
		ComPtr<ID3D11ComputeShader> myComputeShaderSlots[2];

	public:
		ComputeShader();
		void Init(const std::string& aFilePath, ID3D11Device* aDevice);
		void Clear() override;


		bool Failed() const { return myComputeShaderSlots[0] == nullptr; }

		ID3D11ComputeShader* GetShader() const { return myComputeShaderSlots[myActiveSlot].Get(); }
		void AssignRecompiledShader(ID3D11ComputeShader* aShader, bool aSwitchSlot = true);
	};
}
