#pragma once
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <Windows.h>
#include "d3dx12.h"
#include <thread>
#include <vector>

void RunAsyncExport(void* vbData, UINT vbSize, UINT stride,
    void* ibData, UINT ibSize, DXGI_FORMAT format);

void ExportToObj(const void* vbData, UINT vbSize, UINT stride,
    const void* ibData, UINT ibSize, DXGI_FORMAT format);