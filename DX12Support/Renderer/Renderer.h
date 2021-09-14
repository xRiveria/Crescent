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
        void WaitForPreviousFrame();
            
    private:
        static const uint16_t m_FrameCount = 2;

        struct Vertex
        {
            XMFLOAT3 m_Position;
            XMFLOAT4 m_Color;
        };

        // Application resoures.
        ComPtr<ID3D12Resource> m_VertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;

        // Pipeline Objects
        CD3DX12_VIEWPORT m_Viewport;
        CD3DX12_RECT m_ScissorRect;
        ComPtr<IDXGISwapChain3> m_SwapChain;
        ComPtr<ID3D12Device> m_Device;
        ComPtr<ID3D12Resource> m_RenderTargets[m_FrameCount];
        ComPtr<ID3D12CommandAllocator> m_CommandAllocator; // A command allocator manages the underlying storage for command lists and bundles.
        ComPtr<ID3D12CommandQueue> m_CommandQueue;
        ComPtr<ID3D12RootSignature> m_RootSignature;
        ComPtr<ID3D12DescriptorHeap> m_RTVHeap; // A descriptor heap can be thought of as an array of descriptors, where each descriptor fully describes an object to the GPU.
        ComPtr<ID3D12PipelineState> m_PipelineState;
        ComPtr<ID3D12GraphicsCommandList> m_CommandList;
        uint32_t m_RTVDescriptorSize;

        // Synchronization Objects
        uint16_t m_FrameIndex;
        HANDLE m_FenceEvent;
        ComPtr<ID3D12Fence> m_Fence;
        uint64_t m_FenceValue;

        // Adapter information.
        bool m_UseWarpDevice;

        // Window
        uint32_t m_WindowWidth;
        uint32_t m_WindowHeight;
        uint32_t m_AspectRatio;
        std::wstring m_WindowName;
    };
}