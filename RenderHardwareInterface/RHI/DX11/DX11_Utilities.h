#pragma once
#include "../RHI_Device.h"
#include "../RHI_Utilities.h"
#include "../API_Utilities/RHI_Display.h"
#include "DX11_Context.h"
#include <vector>
#include <iostream>

namespace Aurora
{
    namespace DX11_Utilities
    {
        struct GlobalContext
        {
            static inline RHI_Device* m_RHI_Device = nullptr;
            static inline RHI_Context* m_RHI_Context = nullptr;
        };

        inline const char* DXGI_Error_To_String(const HRESULT errorCode)
        {
            switch (errorCode)
            {
                case DXGI_ERROR_DEVICE_HUNG:                   return "DXGI_ERROR_DEVICE_HUNG";                   // The application's device failed due to badly formed commands sent by the application. This is an design-time issue that should be investigated and fixed.
                case DXGI_ERROR_DEVICE_REMOVED:                return "DXGI_ERROR_DEVICE_REMOVED";                // The video card has been physically removed from the system, or a driver upgrade for the video card has occurred. The application should destroy and recreate the device. For help debugging the problem, call ID3D10Device::GetDeviceRemovedReason.
                case DXGI_ERROR_DEVICE_RESET:                  return "DXGI_ERROR_DEVICE_RESET";                  // The device failed due to a badly formed command. This is a run-time issue; The application should destroy and recreate the device.
                case DXGI_ERROR_DRIVER_INTERNAL_ERROR:         return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";         // The driver encountered a problem and was put into the device removed state.
                case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:     return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT";     // An event (for example, a power cycle) interrupted the gathering of presentation statistics.
                case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE:  return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE";  // The application attempted to acquire exclusive ownership of an output, but failed because some other application (or device within the application) already acquired ownership.
                case DXGI_ERROR_INVALID_CALL:                  return "DXGI_ERROR_INVALID_CALL";                  // The application provided invalid parameter data; this must be debugged and fixed before the application is released.
                case DXGI_ERROR_MORE_DATA:                     return "DXGI_ERROR_MORE_DATA";                     // The buffer supplied by the application is not big enough to hold the requested data.
                case DXGI_ERROR_NONEXCLUSIVE:                  return "DXGI_ERROR_NONEXCLUSIVE";                  // A global counter resource is in use, and the Direct3D device can't currently use the counter resource.
                case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:       return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";       // The resource or request is not currently available, but it might become available later.
                case DXGI_ERROR_NOT_FOUND:                     return "DXGI_ERROR_NOT_FOUND";                     // When calling IDXGIObject::GetPrivateData, the GUID passed in is not recognized as one previously passed to IDXGIObject::SetPrivateData or IDXGIObject::SetPrivateDataInterface. When calling IDXGIFentityy::EnumAdapters or IDXGIAdapter::EnumOutputs, the enumerated ordinal is out of range.
                case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:    return "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED";    // Reserved
                case DXGI_ERROR_REMOTE_OUTOFMEMORY:            return "DXGI_ERROR_REMOTE_OUTOFMEMORY";            // Reserved
                case DXGI_ERROR_WAS_STILL_DRAWING:             return "DXGI_ERROR_WAS_STILL_DRAWING";             // The GPU was busy at the moment when a call was made to perform an operation, and did not execute or schedule the operation.
                case DXGI_ERROR_UNSUPPORTED:                   return "DXGI_ERROR_UNSUPPORTED";                   // The requested functionality is not supported by the device or the driver.
                case DXGI_ERROR_ACCESS_LOST:                   return "DXGI_ERROR_ACCESS_LOST";                   // The desktop duplication interface is invalid. The desktop duplication interface typically becomes invalid when a different type of image is displayed on the desktop.
                case DXGI_ERROR_WAIT_TIMEOUT:                  return "DXGI_ERROR_WAIT_TIMEOUT";                  // The time-out interval elapsed before the next desktop frame was available.
                case DXGI_ERROR_SESSION_DISCONNECTED:          return "DXGI_ERROR_SESSION_DISCONNECTED";          // The Remote Desktop Services session is currently disconnected.
                case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:      return "DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE";      // The DXGI output (monitor) to which the swap chain content was restricted is now disconnected or changed.
                case DXGI_ERROR_CANNOT_PROTECT_CONTENT:        return "DXGI_ERROR_CANNOT_PROTECT_CONTENT";        // DXGI can't provide content protection on the swap chain. This error is typically caused by an older driver, or when you use a swap chain that is incompatible with content protection.
                case DXGI_ERROR_ACCESS_DENIED:                 return "DXGI_ERROR_ACCESS_DENIED";                 // You tried to use a resource to which you did not have the required access privileges. This error is most typically caused when you write to a shared resource with read-only access.
                case DXGI_ERROR_NAME_ALREADY_EXISTS:           return "DXGI_ERROR_NAME_ALREADY_EXISTS";           // The supplied name of a resource in a call to IDXGIResource1::CreateSharedHandle is already associated with some other resource.
                case DXGI_ERROR_SDK_COMPONENT_MISSING:         return "DXGI_ERROR_SDK_COMPONENT_MISSING";         // The application requested an operation that depends on an SDK component that is missing or mismatched.
                case DXGI_ERROR_NOT_CURRENT:                   return "DXGI_ERROR_NOT_CURRENT";                   // The DXGI objects that the application has created are no longer current & need to be recreated for this operation to be performed.
                case DXGI_ERROR_HW_PROTECTION_OUTOFMEMORY:     return "DXGI_ERROR_HW_PROTECTION_OUTOFMEMORY";     // Insufficient HW protected memory exits for proper function.
                case DXGI_ERROR_DYNAMIC_CODE_POLICY_VIOLATION: return "DXGI_ERROR_DYNAMIC_CODE_POLICY_VIOLATION"; // Creating this device would violate the process's dynamic code policy.
                case DXGI_ERROR_NON_COMPOSITED_UI:             return "DXGI_ERROR_NON_COMPOSITED_UI";             // The operation failed because the compositor is not in control of the output.
                case DXGI_DDI_ERR_UNSUPPORTED:                 return "DXGI_DDI_ERR_UNSUPPORTED";
                case E_INVALIDARG:                             return "E_INVALIDARG";                             // One or more arguments are invalid.
            }

            return (std::string("Unknown Error Code: %d.", std::system_category().message(errorCode).c_str()).c_str());
        }

        // Matches our RHI_Format enum in RHI_Utilities. As enums are essentially integer literals, they can be used to index these array lists.
        static const DXGI_FORMAT ToDX11Format[] =
        {
            DXGI_FORMAT_UNKNOWN,
            DXGI_FORMAT_R8G8B8A8_UNORM,

            // RGB
            DXGI_FORMAT_R11G11B10_FLOAT,

            // Depth
            DXGI_FORMAT_D32_FLOAT,
            DXGI_FORMAT_D32_FLOAT_S8X24_UINT
        };

        constexpr bool ErrorCheck(const HRESULT result)
        {
            if (FAILED(result))
            {
                std::cout << "Error! " << DXGI_Error_To_String(result);
                return false;
            }

            return true;
        }

        inline DX11_Context* GetDX11Context()
        {
            return static_cast<DX11_Context*>(GlobalContext::m_RHI_Context); // Static cast here as there is no virtual inheritence in our context.

            // return std::dynamic_pointer_cast<DX11_Context>(rhiContext).get();
        }

        inline void QueryAdaptersAndDisplays()
        {
            IDXGIFactory1* factory;
            const HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&factory)); // Reference counted.
            if (FAILED(result))
            {
                std::cout << "Failed to create DXGI Factory: " << DXGI_Error_To_String(result);
                return;
            }

            // An adapter represents a display subsystem (including one or more GPUs, DACs and video memory).
            const auto GetAvaliableAdapters = [](IDXGIFactory1* factory)
            {
                uint32_t i = 0; // Enumerating index.
                IDXGIAdapter* adapter; // Enumerating adapter.

                std::vector<IDXGIAdapter*> adapters;
                while (factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND) // Enumerates the adapters (video cards).
                {
                    adapters.emplace_back(adapter);
                    ++i;
                }

                return adapters;
            };

            // Get all avaliable adapters.
            std::vector<IDXGIAdapter*> adapters = GetAvaliableAdapters(factory);
            factory->Release();
            factory = nullptr;
            if (adapters.empty())
            {
                std::cout << "Couldn't find any adapters.\n";
                return;
            }

            // Save all avaliable adapters.
            DXGI_ADAPTER_DESC adapterDescription;
            for (IDXGIAdapter* displayAdapter : adapters)
            {
                if (FAILED(displayAdapter->GetDesc(&adapterDescription)))
                {
                    std::cout << "Failed to retrieve adapter description.\n";
                    continue;
                }

                // Of course it wouldn't be simple. Let's convert the device name.
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

            // Get display modes. Querying display modes ensures that our application can properly choose a valid full-screen mode.
            const auto GetDisplayModes = [](IDXGIAdapter* adapter, RHI_Format queryingFormat)
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
                    if (ErrorCheck(adapterOutput->GetDisplayModeList(ToDX11Format[queryingFormat], DXGI_ENUM_MODES_INTERLACED, &displayModeCount, nullptr))) // Get count, querying based on the format we wish to support.
                    {
                        // Actually retrieve the display modes themselves.
                        std::vector<DXGI_MODE_DESC> displayModes;
                        displayModes.resize(displayModeCount);
                        if (ErrorCheck(adapterOutput->GetDisplayModeList(ToDX11Format[queryingFormat], DXGI_ENUM_MODES_INTERLACED, &displayModeCount, &displayModes[0])))
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
                IDXGIAdapter* adapter = static_cast<IDXGIAdapter*>(gpu.GetInternalData());

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

            // If we failed to get any display modes but have at least one adapter, use it.
            if (GlobalContext::m_RHI_Device->GetGPUs().size() != 0)
            {
                std::cout << "Falling back to 1st default GPU adapter avaliable.\n";
                GlobalContext::m_RHI_Device->SetPrimaryGPU(0);
            }
        }

        // Determine maximum MSAA level supported - 16.
        inline UINT QueryMaximumMSAALevel()
        {

        }

        /*
            Screen tear is a visual effect that occurs when your GPU's frame rate doesn't match your monitor's refresh rate. This can happen if your GPU outputs 
            a higher framerate than the monitor's refresh rate. This leads to two or more frames being displayed by the monitor at the same time. 
            
            V-Sync (Vertical Synchronization) is a way to prevent screen tears. This technology would synchronize the vertical refresh rate of your display 
            with the framerate your GPU was providing. However, as VSync would give priority to your monitor, your GPU would often have to wait until the monitor
            was ready to receive a frame before sending it - ultimately leading to much greater input lag. Imagine if your FPS drops to 20, your monitor will have to 
            wait a good amount of tuime before freshing the image on screen.

            For example, a 60hz monitor with VSync enabled will force your GPU to cap the framerate to 60 regardless of how high it supports. To uncap your frame rate, 
            turn off VSync. More VSync content here: https://www.wepc.com/gaming-monitor/faq/what-is-vsync/.

            Another feature called VRR (Variable Refresh Rate, or V-Sync Off Mode) also allows your screen to adjust how often it refreshes its image to match the frame rate
            from your GPU, either in a console or PC. There are technologies out there based on VRR, such as Nvidia GSync, which updates the screen only when the GPU tells it to.
        */
        inline bool QueryTearingSupport()
        {
            // Rather than creaing the 1.5 factory interface directly, we create the 1.4 interface and query for the 1.5 interface. This will enable graphics debugging tools which might not support the 1.5 factory interface.
            ComPtr<IDXGIFactory4> factory4;
            HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&factory4));
            BOOL allowTearing = FALSE;

            if (SUCCEEDED(result))
            {
                ComPtr<IDXGIFactory5> factory5; // IDXGIFactory5 enables a single method to support variable refresh rate displays.
                result = factory4.As(&factory5);

                if (SUCCEEDED(result))
                {
                    result = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
                }
            }

            const bool fullscreenBorderlessSupport = SUCCEEDED(result) && allowTearing;
            /// Apparently Intel doesn't support tearing. Might want to check on that?

            return fullscreenBorderlessSupport;
        }

        namespace SwapChain
        {
            inline uint32_t ValidateFlags(uint32_t flags)
            {
                // If tearing support is requested...
                if (flags & RHI_Present_Mode::RHI_Present_Immediate)
                {
                    // Check if the adapter supports it. If not, disable it as it tends to fail with Intel adapters.
                    if (!QueryTearingSupport())
                    {
                        flags &= ~RHI_Present_Immediate;
                        std::cout << "Present_Immediate was requested by wasn't supported by the adapter. Removing flag...\n";
                    }
                    else
                    {
                        std::cout << "Tearing is supported. Enabling feature flag...\n";
                    }
                }

                return flags;
            }

            inline UINT ToDX11Flags(uint32_t flags)
            {
                UINT dxFlags = 0;

                dxFlags |= flags & RHI_Swap_Allow_Mode_Switch ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0; // Allows for mode switching (fullscreen/windowed mode) by calling IDXGISwapChain::ResizeTarget. The display mode (or monitor resolution) will be changed to match the dimensions of the application window.
                // To allow VSync-Off support, we have to check for the below flag. This flag can only bw used with sync interval 0.
                dxFlags |= flags & RHI_Present_Immediate      ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING     : 0; // 
                
                return dxFlags;
            }

            inline DXGI_SWAP_EFFECT ToDX11SwapEffect(uint32_t flags)
            {
                if (flags & RHI_Swap_Discard)         { return DXGI_SWAP_EFFECT_DISCARD;      }
                if (flags & RHI_Swap_Sequential)      { return DXGI_SWAP_EFFECT_SEQUENTIAL;   }
                if (flags & RHI_Swap_Flip_Discard)    { return DXGI_SWAP_EFFECT_FLIP_DISCARD; }
                if (flags & RHI_Swap_Flip_Sequential) { return DXGI_SWAP_EFFECT_SEQUENTIAL;   }

                std::cout << "Failed to determine the requested swap effect, opting for DXGI_SWAP_EFFECT_DISCARD.\n";
                return DXGI_SWAP_EFFECT_DISCARD;
            }
        }
    }
}