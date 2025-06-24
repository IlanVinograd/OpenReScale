#include "WrappedCommandQueue.h"
#include "logger.h"

ID3D12Resource* g_PreDLSS_RenderTarget = nullptr;

void STDMETHODCALLTYPE WrappedCommandQueue::ExecuteCommandLists(UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    Logger::LogInfo() << "ExecuteCommandLists: " << NumCommandLists << " lists submitted" << std::endl;

    for (UINT i = 0; i < NumCommandLists; ++i) {
        ID3D12GraphicsCommandList* gcl = nullptr;
        if (SUCCEEDED(ppCommandLists[i]->QueryInterface(IID_PPV_ARGS(&gcl)))) {
            Logger::LogDebug() << "Captured a graphics command list before execution" << std::endl;

            gcl->Release();
        }
    }

    ID3D12Device* device = nullptr;
    m_real->GetDevice(IID_PPV_ARGS(&device));
    if (g_PreDLSS_RenderTarget)
    {
        //SaveResourceToPng(device, gcl, g_PreDLSS_RenderTarget, L"pre_dlss.png");
    }

    m_real->ExecuteCommandLists(NumCommandLists, ppCommandLists);
}
