#include "ObjectExporter.h"

void RunAsyncExport(void* vbData, UINT vbSize, UINT stride,
    void* ibData, UINT ibSize, DXGI_FORMAT format)
{
    std::vector<char> vbCopy((char*)vbData, (char*)vbData + vbSize);
    std::vector<char> ibCopy((char*)ibData, (char*)ibData + ibSize);

    std::thread([vbCopy = std::move(vbCopy), ibCopy = std::move(ibCopy), stride, format]() {
        ExportToObj(vbCopy.data(), (UINT)vbCopy.size(), stride, ibCopy.data(), (UINT)ibCopy.size(), format);
        }).detach();
}

void ExportToObj(const void* vbData, UINT vbSize, UINT stride,
    const void* ibData, UINT ibSize, DXGI_FORMAT format) {
    static int counter = 0;
    std::ostringstream nameStream;
    nameStream << "Exported/obj_" << std::setw(4) << std::setfill('0') << counter++ << ".obj";
    std::filesystem::create_directories("Exported");

    std::ofstream obj(nameStream.str());
    if (!obj) return;

    const char* vb = reinterpret_cast<const char*>(vbData);

    UINT vertexCount = vbSize / stride;
    for (UINT i = 0; i < vertexCount; ++i) {
        const float* pos = reinterpret_cast<const float*>(vb + i * stride);
        obj << "v " << pos[0] << " " << pos[1] << " " << pos[2] << "\n";
    }

    const char* ib = reinterpret_cast<const char*>(ibData);
    UINT indexCount = ibSize / (format == DXGI_FORMAT_R16_UINT ? 2 : 4);

    for (UINT i = 0; i + 2 < indexCount; i += 3) {
        UINT i0, i1, i2;
        if (format == DXGI_FORMAT_R16_UINT) {
            const uint16_t* indices = reinterpret_cast<const uint16_t*>(ib);
            i0 = indices[i] + 1;
            i1 = indices[i + 1] + 1;
            i2 = indices[i + 2] + 1;
        }
        else {
            const uint32_t* indices = reinterpret_cast<const uint32_t*>(ib);
            i0 = indices[i] + 1;
            i1 = indices[i + 1] + 1;
            i2 = indices[i + 2] + 1;
        }
        obj << "f " << i0 << " " << i1 << " " << i2 << "\n";
    }

    obj.close();
}