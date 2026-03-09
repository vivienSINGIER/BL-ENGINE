#include "pch.h"
#include <windows.h>
#include "main.h"

#ifdef _DEBUG
int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    Console::InitConsol();
    
    /////////////////////////////////////////////////////////////////////////////
    // EngineManager engineManager;
    // engineManager.Initialize(1920, 1080, L"DON'T BLINK TO MUCH", true);
    //
    // SceneManager::CreateSceneType<SplashScreen>("SplashScreen");
    // SceneManager::ChangeCurrentScene("SplashScreen");
    //
    // engineManager.Run();
    
    Console::DeleteConsol();
    return 0;
}
#else
int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	
    return 0;
}

#endif // !_DEBUG