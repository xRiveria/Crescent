#include "Renderer.h"
#include "RenderUtilities.h"
#include "../Window.h"
#include <d3dcompiler.h>

namespace Crescent
{
    // Helper function for resolving the full path of assets.
    std::wstring GetAssetFullPath(LPCWSTR assetName)
    {
        return assetName;
    }

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

        /*
            Create descriptor heaps. We will create a render target view (RTV) descriptor heap.
            A descriptor heap is a collection off contiguous allocations of descriptors, one allocation for every descriptor.

            The primary purpose of a descriptor heap is to encompass the bulk of memory allocation required for storing the descriptor specifications
            of object types that shaders reference for as large of a window of rendering as possible (ideally an entire frame of rendering or more). If 
            an application is switching which textures the pipeline sees rapidly from the API, there has to be space in the descriptor heap to define 
            descriptor tables on the fly for every set of state needed. 

            All heaps are visible to the CPU..
        */
        {
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDescription = {};
            rtvHeapDescription.NumDescriptors = m_FrameCount;
            rtvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ThrowIfFailed(m_Device->CreateDescriptorHeap(&rtvHeapDescription, IID_PPV_ARGS(&m_RTVHeap)));

            // Describe and create a shader resource view (SRV) heap for the texture.
            D3D12_DESCRIPTOR_HEAP_DESC srvHeapDescription = {};
            srvHeapDescription.NumDescriptors = 1;
            srvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV; // The descriptor heap for the combination of constant buffer, shader resource and unordered-access views     
            // Can be set on a descriptor heap to indicate that it is bound on a command list for reference by shaders. Descriptor heaps created without this flag allow
            // applications the option to stage descriptors in CPU memory before copying them to a shader visible descriptor heap as a convenience. However, it is also fine for applications 
            // to directly create descriptors into shader visible descriptor heaps with no requirement to stage anything on the CPU.
            // The D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE flag only applies to CBV/SRV/UAV descriptor heaps and sampler descriptor heaps. It does not apply to other descriptor 
            // heap types since shaders do not directly reference the other types. Attempting to create an RTV/DSV heap with that flag results in a debug layer error.
            srvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            ThrowIfFailed(m_Device->CreateDescriptorHeap(&srvHeapDescription, IID_PPV_ARGS(&m_SRVHeap)));

            m_RTVDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        }

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

    void Renderer::LoadAssets()
    {
        // Create an empty root signature.
        D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {}; // Indicates root signature version support. On input, specifies the highest version D3D_ROOT_SIGNATURE_VERSION to check for. On output, specifies the highest version, up to the input version specified actually avaliable.

        // This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1; // Root Signature 1.1 enables applications to indicate to drivers when descriptors in a descriptor heap won't change or the data descriptors point to won't chnange. This allows optimizations that might be possible knowing that a descriptor or the memory it points to is static for some period of time. 

        if (FAILED(m_Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
        {
            featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
        }

        CD3DX12_DESCRIPTOR_RANGE1 ranges[1];
        ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

        CD3DX12_ROOT_PARAMETER1 rootParameters[1];
        rootParameters[0].InitAsDescriptorTable(1, &ranges[0], D3D12_SHADER_VISIBILITY_PIXEL);

        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER; // Texture coordinates outside the range [0.0, 1.0] are set to the texture color specified in the sampler description.
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = 0;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
        rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 1, &sampler, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT); // We are opting in to using the Input Assembler (requiring an input layout that defines a set of vertex buffer bindings). Omitting this flag can result in one root argument space being saved on some hardware. We can omit this flag if the input assembler is not required, though the optimization is minor.

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription, featureData.HighestVersion, &signature, &error));
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
                { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
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

        // Create the vertex buffer.
        {
            // Define the geometry for a triangle.
            Vertex triangleVertices[] =
            {
                { {  0.0f,   0.25f * m_AspectRatio, 0.0f }, { 0.5f, 0.0f}  },
                { {  0.25f, -0.25f * m_AspectRatio, 0.0f }, { 1.0f, 1.0f}  },
                { { -0.25f, -0.25f * m_AspectRatio, 0.0f }, { 0.0f, 1.0f}  }
            };

            const UINT vertexBufferSize = sizeof(triangleVertices);

            // Note: Using upload heaps to transfer static data like vertex buffers is not recommended. Every time the GPU needs it, the upload heap 
            // will be marshalled over. Please read up on default heap usage. An upload heap is used here for code simplicity and ecause there are very few vertices to actually transfer.
         
            // CreateCommitedResource creates both a resource and an implicit heap, such that the heap is big enough to contain the entire resource, and the resource is mapped to the heap.
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

        /*
            ComPtrs are CPU objects but this resource needs to stay in scope until the command list that references it has finished executing on the GPU.
            We will flush the GPU at the end of this method to ensure that resource is not prematurely destroyed.
        */

        ComPtr<ID3D12Resource> textureUploadHeap;

        // Create the texture.
        {
            // Describe and create a Texture2D.
            D3D12_RESOURCE_DESC textureDescription = {};
            textureDescription.MipLevels = 1;
            textureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            textureDescription.Width = m_TextureWidth;
            textureDescription.Height = m_TextureHeight;
            textureDescription.Flags = D3D12_RESOURCE_FLAG_NONE;
            textureDescription.DepthOrArraySize = 1;
            textureDescription.SampleDesc.Count = 1;
            textureDescription.SampleDesc.Quality = 0;
            textureDescription.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

            ThrowIfFailed(m_Device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &textureDescription, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_Texture)));

            const UINT64 uploadBufferSize = GetRequiredIntermediateSize(m_Texture.Get(), 0, 1);

            // Create the GPU upload buffer.
            ThrowIfFailed(m_Device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
                D3D12_HEAP_FLAG_NONE,
                &CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize),
                D3D12_RESOURCE_STATE_GENERIC_READ,
                nullptr,
                IID_PPV_ARGS(&textureUploadHeap)));

            // Copy data to the intermidate upload heap and then schedule a copy from the upload heap to the Texture2D.
            std::vector<UINT8> texture = GenerateTextureData();

            D3D12_SUBRESOURCE_DATA textureData = {};
            textureData.pData = &texture[0];
            textureData.RowPitch = m_TextureWidth * m_TexturePixelSize;
            textureData.SlicePitch = textureData.RowPitch * m_TextureHeight;

            UpdateSubresources(m_CommandList.Get(), m_Texture.Get(), textureUploadHeap.Get(), 0, 0, 1, &textureData);
            m_CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));

            // Describe and create an SRV for this texture.
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            // Shader4ComponentMapping specifies what values from memory should be return when the texture is accessed in shader via this SRV. For example, it can route component 1 (Green) from memory, or the constant 0 into component 2 (.b) of the value given to the shader.
            srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srvDesc.Format = textureDescription.Format;
            srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            // GetCPUDescriptorHandleForHeapStart() -> Gets the CPU descriptor handle that represents the start of the heap.
            m_Device->CreateShaderResourceView(m_Texture.Get(), &srvDesc, m_SRVHeap->GetCPUDescriptorHandleForHeapStart());
        }

        // Command lists are created in the recording state, but there is nothing to record yet. The main loop expects it to be closed, so close it or now.
        ThrowIfFailed(m_CommandList->Close());
        ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
        m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

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

        /*
            A root signature is configured by the application and links command lists to the resources the shaders require. The graphics command list has 
            both a graphics and compute root signature. A compute command list will simply have one compute root signature. These root signatures are 
            independant of each other.

            A root signature is similar to an API function signature - it determines the types of data the shaders should expect, but does not define the 
            actual memory or data. A root parameter is one entry in the root signature. The actual values of root parameters set and changed at runtime are 
            called root arguments. Changing the root arguments changes the data that the shaders read.

            The root signature can contain 3 types of parameters: root constants (constants inlined in the root arguments), root descriptors (descriptors inlined in the root arguments), 
            and descriptor tables (pointers to a range of descriptors in the descriptor heap). The root constanrts are inline 32-bit values that show up in the shader as a constant buffer.

            The inlined root descriptors should contain descriptors that are accessed most often, though is limited to CBVs, and row or structed UAV or SRV buffers. 
            A more complex type, such as a 2D texture SRV, cannot be used as a root descriptor. Root descriptors do not include a size limit, so there can be no 
            out of bounds checking, unlike descriptors in descriptor heaps, which do include a size.

            Descriptor table entries within root signatures contain the descriptor, HLSL shader bind name and visiblity flag. On some hardware, there can be a performance 
            gin from only making descriptors visible to shader stages that require them.

            An application can make its own tradeoff between how many descriptor tables it wants versus inline descriptors (which takes more space but removes 
            an indirection) versus inline constants (which have no indirection) they want in the root signature. Applications should use the root signature 
            as sparingly as possible, relying on application controlled memory such as heaps and descriptor heaps pointing into them to represent bulk data. 
        */
        // Set the necessary states.
        m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get()); 
        ID3D12DescriptorHeap* ppHeaps[] = { m_SRVHeap.Get() };
        m_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

        m_CommandList->SetGraphicsRootDescriptorTable(0, m_SRVHeap->GetGPUDescriptorHandleForHeapStart());
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

    // Generate a simple black and white checkerboard texture.
    std::vector<UINT8> Renderer::GenerateTextureData()
    {
        const UINT rowPitch = m_TextureWidth * m_TexturePixelSize;
        const UINT cellPitch = rowPitch >> 3; // The width of a cell in the checkerboard texture.
        const UINT cellHeight = m_TextureWidth >> 3;
        const UINT textureSize = rowPitch * m_TextureHeight;

        std::vector<UINT8> data(textureSize);
        UINT8* pData = &data[0];

        for (UINT n = 0; n < textureSize; n += m_TexturePixelSize)
        {
            UINT x = n % rowPitch;
            UINT y = n / rowPitch;
            UINT i = x / cellPitch;
            UINT j = y / cellHeight;

            if (i % 2 == j % 2)
            {
                pData[n] = 0x00;        // R
                pData[n + 1] = 0x00;    // G
                pData[n + 2] = 0x00;    // B
                pData[n + 3] = 0xff;    // A
            }
            else
            {
                pData[n] = 0xff;        // R
                pData[n + 1] = 0xff;    // G
                pData[n + 2] = 0xff;    // B
                pData[n + 3] = 0xff;    // A
            }
        }

        return data;
    }
}