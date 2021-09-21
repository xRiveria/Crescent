#pragma once
#include <wrl/client.h>

using namespace Microsoft::WRL;

namespace Aurora
{
    struct RHI_Context
    {
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

        // Device Limits
        static inline uint32_t m_Texture2D_Dimensions_Max = 16384; // As per DX11 D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION.
    };
}

