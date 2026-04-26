#include "stdafx.h"

#pragma message("KITTY ENGINE IS HERE TO STAY AND SLAY")

#include "Game.h"

class Copier
{
public:
	Copier()
	{
		KE::AssetLoader(nullptr).CopyAssets();
	}
};

#ifndef KITTYENGINE_SHIP
inline Copier copier;
#endif

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
#ifndef KITTYENGINE_SHIP2
	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	// KE Logger prefers the stderr stream for error logging.
	freopen_s(&fp, "CONOUT$", "w", stderr);
#endif // KITTYENGINE_SHIP

	// The right method call in the wrong place can make all the difference in the world. -- Vilhelm
	KE_GLOBAL::audioPlayer.Init();

	//try {
		KE::Game{}.Go();
	//}
	//catch(std::exception& e)
	//{ 
	//	std::string str;
	//	str = e.what();

	//	wchar_t wtext[200];
	//	size_t outSize;
	//	mbstowcs_s(&outSize, wtext, str.length(), str.c_str(), str.length() - 1);

	//	MessageBox(nullptr, wtext, L"Exception!", MB_OK | MB_ICONEXCLAMATION);
	//	return EXIT_FAILURE;
	//}
	return EXIT_SUCCESS;
}
