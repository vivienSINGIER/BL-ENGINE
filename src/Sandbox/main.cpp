#include "pch.h"
#include <windows.h>
#include "main.h"

#include "Tests.h"

#ifdef _DEBUG
int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	Console::InitConsol();
	
	TestCameraFrustum test;
	
	test.Run();

	float a;
	std::cin >> a;
	
	Console::DeleteConsol();
	return 0;
}
#else
int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	TestCollision::Run();
	
	return 0;
}

#endif // !_DEBUG