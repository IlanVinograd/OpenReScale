#include "WrappedCommandQueue.h"
#include "logger.h"

ID3D12Resource* g_PreDLSS_RenderTarget = nullptr;

void STDMETHODCALLTYPE WrappedCommandQueue::ExecuteCommandLists(UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists)
{
    Logger::LogInfo() << "ExecuteCommandLists: " << NumCommandLists << " lists submitted" << std::endl;

    ID3D12GraphicsCommandList* gcl = nullptr;

    for (UINT i = 0; i < NumCommandLists; ++i) {
        if (SUCCEEDED(ppCommandLists[i]->QueryInterface(IID_PPV_ARGS(&gcl)))) {
            Logger::LogDebug() << "Captured a graphics command list before execution" << std::endl;
            break;
        }
    }

    if (g_PreDLSS_RenderTarget && gcl) {
        ID3D12Device* device = nullptr;
        m_real->GetDevice(IID_PPV_ARGS(&device));

        //SaveResourceToPng(device, gcl, g_PreDLSS_RenderTarget, L"pre_dlss.png");
    }

    if (gcl) gcl->Release();

    m_real->ExecuteCommandLists(NumCommandLists, ppCommandLists);
}
