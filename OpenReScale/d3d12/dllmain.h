#pragma once

using PFN_D3D12CreateDevice = HRESULT(__stdcall*)(IUnknown*, D3D_FEATURE_LEVEL, REFIID, void**);
using PFN_D3D12CreateRootSignatureDeserializer = HRESULT(__stdcall*)(LPCVOID, SIZE_T, REFIID, void**);
using PFN_D3D12CreateVersionedRootSignatureDeserializer = HRESULT(__stdcall*)(LPCVOID, SIZE_T, REFIID, void**);
using PFN_D3D12EnableExperimentalFeatures = HRESULT(__stdcall*)(UINT, const IID*, void*, UINT*);
using PFN_D3D12GetDebugInterface = HRESULT(__stdcall*)(REFIID, void**);
using PFN_D3D12GetInterface = HRESULT(__stdcall*)(REFCLSID, REFIID, void**);
using PFN_D3D12SerializeRootSignature = HRESULT(__stdcall*)(const D3D12_ROOT_SIGNATURE_DESC*, D3D_ROOT_SIGNATURE_VERSION, ID3DBlob**, ID3DBlob**);
using PFN_D3D12SerializeVersionedRootSignature = HRESULT(__stdcall*)(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC*, ID3DBlob**, ID3DBlob**);