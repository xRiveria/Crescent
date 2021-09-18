#pragma once
#include "../RHI_Context.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#include <d3d11_4.h>

namespace Aurora
{
    struct DX11_Context : public RHI_Context
    {
        ID3D11Device5* m_Device = nullptr;
        ID3D11DeviceContext4* m_DeviceContext = nullptr;
        ID3DUserDefinedAnnotation* m_Annotation = nullptr;
    };
}