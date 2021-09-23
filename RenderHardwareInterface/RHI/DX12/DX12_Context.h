#pragma once
#include "../RHI_Context.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include <dxgi1_4.h>
#include <d3d12.h>
#include "d3dx12.h"

namespace Aurora
{
    struct DX12_Context : public RHI_Context
    {
        ID3D12Device* m_Device;
    };
}