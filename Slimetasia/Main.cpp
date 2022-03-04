#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

#include "Application.h"
#include "LinkedList.h"
#include "List.h"
#include "PEVector.h"
#include "Pair.h"

#ifdef ENABLE_MEMORY_MANAGER
MemoryManager g_MemoryManager;
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
#ifdef _DEBUG
    _CrtDumpMemoryLeaks();
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    //_CrtSetBreakAlloc(7238936);
#endif

    Application::Initialize(hInstance, hPrevInstance, szCmdLine, iCmdShow);
    Application::Instance().RunMainLoop();
    Application::Shutdown();

#ifdef ENABLE_MEMORY_MANAGER
    g_MemoryManager.Memory_Leaks_report();
#endif

    return 0;
}
