#include "pch.h"
#include <windows.h>
#include "main.h"

#include "Tests.h"
#include "Tests/DemoLight.hpp"

#ifdef _DEBUG
int WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	Console::InitConsol();
	/////////////////////////////////////////////////////////////////////////////
	
	TestECS::Run();

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