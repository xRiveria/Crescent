#pragma once
#include <wrl/client.h>
#include <string>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include "d3dx12.h"

// Note that while ComPtr is used to manage the lifetime of resources on the CPU, it has no understanding of the lifetime of resources on the GPU.
// Applications must account for the GPU lifetime of resources to avoid destroying objects that may still be referenced by the GPU. 

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace Crescent
{
    class Renderer
    {
    public:
        Renderer(uint32_t windowWidth, uint32_t windowHeight, std::wstring windowName);

        void OnInitialize();
        void OnUpdate();
        void OnRender();
        void OnDestroy();

        uint32_t GetWindowWidth() const { return m_WindowWidth; }
        uint32_t GetWindowHeight() const { return m_WindowHeight; }
        std::wstring GetWindowName() const { return m_WindowName; }

    private:
        void LoadPipeline();
        void LoadAssets();
        void PopulateCommandList();
        void WaitForGPU();
        void MoveToNextFrame();
        std::vector<UINT8> GenerateTextureData();
            
    private:
        // We overload the meaning of frameCount here to mean both the maximum number of frames that will be queued to the GPU at a time, 
        // as well as the number of backbuffers in the DXGI swapchain. For majority of apps, this works well nad is convenient. However, there may be 
        // certain cases where an application may want to queue up more frames than there are backbuffers avaliable.
        // It should be noted that excessive buffering of frames depndant on user input may result in noticable latency in your application.
        // Use fences and multiple allocators to queue up multiple frames to the GPU.
        static const uint16_t m_FrameCount = 2;
        static const unsigned int m_TextureWidth = 256;
        static const unsigned int m_TextureHeight = 256;
        static const unsigned int m_TexturePixelSize = 4; // The number of bytes to represent a pixel in the texture.

        struct Vertex
        {
            XMFLOAT3 m_Position;
            XMFLOAT2 m_UV;
        };

        struct SceneConstantBuffer
        {
            XMFLOAT4 m_Offset;
            XMFLOAT4 m_Color;
            float m_Padding[56]; // Padding som that the constant buffer is 256 byte aligned.
        };
        static_assert((sizeof(SceneConstantBuffer) % 256) == 0, "Constant Buffer size must be 256 byte aligned");

        // Constant Buffers
        ComPtr<ID3D12Resource> m_ConstantBuffer;
        SceneConstantBuffer m_ConstantBufferData;
        UINT8* m_PConstantBufferViewDataBegin;
        ComPtr<ID3D12DescriptorHeap> m_ConstantBufferViewHeap;

        // Application resoures.
        ComPtr<ID3D12Resource> m_VertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
        ComPtr<ID3D12Resource> m_Texture;

        ComPtr<ID3D12DescriptorHeap> m_RTVHeap; // A descriptor heap can be thought of as an array of descriptors, where each descriptor fully describes an object to the GPU.
        ComPtr<ID3D12DescriptorHeap> m_SRVHeap;

        // Command Lists
        ComPtr<ID3D12CommandAllocator> m_CommandAllocators[m_FrameCount]; // A command allocator manages the underlying storage for command lists and bundles.
        ComPtr<ID3D12CommandAllocator> m_BundleAllocator;
        ComPtr<ID3D12GraphicsCommandList> m_CommandList;
        ComPtr<ID3D12GraphicsCommandList> m_Bundle;
        ComPtr<ID3D12CommandQueue> m_CommandQueue;

        // Pipeline Objects
        ComPtr<ID3D12Resource> m_RenderTargets[m_FrameCount];

        CD3DX12_VIEWPORT m_Viewport;
        CD3DX12_RECT m_ScissorRect;
        ComPtr<IDXGISwapChain3> m_SwapChain;
        ComPtr<ID3D12Device> m_Device;
        ComPtr<ID3D12RootSignature> m_RootSignature;
        ComPtr<ID3D12PipelineState> m_PipelineState;
        uint32_t m_RTVDescriptorSize;

        // Synchronization Objects
        uint16_t m_FrameIndex;
        HANDLE m_FenceEvent;
        ComPtr<ID3D12Fence> m_Fence;
        UINT64 m_FenceValues[m_FrameCount];

        // Adapter information.
        bool m_UseWarpDevice;

        // Window
        uint32_t m_WindowWidth;
        uint32_t m_WindowHeight;
        uint32_t m_AspectRatio;
        std::wstring m_WindowName;
    };

    /*  Command lists and bundles both allow applications to record drawing or state-changing calls for later execution on the GPU.
    
        Beyond command lists, the API exploits functionality present in GPU hardware by adding a second level of command lists, which are referred to as 
        bundles. The purpose of bundles is to allow applications to group a small number of API commands together for later execution.

        At bundle creation time, the driver will perform as much pre-processing as it possible to make those cheap to execution to execute later. 
        Bundles are designed to be used and reused any number of times. Command lists, on the other hand, are typically executed only a single time.
        However, a command list *can* be executed multiple times (as long as the application ensures that the previous executions have completed before 
        submitting new executions).

        // ======================================================

        - ID3D12Device4::CreateCommandList1 - Create a closed command list, rather than creating a new list and immediately closing it. This avoids the 
        inefficiency of creating a list with an allocator and PSO but not using it.

        - D3D12_COMMAND_LIST_TYPE: Indicates the type of command list that is being created - can be a direct command list, a bundle, a compute command list or a copy command list.

        - ID3D12CommandAllocator - 
    */
}