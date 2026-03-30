#include "pch.h"

#include "../Core/define.h"

#include "main.h"
#include "Engine/Engine.h"

#ifdef _DEBUG
int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    Console::InitConsol();
    
     EngineManager engineManager;
     engineManager.Initialize(1920, 1080, L"LabyBL");
     
	 SceneManager::CreateScene("Default");
	 SceneManager::ChangeCurrentScene("Default");

     engineManager.Run();
    
    Console::DeleteConsol();
    return 0;
}
#else
int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	
    return 0;
}

#endif // !_DEBUG