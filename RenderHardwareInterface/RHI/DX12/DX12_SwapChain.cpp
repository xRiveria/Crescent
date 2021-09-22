#include "RHI_PCH.h"
#include "DX12_Device.h"
#include "DX12_SwapChain.h"
#include "DX12_Utilities.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Aurora
{
    DX12_SwapChain::DX12_SwapChain(uint32_t width, uint32_t height, void* windowHandle, RHI_Device* rhiDevice, uint32_t bufferCount, uint32_t flags, RHI_Format format, const char* objectName)
    {
        /// Assert that our pointers aren't empty.

        /// Assert Window Handle

        // Validate Resolution
        if (!rhiDevice->IsValidResolution(width, height))
        {
            std::cout << "Requested Swapchain Resolution Is Invalid... Returning. " << width << ", " << height << "\n";
            return;
        }

        // Get factory.
        IDXGIFactory4* dxgiFactory = nullptr;
        if (const RHI_GPU* gpu = rhiDevice->GetPrimaryGPU())
        {
            IDXGIAdapter1* adapter = static_cast<IDXGIAdapter1*>(gpu->GetInternalData());
            if (adapter->GetParent(IID_PPV_ARGS(&dxgiFactory)) != S_OK) // Parent factories of adapters can be retrieved with GetParent.
            {
                std::cout << "Failed to retrieve adapter's factory.\n";
                return;
            }
        }
        else
        {
            std::cout << "Invalid GPU adapter.\n";
            return;
        }

        ///

        m_IsInitialized = true;
    }

    DX12_SwapChain::~DX12_SwapChain()
    {
    }

    void DX12_SwapChain::Resize(uint32_t newWidth, uint32_t newHeight, const bool forceResize)
    {
    }
}