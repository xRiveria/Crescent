#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#include <d3d11_4.h>

namespace Aurora
{
    struct RHI_Context
    {
        ID3D11Device5* m_Device = nullptr;
        ID3D11DeviceContext4* m_DeviceContext = nullptr;
        ID3DUserDefinedAnnotation* m_Annotation = nullptr;

        // Debugging
#ifdef _DEBUG
        bool m_DebuggingEnabled = true;
        bool m_MarkersEnabled = true;
        bool m_ProfilingEnabled = true;
#else
        bool m_DebuggingEnabled = false;
        bool m_MarkersEnabled = false;
        bool m_ProfilingEnabled = true;
#endif
    };
}