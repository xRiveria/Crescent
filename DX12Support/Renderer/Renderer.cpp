#include "Renderer.h"
#include "RenderUtilities.h"
#include "../Window.h"
#include <d3dcompiler.h>

namespace Crescent
{
    /*
        COM uses GUID for two primary purposes:

        - To uniquely identify a particular COM interface. A GUID that is assigned to identify a COM interface is called an interface identifier (IID),
        and you use IID when you request a particular interface from an instance of a component (an object). An interface's IID will be the same regardless 
        of the component that implements the interface.

        For conviennce, DirectX refers to components and interfaces by their descriptive names (ID3D12Device for example), rather than by their GUIDs.
        Within the context of DirectX, there is no ambiguity. Its technically possible for a third-party to author an inrterface with the descriptive name 
        Id3D12Device (it would need to have a different IID to be valid). Thus, the only unambigitous way to refer to a particular object or interface 
        is by its GUID.
        
        IID_PPV_ARGS is used to retrieve an interface pointer, supplying the IID value of the requested interface automatically.
        We pass in an address of an interface pointer to IID_PPV_ARGS whose type T is used to determine the type of object being requested.
        The macro returns the interface pointer through this parameter.
*/

    Renderer::Renderer(uint32_t windowWidth, uint32_t windowHeight, std::wstring windowName) : 
        m_WindowWidth(windowWidth), m_WindowHeight(windowHeight), m_WindowName(std::move(windowName)), m_FrameIndex(0),
        m_Viewport(0.0f, 0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight)),
        m_ScissorRect(0, 0, static_cast<LONG>(windowWidth), static_cast<LONG>(windowHeight))
    {
        m_UseWarpDevice = false;
        m_RTVDescriptorSize = 0;
        m_AspectRatio = static_cast<float>(m_WindowWidth) / static_cast<float>(m_WindowHeight);
    }

    void Renderer::OnInitialize()
    {
        LoadPipeline();
        LoadAssets();
    }

    // Update frame-based values.
    void Renderer::OnUpdate()
    {
    }

    // Render the scene.
    void Renderer::OnRender()
    {
        // Record all the commands we need to render the scene into command lists.
        PopulateCommandList();

        // Execute the command list.
        ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
        m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

        // Present the frame.
        ThrowIfFailed(m_SwapChain->Present(1, 0));

        WaitForPreviousFrame();
    }
    
    void Renderer::OnDestroy()
    {
        // Ensure that the GPU is no longer referencing resources that are about to be cleaned up by the destructor.
        WaitForPreviousFrame();
        CloseHandle(m_FenceEvent);
    }
    
    // Load the rendering pipeline dependencies.
    void Renderer::LoadPipeline()
    {
        uint16_t dxgiFactoryFlags = 0;
#if defined(_DEBUG)
        // Enable the debug layer. Note that enabling the debug layer after device creation will invalidate the active device.
        ComPtr<ID3D12Debug> debugContext;

        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugContext)))) 
        {
            debugContext->EnableDebugLayer();
            
            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
#endif

        ComPtr<IDXGIFactory4> factory;
        ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory))); 

        if (m_UseWarpDevice)
        {

        }
        else
        {
            ComPtr<IDXGIAdapter1> hardwareAdapter;
            GetHardwareAdapter(factory.Get(), &hardwareAdapter);

            ThrowIfFailed(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device)));
        }

        // Describe and create the command queue.
        D3D12_COMMAND_QUEUE_DESC queueDescription = {};
        queueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Specifies a command buffer that the GPU can execute. A direct command list doesn't inherit any GPU state.
    
        ThrowIfFailed(m_Device->CreateCommandQueue(&queueDescription, IID_PPV_ARGS(&m_CommandQueue)));

        // Describe and create the swapchain.
        DXGI_SWAP_CHAIN_DESC1 swapchainDescription = {};
        swapchainDescription.BufferCount = m_FrameCount;
        swapchainDescription.Width = m_WindowWidth;
        swapchainDescription.Height = m_WindowHeight;
        swapchainDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapchainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // Uses the surface as an output render target.
        swapchainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Contents of the backbuffer is discarded after calling present. 
        swapchainDescription.SampleDesc.Count = 1;

        ComPtr<IDXGISwapChain1> swapChain;
        ThrowIfFailed(factory->CreateSwapChainForHwnd(
            m_CommandQueue.Get(),    // The swapchain needs the queue so that it can force a flush on it.
            Window::GetHWND(),
            &swapchainDescription,
            nullptr, nullptr,
            &swapChain
        ));

        // We do not support full screen transitions at the moment.
        ThrowIfFailed(factory->MakeWindowAssociation(Window::GetHWND(), DXGI_MWA_NO_ALT_ENTER));
        ThrowIfFailed(swapChain.As(&m_SwapChain));
        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();

        // Create descriptor heaps. We will create a render target view (RTV) descriptor heap.
        D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDescription = {};
        rtvHeapDescription.NumDescriptors = m_FrameCount;
        rtvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        ThrowIfFailed(m_Device->CreateDescriptorHeap(&rtvHeapDescription, IID_PPV_ARGS(&m_RTVHeap)));

        m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

        // Create frame resources.
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT i = 0; i < m_FrameCount; i++)
        {
            ThrowIfFailed(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTargets[i])));
            m_Device->CreateRenderTargetView(m_RenderTargets[i].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, m_RTVDescriptorSize);
        }

        ThrowIfFailed(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));
    }

    // Helper function for resolving the full path of assets.
    std::wstring GetAssetFullPath(LPCWSTR assetName)
    {
        return assetName;
    }

    void Renderer::LoadAssets()
    {
        // Create an empty root signature.
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDescription;
        rootSignatureDescription.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDescription, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));

        // Creates the pipeline state, which includes compiling and loading shaders.
        {
            ComPtr<ID3DBlob> vertexShader;
            ComPtr<ID3DBlob> pixelShader;

#if defined (_DEBUG)
            // Enable better shader debugging with the graphics debugging tools.
            UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
            UINT compileFlags = 0;
#endif

            ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"Shaders.hlsl").c_str(), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
            ThrowIfFailed(D3DCompileFromFile(GetAssetFullPath(L"Shaders.hlsl").c_str(), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));
        
            // Define the vertex input layout.
            D3D12_INPUT_ELEMENT_DESC inputElementDescriptions[] =
            {
                { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
                { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
            };

            // Describe and create the graphics state object (PSO).
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDescription = {};
            psoDescription.InputLayout = { inputElementDescriptions, _countof(inputElementDescriptions) };
            psoDescription.pRootSignature = m_RootSignature.Get();
            psoDescription.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
            psoDescription.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
            psoDescription.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            psoDescription.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            psoDescription.DepthStencilState.DepthEnable = FALSE;
            psoDescription.DepthStencilState.StencilEnable = FALSE;
            psoDescription.SampleMask = UINT_MAX;
            psoDescription.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            psoDescription.NumRenderTargets = 1;
            psoDescription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
            psoDescription.SampleDesc.Count = 1;
            
            ThrowIfFailed(m_Device->CreateGraphicsPipelineState(&psoDescription, IID_PPV_ARGS(&m_PipelineState)));
        }

        // Create the command list.
        ThrowIfFailed(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList)));

        // Command lists are created in the recording state, but there is nothing to record yet. The main loop expects it to be closed, so close it or now.
        ThrowIfFailed(m_CommandList->Close());

        // Create the vertex buffer.
        {
            // Define the geometry for a triangle.
            Vertex triangleVertices[] =
            {
                { {  0.0f,   0.25f * m_AspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
                { {  0.25f, -0.25f * m_AspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
                { { -0.25f, -0.25f * m_AspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
            };

            const UINT vertexBufferSize = sizeof(triangleVertices);

            // Note: Using upload heaps to transfer static data like vertex buffers is not recommended. Every time the GPU needs it, the upload heap 
            // will be marshalled over. Please read up on default heap usage. An upload heap is used here for code simplicity and ecause there are very few vertices to actually transfer.
            ThrowIfFailed(m_Device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&m_VertexBuffer)));


            // Copy the triangle data to the vertex buffer.
            UINT8* pVertexDataBegin;
            CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the GPU.
            ThrowIfFailed(m_VertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
            memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
            m_VertexBuffer->Unmap(0, nullptr);

            // Initialize the vertex buffer view.
            m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
            m_VertexBufferView.StrideInBytes = sizeof(Vertex);
            m_VertexBufferView.SizeInBytes = vertexBufferSize;
        }

        // Create synchronization objects and wait until assets have been uploaded to the GPU.
        ThrowIfFailed(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
        m_FenceValue = 1;

        // Create an event handle to use for frame synchronization.
        m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_FenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute. We are reusing the same command list in our main loop for now, we just want to wait for setup to complete before continuing.
        WaitForPreviousFrame();
    }

    void Renderer::PopulateCommandList()
    {
        // Command list allocators can only be reset when the associated command lists have finished execution on the GPU. Thus, we should use fences to determine GPU execution progress.
        ThrowIfFailed(m_CommandAllocator->Reset());

        // However, when ExecuteCommandList() is called on a particular command list, that command listc an then be reset at any time and must be before re-recording.
        ThrowIfFailed(m_CommandList->Reset(m_CommandAllocator.Get(), m_PipelineState.Get()));

        // Set the necessary states.
        m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get()); // Defines what resources are bound to the graphics pipeline. It links command lists to the resources the shaders require. There is one graphics and one compute root signature per application.
        m_CommandList->RSSetViewports(1, &m_Viewport);
        m_CommandList->RSSetScissorRects(1, &m_ScissorRect);

        // Indicate that the back buffer will be used as a render target.
        m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RTVHeap->GetCPUDescriptorHandleForHeapStart(), m_FrameIndex, m_RTVDescriptorSize);
        m_CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

        // Record commands.
        const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
        m_CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
        m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView);
        m_CommandList->DrawInstanced(3, 1, 0, 0);

        // Indicate that the backbuffer wikll now be used to present.
        m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[m_FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

        ThrowIfFailed(m_CommandList->Close());
    }

    void Renderer::WaitForPreviousFrame()
    {
        // Waiting for the frame to complete before continuing is not best practice. This is for simplicity's state.
        // The D3D12HelloFrameBuffering sample illustrates how to use fences for efficient resource usage and to maximize GPU utilization.

        // Signal and increment fence value.
        const UINT64 fence = m_FenceValue;
        ThrowIfFailed(m_CommandQueue->Signal(m_Fence.Get(), fence));
        m_FenceValue++;

        // Waitr until the previous frame is finished.
        if (m_Fence->GetCompletedValue() < fence)
        {
            ThrowIfFailed(m_Fence->SetEventOnCompletion(fence, m_FenceEvent));
            WaitForSingleObject(m_FenceEvent, INFINITE);
        }

        m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
    }
}