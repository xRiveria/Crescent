#include "RHI_PCH.h"
#include "DX11_Device.h"
#include "DX11_Utilities.h"

namespace Aurora
{
    DX11_Device::~DX11_Device()
    {
        DX11_Utilities::GetDX11Context(m_RHI_Context)->m_DeviceContext->Release();
        DX11_Utilities::GetDX11Context(m_RHI_Context)->m_DeviceContext = nullptr;

        DX11_Utilities::GetDX11Context(m_RHI_Context)->m_Device->Release();
        DX11_Utilities::GetDX11Context(m_RHI_Context)->m_Device = nullptr;

        DX11_Utilities::GetDX11Context(m_RHI_Context)->m_Annotation->Release();
        DX11_Utilities::GetDX11Context(m_RHI_Context)->m_Annotation = nullptr;
    }

    void DX11_Device::Initialize()
    {
        m_RHI_Context = std::make_shared<DX11_Context>();

        // Pass our pointers into the global utility namespace.
        DX11_Utilities::GlobalContext::m_RHI_Context = m_RHI_Context.get();
        DX11_Utilities::GlobalContext::m_RHI_Device = this;
        const bool multithreadedProtection = true;

        // Detect and set ideal GPU adapter.
        DX11_Utilities::QueryAdaptersAndDisplays();

        m_RHI_Context->m_Texture2D_Dimensions_Max = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;

        const RHI_GPU* acquiredGPU = GetPrimaryGPU();
        if (!acquiredGPU)
        {
            std::cout << "Failed to detect any device.\n";
            return;
        }

        // Create DX11 Device
        UINT deviceFlags = 0;
        
        if (m_RHI_Context->m_DebuggingEnabled)
        {
            deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        }

        // The order of the feature levels that we will try to create a device with. Any level above 11.1 are for DirectX 12 implementations.
        std::vector<D3D_FEATURE_LEVEL> featureLevels =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        IDXGIAdapter* adapter = static_cast<IDXGIAdapter*>(acquiredGPU->GetInternalData());
        // We specify this as Unknown. In D3D11 context, unknown doesn't mean "I don't know it". It means "I don't specify it, the runtime or driver gets this information from elsewhere".
        // The call to create will determine the driver type for the adapter, hence the adapter contains the driver type. We provide this adapter entry point.
        D3D_DRIVER_TYPE driverType = adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE;

        const auto CreateDevice = [this, &adapter, &driverType, &deviceFlags, &featureLevels]()
        {
            ID3D11Device* temporaryDevice = nullptr;
            ID3D11DeviceContext* temporaryDeviceContext = nullptr;

            const HRESULT result = D3D11CreateDevice(
                adapter,                                    // Adapter
                driverType,                                 // Driver Type
                nullptr,                                    // Software Driver Implementation
                deviceFlags,                                // Device Flags
                featureLevels.data(),                       // Feature Levels
                static_cast<UINT>(featureLevels.size()),    // Size of Feature Levels
                D3D11_SDK_VERSION,                          // Default to D3D11_SDK_VERSION
                &temporaryDevice,                           // Pointer to our Device
                nullptr,                                    // Feature Level
                &temporaryDeviceContext                     // Pointer to our Device Context
            );

            if (SUCCEEDED(result))
            {
                // Query old device for newer interface.
                if (!DX11_Utilities::ErrorCheck(temporaryDevice->QueryInterface(__uuidof(ID3D11Device5), (void**)&DX11_Utilities::GetDX11Context(m_RHI_Context)->m_Device)))
                {
                    return E_FAIL;
                }

                // Release old device.
                temporaryDevice->Release();
                temporaryDevice = nullptr;

                // Query old device for newer interface.
                if (!DX11_Utilities::ErrorCheck(temporaryDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext4), (void**)&DX11_Utilities::GetDX11Context(m_RHI_Context)->m_DeviceContext)))
                {
                    return E_FAIL;
                }

                // Release old context.
                temporaryDeviceContext->Release();
                temporaryDeviceContext = nullptr;
            }

            return result;
        };

        // Create Direct3D device and device context.
        HRESULT result = CreateDevice();

        // Our D3D11 create device flags must have D3D11SDKLayers installed. Otherwise, device creation fails. Lets keep track of that.
        if (result == DXGI_ERROR_SDK_COMPONENT_MISSING)
        {
            // Warning
            std::cout << "Failed to create device with D3D11_CREATE_DEVICE_DEBUG flags as it requires the DirectX SDK to be installed. Attempting to create one without it...";
            deviceFlags &= ~D3D11_CREATE_DEVICE_DEBUG;
            result = CreateDevice();
        }

        if (FAILED(result))
        {
            std::cout << "Failed to create DX11_Device. " << DX11_Utilities::DXGI_Error_To_String(result);
            return;
        }       

        std::cout << "Created DX11 Device and Context.\n";

        // By default, applications can only use one thread with immediate context. With this, we can change that restriction and turn on threading protection for the immediate context, which will increase the overhead of each immediate context call to share one context with multiple threads.
        if (multithreadedProtection) // Provides threading protection for critical sections of a multi-threaded applications.
        {
            ID3D11Multithread* multithreadingContext = nullptr;
            if (SUCCEEDED(DX11_Utilities::GetDX11Context(m_RHI_Context)->m_DeviceContext->QueryInterface(__uuidof(ID3D11Multithread), reinterpret_cast<void**>(&multithreadingContext))))
            {
                multithreadingContext->SetMultithreadProtected(TRUE);
                multithreadingContext->Release();
            }
            else
            {
                std::cout << "Failed to enable multi-threading protection.\n";
            }
        }

        std::cout << "Successfully initialized Multithreading.\n";
                
        if (m_RHI_Context->m_DebuggingEnabled)
        {
            // Queries and retrieves a pointer to the requested interface whilst calling AddRef.
            const HRESULT result = DX11_Utilities::GetDX11Context(m_RHI_Context)->m_DeviceContext->QueryInterface(IID_PPV_ARGS(&DX11_Utilities::GetDX11Context(m_RHI_Context)->m_Annotation));
            if (FAILED(result))
            {
                std::cout << DX11_Utilities::DXGI_Error_To_String(result);
                return;
            }

            std::cout << "Debug Annotations Enabled.\n";
        }

        std::cout << "Successfully initialized DX11.\n";
        std::cout << RHI_Display::GetWidthPrimary() << ", " << RHI_Display::GetHeightPrimary() << "\n";

        m_IsInitialized = true;
    }
}