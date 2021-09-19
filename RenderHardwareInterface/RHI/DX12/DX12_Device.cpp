#include "RHI_PCH.h"
#include "DX12_Device.h"
#include "DX12_Utilities.h"

namespace Aurora
{
    DX12_Device::~DX12_Device()
    {
        GetContext()->m_Device->Release();
    }

    DX12_Context* DX12_Device::GetContext() const
    {
        return std::static_pointer_cast<DX12_Context>(m_RHI_Context).get();
    }

    void DX12_Device::Initialize()
    {
        m_RHI_Context = std::make_shared<DX12_Context>();

        // Pass our pointers into the global utility namespace.
        DX12_Utilities::GlobalContext::m_RHI_Context = m_RHI_Context.get();
        DX12_Utilities::GlobalContext::m_RHI_Device = this;

        // Detect and set primary GPU.
        DX12_Utilities::QueryAdaptersAndDisplays();

        const RHI_GPU* acquiredGPU = GetPrimaryGPU();
        if (!acquiredGPU)
        {
            std::cout << "Failed to acquire GPU.\n";
            return;
        }

        // Create Direct3D device.
        const HRESULT result = D3D12CreateDevice(static_cast<IDXGIAdapter1*>(acquiredGPU->GetInternalData()), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&GetContext()->m_Device));

        if (FAILED(result))
        {
            std::cout << "Failed to create DX12 Device.\n";
            return;
        }
        else
        {
            std::cout << "Successfully created DX12 Device.\n";
        }

        std::cout << "Successfully initialized DX12.\n";
        m_IsInitialized = true;
    }
}