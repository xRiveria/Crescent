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

        m_Width = width;
        m_Height = height;
        m_Format = format;
        m_BufferCount = bufferCount;
        m_RHI_Device = rhiDevice;

        {
            // Describe and create the swapchain.
            DXGI_SWAP_CHAIN_DESC1 swapchainDescription = {};

            swapchainDescription.BufferCount = bufferCount;
            swapchainDescription.Width = width;
            swapchainDescription.Height = height;
            swapchainDescription.Format = DX12_Utilities::ToDX12Format[format];
            swapchainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Uses the surface as an output render target.
            swapchainDescription.SwapEffect = DX12_Utilities::SwapChain::ToDX12SwapEffect(flags); // Contents of the backbuffer is discarded after calling present. 
            swapchainDescription.SampleDesc.Count = 1;

            if (!DX12_Utilities::ErrorCheck(dxgiFactory->CreateSwapChainForHwnd(
                static_cast<ID3D12CommandQueue*>(m_RHI_Device->m_CommandPool), // The swapchain needs the queue so that it can force a flush on it.
                glfwGetWin32Window(static_cast<GLFWwindow*>(windowHandle)),
                &swapchainDescription,
                nullptr, nullptr,
                reinterpret_cast<IDXGISwapChain1**>(&m_SwapChain)
            )))
            {
                std::cout << "Failed to create DX12 SwapChain.\n";
                return;
            }

            std::cout << "Successfully created DX12 SwapChain.\n";
        }

        // Heap with m_BufferCount descriptors for our RTV.
        DX12_Utilities::Heap::Create(m_ResourceHeap_RenderTargetView, m_BufferCount, RHI_Descriptor_Heap_Flag::RHI_Descriptor_Heap_Flag_None, RHI_Descriptor_Heap_Type::RHI_Descriptor_Heap_Type_RTV);
        // Essentially retrieves the size to increment for each RTV handle in our heap. This value is used to increment a handle into a descriptor array by the correct amount.
        m_RTV_DescriptorSize = DX12_Utilities::GetDX12Context()->m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV); 

        // Gets the CPU descriptor handle that represents the start of the heap.
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle(static_cast<ID3D12DescriptorHeap*>(m_ResourceHeap_RenderTargetView)->GetCPUDescriptorHandleForHeapStart());

        // Create an RTV for each frame.
        for (UINT i = 0; i < m_BufferCount; i++)
        {
            ID3D12Resource* buffer;
            if (DX12_Utilities::ErrorCheck(static_cast<IDXGISwapChain1*>(m_SwapChain)->GetBuffer(i, IID_PPV_ARGS(&buffer))))
            {
                DX12_Utilities::GetDX12Context()->m_Device->CreateRenderTargetView(buffer, nullptr, rtvCPUHandle);
                rtvCPUHandle.Offset(1, m_RTV_DescriptorSize); // Offset by 1 for the size of an RTV descriptor.
                m_ResourceView_RenderTarget[i] = static_cast<void*>(buffer);

                std::cout << "Successfully created RTV for Swapchain: " << i << "\n";
            }
        }

        m_IsInitialized = true;
    }

    DX12_SwapChain::~DX12_SwapChain()
    {

    }

    void DX12_SwapChain::Resize(uint32_t newWidth, uint32_t newHeight, const bool forceResize)
    {
    }
}