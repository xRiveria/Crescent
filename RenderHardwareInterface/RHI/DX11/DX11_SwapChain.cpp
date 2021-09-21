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

        /// Assert Window Handle

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

            if (!DX11_Utilities::ErrorCheck(dxgiFactory->CreateSwapChain((DX11_Utilities::GetDX11Context())->m_Device, &swapchainDescription, reinterpret_cast<IDXGISwapChain**>(&m_SwapChain))))
            {
                std::cout << "Failed to create SwapChain.\n";
                return;
            }
            else
            {
                std::cout << "Successfully created SwapChain.\n";
            }
        }

        // Creates the render target.
        if (IDXGISwapChain* swapChain = static_cast<IDXGISwapChain*>(m_SwapChain))
        {
            ID3D11Texture2D* backBuffer = nullptr;
            // GetBuffer() utilizes a zero-based buffer index. Note that if the swapchain's swap effect is either DXGI_SWAP_EFFECT_SEQUENTIAL or DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL, only the swapchain's zero-index buffer can be read from and written to.
            // The swapchain's buffers with indexes greater than zero can only be read from. Hence, if you call IDXGIResource::GetUsage() for such buffers, they only have the DXGI_USAGE_READ_ONLY flag set.
            HRESULT result = swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)); // Accesses one of the swapchain's backbuffers and stores it in ID3D11Texture2D.
            if (FAILED(result))
            {
                std::cout << "Error getting Swapchain Buffer: " << DX11_Utilities::DXGI_Error_To_String(result) << "\n";
                return;
            }

            ID3D11RenderTargetView* renderTargetView = static_cast<ID3D11RenderTargetView*>(m_ResourceView_RenderTarget);
            result = DX11_Utilities::GetDX11Context()->m_Device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
            backBuffer->Release();

            if (FAILED(result))
            {
                std::cout << "Failed to create SwapChain RTV. " << DX11_Utilities::DXGI_Error_To_String(result) << "\n";
                return;
            }

            std::cout << "Successfully created SwapChain RTV.\n";
            m_ResourceView_RenderTarget = static_cast<void*>(renderTargetView);
        }

        m_IsInitialized = true;
    }

    DX11_SwapChain::~DX11_SwapChain()
    {
        IDXGISwapChain* swapChain = static_cast<IDXGISwapChain*>(m_SwapChain);

        // Before shutting down, set to windowed mode to avoid swapchain exception.

        swapChain->Release();
        swapChain = nullptr;
    }

    void DX11_SwapChain::Resize(uint32_t newWidth, uint32_t newHeight, const bool forceResize)
    {

    }
}