#include "WrappedCommandList.h"
#include "WrappedDevice.h"
#include "ObjectExporter.h"
#include "d3dx12.h"
#include <chrono>

D3D12_VERTEX_BUFFER_VIEW lastVBView = {};
D3D12_INDEX_BUFFER_VIEW lastIBView = {};
std::vector<ResourceRange> g_Resources;

static bool wasPressed = false;
static std::unordered_set<size_t> g_exportedHashes;

void STDMETHODCALLTYPE WrappedCommandList::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) {
    //Logger::LogInfo() << "DrawInstanced: " << InstanceCount << std::endl;
    m_real->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void STDMETHODCALLTYPE WrappedCommandList::DrawIndexedInstanced(
    UINT IndexCountPerInstance,
    UINT InstanceCount,
    UINT StartIndexLocation,
    INT BaseVertexLocation,
    UINT StartInstanceLocation)
{
    static bool g_deviceLost = false;
    static bool wasPressed = false;
    static std::chrono::steady_clock::time_point lastSaveTime = std::chrono::steady_clock::now();
    const std::chrono::milliseconds kMinInterval(400);

    if (g_deviceLost)
    {
        m_real->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        return;
    }

    ID3D12Device* device = nullptr;
    if (FAILED(m_real->GetDevice(__uuidof(ID3D12Device), (void**)&device)) || !device)
    {
        Logger::LogError() << "[DrawIndexedInstanced] Failed to get ID3D12Device.";
        return;
    }

    if (device->GetDeviceRemovedReason() != S_OK)
    {
        Logger::LogError() << "[DrawIndexedInstanced] Device was removed.";
        g_deviceLost = true;
        device->Release();
        return;
    }

    if (lastVBView.BufferLocation == 0 || lastVBView.BufferLocation == ~0ull ||
        lastIBView.BufferLocation == 0 || lastIBView.BufferLocation == ~0ull)
    {
        Logger::LogError() << "[DrawIndexedInstanced] Invalid buffer locations.";
        device->Release();
        m_real->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
        return;
    }

    ID3D12Resource* vbResource = nullptr;
    ID3D12Resource* ibResource = nullptr;
    for (const auto& entry : g_Resources)
    {
        if (!vbResource && lastVBView.BufferLocation >= entry.base && lastVBView.BufferLocation < entry.base + entry.size)
            vbResource = entry.resource;
        if (!ibResource && lastIBView.BufferLocation >= entry.base && lastIBView.BufferLocation < entry.base + entry.size)
            ibResource = entry.resource;
        if (vbResource && ibResource) break;
    }

    SHORT keyState = GetAsyncKeyState('N');
    bool isPressed = (keyState & 0x8000) != 0;

    if (isPressed)
    {
        auto now = std::chrono::steady_clock::now();
        if (now - lastSaveTime < kMinInterval)
            goto CleanupAndDraw;

        lastSaveTime = now;

        if (!vbResource || !ibResource)
        {
            Logger::LogWarning() << "[DrawIndexedInstanced] Buffers not found.";
            goto CleanupAndDraw;
        }

        UINT64 vbOffset = lastVBView.BufferLocation - vbResource->GetGPUVirtualAddress();
        UINT64 ibOffset = lastIBView.BufferLocation - ibResource->GetGPUVirtualAddress();
        UINT vbSize = lastVBView.SizeInBytes;
        UINT ibSize = lastIBView.SizeInBytes;

        if (vbSize < kMinSaveSize || ibSize < kMinSaveSize)
        {
            Logger::LogInfo() << "[DrawIndexedInstanced] Skipped: buffer too small.";
            goto CleanupAndDraw;
        }
        if (vbSize > kMaxSaveSize || ibSize > kMaxSaveSize)
        {
            Logger::LogWarning() << "[DrawIndexedInstanced] Skipped: buffer too large.";
            goto CleanupAndDraw;
        }

        auto vbDesc = vbResource->GetDesc();
        auto ibDesc = ibResource->GetDesc();
        if (vbOffset + vbSize > vbDesc.Width || ibOffset + ibSize > ibDesc.Width)
        {
            Logger::LogError() << "[DrawIndexedInstanced] Readback exceeds resource bounds.";
            goto CleanupAndDraw;
        }

        ID3D12Resource* vbReadback = nullptr;
        ID3D12Resource* ibReadback = nullptr;
        ID3D12CommandAllocator* allocator = nullptr;
        ID3D12GraphicsCommandList* cmdList = nullptr;
        ID3D12CommandQueue* queue = nullptr;
        ID3D12Fence* fence = nullptr;
        HANDLE fenceEvent = nullptr;
        bool success = true;

        CD3DX12_HEAP_PROPERTIES readbackHeap(D3D12_HEAP_TYPE_READBACK);
        CD3DX12_RESOURCE_DESC vbDescBuffer = CD3DX12_RESOURCE_DESC::Buffer(vbSize);
        CD3DX12_RESOURCE_DESC ibDescBuffer = CD3DX12_RESOURCE_DESC::Buffer(ibSize);

        if (FAILED(device->CreateCommittedResource(&readbackHeap, D3D12_HEAP_FLAG_NONE, &vbDescBuffer,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&vbReadback))))
        {
            Logger::LogError() << "[DrawIndexedInstanced] Failed to create vbReadback.";
            success = false;
        }

        if (success && FAILED(device->CreateCommittedResource(&readbackHeap, D3D12_HEAP_FLAG_NONE, &ibDescBuffer,
            D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&ibReadback))))
        {
            Logger::LogError() << "[DrawIndexedInstanced] Failed to create ibReadback.";
            success = false;
        }

        if (success && FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&allocator))))
        {
            Logger::LogError() << "[DrawIndexedInstanced] Failed to create command allocator.";
            success = false;
        }

        if (success && FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator, nullptr, IID_PPV_ARGS(&cmdList))))
        {
            Logger::LogError() << "[DrawIndexedInstanced] Failed to create command list.";
            success = false;
        }

        if (success)
        {
            CD3DX12_RESOURCE_BARRIER vbBarrier = CD3DX12_RESOURCE_BARRIER::Transition(vbResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
            CD3DX12_RESOURCE_BARRIER ibBarrier = CD3DX12_RESOURCE_BARRIER::Transition(ibResource, D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_SOURCE);
            cmdList->ResourceBarrier(1, &vbBarrier);
            cmdList->ResourceBarrier(1, &ibBarrier);
            cmdList->CopyBufferRegion(vbReadback, 0, vbResource, vbOffset, vbSize);
            cmdList->CopyBufferRegion(ibReadback, 0, ibResource, ibOffset, ibSize);
            cmdList->Close();
        }

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        if (success && FAILED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue))))
        {
            Logger::LogError() << "[DrawIndexedInstanced] Failed to create command queue.";
            success = false;
        }

        if (success && FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence))))
        {
            Logger::LogError() << "[DrawIndexedInstanced] Failed to create fence.";
            success = false;
        }

        if (success)
        {
            fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
            if (!fenceEvent)
            {
                Logger::LogError() << "[DrawIndexedInstanced] Failed to create fence event.";
                success = false;
            }
        }

        if (success)
        {
            queue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&cmdList);
            queue->Signal(fence, 1);
            fence->SetEventOnCompletion(1, fenceEvent);
            WaitForSingleObject(fenceEvent, INFINITE);
        }

        if (success && device->GetDeviceRemovedReason() != S_OK)
        {
            Logger::LogError() << "[DrawIndexedInstanced] Device lost during readback.";
            g_deviceLost = true;
            success = false;
        }

        if (success)
        {
            void* vbData = nullptr;
            void* ibData = nullptr;

            if (SUCCEEDED(vbReadback->Map(0, nullptr, &vbData)) &&
                SUCCEEDED(ibReadback->Map(0, nullptr, &ibData)))
            {
                size_t hash = HashBuffers(vbData, vbSize, ibData, ibSize);
                if (g_exportedHashes.find(hash) != g_exportedHashes.end())
                {
                    Logger::LogInfo() << "[DrawIndexedInstanced] Skipped duplicate mesh.";
                }
                else
                {
                    Logger::LogInfo() << "[DrawIndexedInstanced] Readback success. vb=" << vbData << ", ib=" << ibData;
                    g_exportedHashes.insert(hash);
                    RunAsyncExport(vbData, vbSize, lastVBView.StrideInBytes, ibData, ibSize, lastIBView.Format);
                }
                vbReadback->Unmap(0, nullptr);
                ibReadback->Unmap(0, nullptr);
            }
            else
            {
                Logger::LogError() << "[DrawIndexedInstanced] Failed to map readback buffers.";
            }
        }

        if (fenceEvent) CloseHandle(fenceEvent);
        if (fence) fence->Release();
        if (queue) queue->Release();
        if (cmdList) cmdList->Release();
        if (allocator) allocator->Release();
        if (vbReadback) vbReadback->Release();
        if (ibReadback) ibReadback->Release();
    }

CleanupAndDraw:
    wasPressed = isPressed;
    device->Release();
    m_real->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void STDMETHODCALLTYPE WrappedCommandList::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pView) {
    if (pView) {
        lastIBView = *pView;
        /*Logger::LogInfo() << "BufferLocation: " << lastIBView.BufferLocation << std::endl;
        Logger::LogInfo() << "SizeInBytes: " << lastIBView.SizeInBytes << std::endl;
        Logger::LogInfo() << "Format: " << lastIBView.Format << std::endl;*/
    }

    m_real->IASetIndexBuffer(pView);
}

void STDMETHODCALLTYPE WrappedCommandList::IASetVertexBuffers(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews) {
    if (StartSlot == 0 && NumViews > 0) {
        lastVBView = pViews[0];
        /*Logger::LogInfo() << "BufferLocation: " << lastVBView.BufferLocation << std::endl;
        Logger::LogInfo() << "SizeInBytes: " << lastVBView.SizeInBytes << std::endl;
        Logger::LogInfo() << "Format: " << lastVBView.StrideInBytes << std::endl;*/
    }

    m_real->IASetVertexBuffers(StartSlot, NumViews, pViews);
}

void STDMETHODCALLTYPE WrappedCommandList::SetGraphicsRootShaderResourceView(UINT RootParameterIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferLocation) {
    Logger::LogInfo() << "SetGraphicsRootShaderResourceView[" << RootParameterIndex << "] -> GPU addr: 0x" << std::hex << BufferLocation;

    for (const auto& entry : g_Resources) {
        if (BufferLocation >= entry.base && BufferLocation < entry.base + entry.size) {
            Logger::LogInfo() << "Matched resource: " << entry.resource;
            // Можешь сохранить это соответствие (например, currentSRV[RootParameterIndex] = entry.resource)
        }
    }

    m_real->SetGraphicsRootShaderResourceView(RootParameterIndex, BufferLocation);
}