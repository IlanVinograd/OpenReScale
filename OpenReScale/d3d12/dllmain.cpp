#include <Windows.h>
#include <d3d12.h>
#include "dllmain.h"
#include "logger.h"

static HMODULE d3d12dll = nullptr;

PFN_D3D12CreateDevice m_D3D12CreateDevice;

FARPROC m_SetAppCompatStringPointer;
//FARPROC m_D3D12CreateDevice;
FARPROC m_D3D12GetDebugInterface;
FARPROC m_D3D12CoreCreateLayeredDevice;
FARPROC m_D3D12CoreGetLayeredDeviceSize;
FARPROC m_D3D12CoreRegisterLayers;
FARPROC m_D3D12CreateRootSignatureDeserializer;
FARPROC m_D3D12CreateVersionedRootSignatureDeserializer;
FARPROC m_D3D12DeviceRemovedExtendedData;
FARPROC m_D3D12EnableExperimentalFeatures;
FARPROC m_D3D12GetInterface;
FARPROC m_D3D12PIXEventsReplaceBlock;
FARPROC m_D3D12PIXGetThreadInfo;
FARPROC m_D3D12PIXNotifyWakeFromFenceSignal;
FARPROC m_D3D12PIXReportCounter;
FARPROC m_D3D12SerializeRootSignature;
FARPROC m_D3D12SerializeVersionedRootSignature;
FARPROC m_GetBehaviorValue;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH: 
        {
            Logger::Init();
            Logger::LogInfo() << "DLL PROCESS ATTACH - Successfully" << std::endl;

            char path[MAX_PATH];
            GetSystemDirectoryA(path, MAX_PATH);
            strcat_s(path, "\\d3d12.dll");
            Logger::LogInfo() << "Loading " << path << std::endl;
            d3d12dll = LoadLibraryA(path);

            m_D3D12CreateDevice = (PFN_D3D12CreateDevice)GetProcAddress(d3d12dll, "D3D12CreateDevice");
            m_SetAppCompatStringPointer = GetProcAddress(d3d12dll,"SetAppCompatStringPointer");
            m_D3D12GetDebugInterface = GetProcAddress(d3d12dll, "D3D12GetDebugInterface");
            m_D3D12CoreCreateLayeredDevice = GetProcAddress(d3d12dll, "D3D12CoreCreateLayeredDevice");
            m_D3D12CoreGetLayeredDeviceSize = GetProcAddress(d3d12dll, "D3D12CoreGetLayeredDeviceSize");
            m_D3D12CoreRegisterLayers = GetProcAddress(d3d12dll, "D3D12CoreRegisterLayers");
            m_D3D12CreateRootSignatureDeserializer = GetProcAddress(d3d12dll, "D3D12CreateRootSignatureDeserializer");
            m_D3D12CreateVersionedRootSignatureDeserializer = GetProcAddress(d3d12dll, "D3D12CreateVersionedRootSignatureDeserializer");
            m_D3D12DeviceRemovedExtendedData = GetProcAddress(d3d12dll, "D3D12DeviceRemovedExtendedData");
            m_D3D12EnableExperimentalFeatures = GetProcAddress(d3d12dll, "D3D12EnableExperimentalFeatures");
            m_D3D12GetInterface = GetProcAddress(d3d12dll, "D3D12GetInterface");
            m_D3D12PIXEventsReplaceBlock = GetProcAddress(d3d12dll, "D3D12PIXEventsReplaceBlock");
            m_D3D12PIXGetThreadInfo = GetProcAddress(d3d12dll, "D3D12PIXGetThreadInfo");
            m_D3D12PIXNotifyWakeFromFenceSignal = GetProcAddress(d3d12dll, "D3D12PIXNotifyWakeFromFenceSignal");
            m_D3D12PIXReportCounter = GetProcAddress(d3d12dll, "D3D12PIXReportCounter");
            m_D3D12SerializeRootSignature = GetProcAddress(d3d12dll, "D3D12SerializeRootSignature");
            m_D3D12SerializeVersionedRootSignature = GetProcAddress(d3d12dll, "D3D12SerializeVersionedRootSignature");
            m_GetBehaviorValue = GetProcAddress(d3d12dll, "GetBehaviorValue");

            break;
        }
        
        case DLL_PROCESS_DETACH: 
        {
            Logger::LogInfo() << "DLL PROCESS DETACH - Successfully" << std::endl;
            Logger::Shutdown();

            FreeLibrary(d3d12dll);
            break;
        }
    }
    return TRUE;
}

HRESULT D3D12CreateDevice(IUnknown* pAdapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, REFIID riid, void** ppDevice) {
    HRESULT original = S_OK;

    D3D_FEATURE_LEVEL ModifiedFeatureLevel = D3D_FEATURE_LEVEL_11_0;

    if (m_D3D12CreateDevice) Logger::LogInfo() << "D3D12CreateDevice - Address Available" << std::endl;
    else {
                             Logger::LogError() << "D3D12CreateDevice - Address Unavailable" << std::endl;
                             return S_FALSE;
    }

    original = m_D3D12CreateDevice(pAdapter, ModifiedFeatureLevel, riid, ppDevice);
    if (original == S_OK) Logger::LogDebug() << "D3D12CreateDevice - Success" << std::endl;
    if (original == S_FALSE) Logger::LogDebug() << "D3D12CreateDevice - Failed" << std::endl;

    if(original >= 0x887A0000) Logger::LogError() << "Possible DXGI_ERROR code" << std::endl;

    return original;
}