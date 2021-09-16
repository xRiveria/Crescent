#include "DX11_Device.h"
#include "DX11_Utilities.h"
#include "../RHI_Context.h"
#include <iostream>

namespace Aurora
{
    DX11_Device::~DX11_Device()
    {
        m_RHI_Context->m_DeviceContext->Release();
        m_RHI_Context->m_DeviceContext = nullptr;

        m_RHI_Context->m_Device->Release();
        m_RHI_Context->m_Device = nullptr;

        m_RHI_Context->m_Annotation->Release();
        m_RHI_Context->m_Annotation = nullptr;
    }

    void DX11_Device::Initialize()
    {
        m_RHI_Context = std::make_shared<RHI_Context>();

        DX11_Utilities::GlobalContext::m_RHI_Context = m_RHI_Context.get();
        DX11_Utilities::GlobalContext::m_RHI_Device = this;
        const bool multithreadedProtection = true;






        if (multithreadedProtection) // Provides threading protection for critical sections of a multi-threaded applications.
        {
            ID3D11Multithread* multithread = nullptr;
            if (SUCCEEDED(m_RHI_Context->m_DeviceContext->QueryInterface(__uuidof(ID3D11Multithread), reinterpret_cast<void**>(&multithread))))
            {
                multithread->SetMultithreadProtected(TRUE);
                multithread->Release();
            }
            else
            {
                std::cout << "Failed to enable multi-threading protection.\n";
            }
        }

        if (m_RHI_Context->m_DebuggingEnabled)
        {
            // Queries and retrieves a pointer to the requested interface whilst calling AddRef.
            const HRESULT result = m_RHI_Context->m_DeviceContext->QueryInterface(IID_PPV_ARGS(&m_RHI_Context->m_Annotation));
            if (FAILED(result))
            {
                std::cout << DX11_Utilities::DXGI_Error_To_String(result);
                return;
            }
        }

        std::cout << "Successfully initialized DirectX11.\n";
        m_IsInitialized = true;
    }
}