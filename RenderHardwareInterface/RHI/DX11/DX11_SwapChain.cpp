#include "RHI_PCH.h"
#include "DX11_Device.h"
#include "DX11_SwapChain.h"
#include "DX11_Utilities.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Aurora
{
    DX11_SwapChain::DX11_SwapChain(uint32_t width, uint32_t height, void* windowHandle, RHI_Device* rhiDevice, uint32_t bufferCount, uint32_t flags, RHI_Format format, const char* objectName)
    {
        /// Assert that our pointers aren't empty.

        // Assert Window Handle

        // Validate Resolution
        if (!rhiDevice->IsValidResolution(width, height))
        {
            std::cout << "Requested Swapchain Resolution Is Invalid... Returning. " << width << ", " << height << "\n";
            return;
        }

        // Get factory.
        IDXGIFactory* dxgiFactory = nullptr;
        if (const RHI_GPU* gpu = rhiDevice->GetPrimaryGPU())
        {
            IDXGIAdapter* adapter = static_cast<IDXGIAdapter*>(gpu->GetInternalData());
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

        // Save Properties
        m_Width = width;
        m_Height = height;
        m_BufferCount = bufferCount;
        m_Flags = DX11_Utilities::SwapChain::ValidateFlags(flags);
        m_Format = format;
        m_RHI_Device = rhiDevice;
        m_IsWindowed = true;

        // Create SwapChain
        {
            DXGI_SWAP_CHAIN_DESC swapchainDescription = {};
            swapchainDescription.BufferCount = static_cast<UINT>(bufferCount);
            swapchainDescription.BufferDesc.Width = static_cast<UINT>(width);
            swapchainDescription.BufferDesc.Height = static_cast<UINT>(height);
            swapchainDescription.BufferDesc.Format = DX11_Utilities::ToDX11Format[format];
            swapchainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Use the surface or resource as an output render target.
            swapchainDescription.OutputWindow = glfwGetWin32Window(static_cast<GLFWwindow*>(windowHandle));
            swapchainDescription.SampleDesc.Count = 1;
            swapchainDescription.SampleDesc.Quality = 0;
            swapchainDescription.Windowed = m_IsWindowed ? TRUE : FALSE;
            swapchainDescription.SwapEffect = DX11_Utilities::SwapChain::ToDX11SwapEffect(m_Flags);
            swapchainDescription.Flags = DX11_Utilities::SwapChain::ToDX11Flags(m_Flags);

            if (!DX11_Utilities::ErrorCheck(dxgiFactory->CreateSwapChain((DX11_Utilities::GetDX11Context(m_RHI_Device->GetContextRHI()))->m_Device, &swapchainDescription, reinterpret_cast<IDXGISwapChain**>(&m_SwapChain))))
            {
                std::cout << "Failed to create SwapChain.\n";
                return;
            }
            else
            {
                std::cout << "Successfully created SwapChain.\n";
            }
        }

        m_IsInitialized = true;
    }

    DX11_SwapChain::~DX11_SwapChain()
    {

    }

    void DX11_SwapChain::Resize(uint32_t newWidth, uint32_t newHeight, const bool forceResize)
    {

    }
}