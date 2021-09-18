#pragma once
#include "../RHI_Device.h"
#include "DX12_Context.h"
#include <iostream>

namespace Aurora
{
    namespace DX12_Utilities
    {
        namespace GlobalContext
        {
            static inline RHI_Device* m_RHI_Device = nullptr;
            static inline RHI_Context* m_RHI_Context = nullptr;
        };

        inline void DetectGraphicsAdapters()
        {
            uint16_t dxgiFactoryFlags = 0;

            if (GlobalContext::m_RHI_Context->m_DebuggingEnabled)
            {
                // Enable our debug layer.
                ComPtr<ID3D12Debug> debugContext;
                if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugContext))))
                {
                    debugContext->EnableDebugLayer();
                    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
                }
            }

            IDXGIFactory4* factory;
            if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory))))
            {
                std::cout << "Failed to create DXGI Factory.";
            }

            const auto GetAvaliableAdapters = [](IDXGIFactory4* factory)
            {
                uint32_t i = 0;         // Enumerating Index
                IDXGIAdapter1* adapter; // Enumerating adapter.

                std::vector<IDXGIAdapter1*> adapters;
                while (factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND)
                {
                    adapters.emplace_back(std::move(adapter));
                    ++i;
                }

                return adapters;
            };

            std::vector<IDXGIAdapter1*> adapters = GetAvaliableAdapters(factory);
            factory->Release();
            factory = nullptr;

            // Save all avaliable adapters.
            DXGI_ADAPTER_DESC adapterDescription;
            for (IDXGIAdapter* displayAdapter : adapters)
            {
                if (FAILED(displayAdapter->GetDesc(&adapterDescription)))
                {
                    std::cout << "Failed to retrieve adapter descriptipn.\n";
                    continue;
                }

                // Save adapter properties.
                char adapterName[128];
                char defaultChar = ' ';
                WideCharToMultiByte(CP_ACP, 0, adapterDescription.Description, -1, adapterName, 128, &defaultChar, nullptr);
                
                GlobalContext::m_RHI_Device->RegisterGPU(RHI_GPU(
                    adapterDescription.VendorId,
                    RHI_GPU_Type::Unknown,
                    &adapterName[0],
                    static_cast<uint64_t>(adapterDescription.DedicatedVideoMemory),
                    static_cast<void*>(displayAdapter)));
            }




            // If we failed to get any display modes but have at least one adapter, use it.
            if (GlobalContext::m_RHI_Device->GetGPUs().size() != 0)
            {
                std::cout << "Falling back to 1st default GPU adapter avaliable.\n";
                GlobalContext::m_RHI_Device->SetPrimaryGPU(0);
            }
        }
    }
}