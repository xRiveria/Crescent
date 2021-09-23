#pragma once
#include "../RHI_Device.h"
#include "../API_Utilities/RHI_Display.h"
#include "../RHI_Utilities.h"
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

        inline DX12_Context* GetDX12Context()
        {
            return static_cast<DX12_Context*>(GlobalContext::m_RHI_Context); // Static cast here as there is no virtual inheritence in our context.

            // return std::dynamic_pointer_cast<DX12_Context>(rhiContext).get();
        }

        constexpr bool ErrorCheck(const HRESULT result)
        {
            if (FAILED(result))
            {
                std::cout << "Error! "; ///
                return false;
            }

            return true;
        }

        // Matches our RHI_Format enum in RHI_Utilities. As enums are essentially integer literals, they can be used to index these array lists.
        static const DXGI_FORMAT ToDX12Format[] =
        {
            DXGI_FORMAT_UNKNOWN,
            DXGI_FORMAT_R8G8B8A8_UNORM,

            // RGB
            DXGI_FORMAT_R11G11B10_FLOAT,

            // Depth
            DXGI_FORMAT_D32_FLOAT,
            DXGI_FORMAT_D32_FLOAT_S8X24_UINT
        };

        static const D3D12_DESCRIPTOR_HEAP_TYPE ToDX12DescriptorType[] =
        {
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            D3D12_DESCRIPTOR_HEAP_TYPE_DSV
        };

        inline void QueryAdaptersAndDisplays()
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
            for (IDXGIAdapter1* displayAdapter : adapters)
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

            /*
            // Get display modes. Querying display modes ensures that our application can properly choose a valid full-screen mode.
            const auto GetDisplayModes = [](IDXGIAdapter1* adapter, RHI_Format queryingFormat)
            {
                bool result = false;

                // Enumerate the primary adapter output - our monitor.
                IDXGIOutput* adapterOutput = nullptr;
                // Reference counted.
                if (ErrorCheck(adapter->EnumOutputs(0, &adapterOutput))) // We select 0 here as EnumOutput() first returns the output on which the primary desktop is displayed. This is usually index 0, followed by other outputs.
                {
                    // Get supported display mode count.
                    UINT displayModeCount = 0;
                    // Interlaced Mode essentially works by switching the odd and even lines of a monitor on and off in rapid succession and is used to obtain a 1080p resolution: https://en.wikipedia.org/wiki/Interlaced_video
                    if (ErrorCheck(adapterOutput->GetDisplayModeList(ToDX12Format[queryingFormat], DXGI_ENUM_MODES_INTERLACED, &displayModeCount, nullptr))) // Get count, querying based on the format we wish to support.
                    {
                        // Actually retrieve the display modes themselves.
                        std::vector<DXGI_MODE_DESC> displayModes;
                        displayModes.resize(displayModeCount);
                        if (ErrorCheck(adapterOutput->GetDisplayModeList(ToDX12Format[queryingFormat], DXGI_ENUM_MODES_INTERLACED, &displayModeCount, &displayModes[0])))
                        {
                            // Save all display modes.
                            for (const DXGI_MODE_DESC& modeDescription : displayModes)
                            {
                                bool updateFPSLimitToHighestHertz = true;
                                RHI_Display::RegisterDisplayMode(RHI_DisplayMode(modeDescription.Width, modeDescription.Height, modeDescription.RefreshRate.Numerator, modeDescription.RefreshRate.Denominator), updateFPSLimitToHighestHertz);
                                result = true;
                            }
                        }
                    }

                    adapterOutput->Release();
                }

                return result;
            };

            // Get display modes and set primary adaper.
            for (uint32_t deviceIndex = 0; deviceIndex < GlobalContext::m_RHI_Device->GetGPUs().size(); deviceIndex++)
            {
                const RHI_GPU& gpu = GlobalContext::m_RHI_Device->GetGPUs()[deviceIndex];
                IDXGIAdapter1* adapter = static_cast<IDXGIAdapter1*>(gpu.GetInternalData());

                // Adapters are ordered by memory (descending order). We will pick the one with the most memory, hence stopping on the first success at index 0.
                const RHI_Format format = RHI_Format::RHI_Format_R8G8B8A8_Unorm;
                if (GetDisplayModes(adapter, format))
                {
                    GlobalContext::m_RHI_Device->SetPrimaryGPU(deviceIndex);
                    return;
                }
                else
                {
                    std::cout << "Failed to get display modes for " << gpu.GetGPUName();
                }
            }
            */

            // If we failed to get any display modes but have at least one adapter, use it.
            if (GlobalContext::m_RHI_Device->GetGPUs().size() != 0)
            {
                std::cout << "Falling back to 1st default GPU adapter avaliable.\n";
                GlobalContext::m_RHI_Device->SetPrimaryGPU(0);
            }
        }

        namespace Heap
        {
            inline D3D12_DESCRIPTOR_HEAP_FLAGS ValidateFlags(uint32_t flags)
            {
                if (flags & RHI_Descriptor_Heap_Flag::RHI_Descriptor_Heap_Flag_None)
                {
                    return D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
                }

                if (flags & RHI_Descriptor_Heap_Flag::RHI_Descriptor_Heap_Flag_ShaderVisible)
                {
                    return D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
                }

                std::cout << "Descriptor Heap Flag not found. Returning default.\n";
                return D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            }

            inline bool Create(void*& heap, uint32_t descriptorCount, RHI_Descriptor_Heap_Flag flag, RHI_Descriptor_Heap_Type heapType)
            {
                D3D12_DESCRIPTOR_HEAP_DESC heapDescription = {};
                heapDescription.Type = ToDX12DescriptorType[heapType];
                heapDescription.NumDescriptors = static_cast<UINT>(descriptorCount);
                heapDescription.Flags = ValidateFlags(flag);

                ID3D12DescriptorHeap* descriptorHeap;

                if (!ErrorCheck(GetDX12Context()->m_Device->CreateDescriptorHeap(&heapDescription, IID_PPV_ARGS(&descriptorHeap))))
                {
                    std::cout << "Failed to create Descriptor Heap.\n";
                    return false;
                }

                heap = static_cast<void*>(descriptorHeap);
                std::cout << "Successfully created Descriptor Heap.\n";
                return true;
            }
        }

        namespace CommandQueue
        {
            inline bool Create(void*& commandQueue, ID3D12Device* device)
            {
                D3D12_COMMAND_QUEUE_DESC commandQueueDescription = {};
                commandQueueDescription.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_NONE; // Signifies a default command queue.
                commandQueueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Specifies a command buffer that the GPU can execute. A direct command list doesn't inherit any GPU state.
                
                ID3D12CommandQueue* dx12CommandQueue;
                if (!ErrorCheck(device->CreateCommandQueue(&commandQueueDescription, IID_PPV_ARGS(&dx12CommandQueue))))
                {
                    std::cout << "Failed to create DX12 Command Queue.\n";
                    return false;
                }
                else
                {
                    commandQueue = static_cast<void*>(dx12CommandQueue);
                    std::cout << "Successfully created DX12 Command Queue.\n";
                    return true;
                }
            }

            inline void Destroy(void*& commandQueue)
            {
                ID3D12CommandQueue* dx12CommandQueue = static_cast<ID3D12CommandQueue*>(commandQueue);
                dx12CommandQueue->Release();
                commandQueue = nullptr;
            }
        }

        namespace SwapChain
        {
            inline DXGI_SWAP_EFFECT ToDX12SwapEffect(uint32_t flags)
            {
                if (flags & RHI_Swap_Discard) { return DXGI_SWAP_EFFECT_DISCARD; }
                if (flags & RHI_Swap_Sequential) { return DXGI_SWAP_EFFECT_SEQUENTIAL; }
                if (flags & RHI_Swap_Flip_Discard) { return DXGI_SWAP_EFFECT_FLIP_DISCARD; }
                if (flags & RHI_Swap_Flip_Sequential) { return DXGI_SWAP_EFFECT_SEQUENTIAL; }

                std::cout << "Failed to determine the requested swap effect, opting for DXGI_SWAP_EFFECT_DISCARD.\n";
                return DXGI_SWAP_EFFECT_DISCARD;
            }
        }
    }
}