#include "WrappedCommandList.h"

D3D12_VERTEX_BUFFER_VIEW lastVBView = {};
D3D12_INDEX_BUFFER_VIEW lastIBView = {};

void STDMETHODCALLTYPE WrappedCommandList::DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount, UINT StartVertexLocation, UINT StartInstanceLocation) {
    Logger::LogInfo() << "DrawInstanced: " << InstanceCount << std::endl;
    m_real->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void STDMETHODCALLTYPE WrappedCommandList::DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation, INT BaseVertexLocation, UINT StartInstanceLocation) {
    Logger::LogInfo() << "DrawIndexedInstanced: " << IndexCountPerInstance << std::endl;
    m_real->DrawIndexedInstanced(IndexCountPerInstance, InstanceCount, StartIndexLocation, BaseVertexLocation, StartInstanceLocation);
}

void STDMETHODCALLTYPE WrappedCommandList::IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pView) {
    if (pView)
        lastIBView = *pView;
    m_real->IASetIndexBuffer(pView);
}
void STDMETHODCALLTYPE WrappedCommandList::IASetVertexBuffers(UINT StartSlot, UINT NumViews, const D3D12_VERTEX_BUFFER_VIEW* pViews) {
    if (StartSlot == 0 && NumViews > 0)
        lastVBView = pViews[0];
    m_real->IASetVertexBuffers(StartSlot, NumViews, pViews);
}