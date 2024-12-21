#pragma once
#include <thread>
#include <mutex>

namespace KE
{
	class Graphics;

	class AssetLoader
	{
	private:
		std::thread myTextureThread;
		std::thread myModelThread;
		std::thread myShaderThread;

		Graphics* myGraphics;

		bool isInit = false;


		
		void Load();

		void TextureLoad();
		void ModelLoad();
		void ShaderLoad();


	public:
		AssetLoader(Graphics* aGraphics);
		~AssetLoader();

		void CopyAssets();

		void Init();

		void Update();

	};
}