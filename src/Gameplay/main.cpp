#include "pch.h"

#include "../Core/define.h"

#include "main.h"
#include "Engine/Engine.h"
#include "../Gameplay/Scene/MainScene.h"

#ifdef _DEBUG
int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    srand(time(NULL));
    Console::InitConsol();
    
    EngineManager engineManager;
    engineManager.Initialize(1920, 1080, L"LabyBL");
     
	SceneManager::CreateSceneType<MainScene>("MainScene");
	SceneManager::SetCurrentScene("MainScene");

    engineManager.Run();
    
    //Console::DeleteConsol();
    return 0;
}
#else
int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	
    return 0;
}

#endif // !_DEBUG