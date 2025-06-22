#include <Windows.h>
#include "logger.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH: 
        {
            Logger::Init();
            Logger::LogInfo() << "DLL PROCESS ATTACH - Successfully" << std::endl;

            break;
        }

        case DLL_PROCESS_DETACH: 
        {
            Logger::LogInfo() << "DLL PROCESS DETACH - Successfully" << std::endl;
            Logger::Shutdown();

            break;
        }
    }
    return TRUE;
}