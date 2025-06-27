#include "WrappedCommandList.h"

D3D12_VERTEX_BUFFER_VIEW lastVBView = {};
D3D12_INDEX_BUFFER_VIEW lastIBView = {};
std::vector<ResourceRange> g_Resources;

void STDMETHODCALLTYPE WrappedCommandList::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) {
    //Logger::LogInfo() << "DrawInstanced: " << InstanceCount << std::endl;
    m_real->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void STDMETHODCALLTYPE WrappedCommandList::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) {
    //Logger::LogInfo() << "DrawIndexedInstanced: " << IndexCountPerInstance << std::endl;



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