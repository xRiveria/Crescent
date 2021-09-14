#pragma once
#include <d3d12.h>
#include <stdexcept>
#include <dxgi1_6.h>

namespace Crescent
{
    inline std::string HRToString(HRESULT hr)
    {
        char stringBuffer[64] = {};
        sprintf_s(stringBuffer, "HRESULT of 0x%08X", static_cast<UINT>(hr));
        return std::string(stringBuffer);
    }

    class HRException : public std::runtime_error
    {
    public:
        HRException(HRESULT hr) : std::runtime_error(HRToString(hr)), m_HR(hr) {}
        HRESULT GetError() const { return m_HR; }

    private:
        const HRESULT m_HR;
    };

    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            throw HRException(hr);
        }
    }

    // Helper function for acquiring the first avaliable hardware adapter that supports Direct3D 12. If no such adapter can be found, *ppAdapter will be set to nullptr;
    inline void GetHardwareAdapter(IDXGIFactory1* pFactory, IDXGIAdapter1** ppAdapter, bool requestHigherPerformanceAdapter = false)
    {
        *ppAdapter = nullptr;

        ComPtr<IDXGIAdapter1> adapter;
        ComPtr<IDXGIFactory6> factory6;

        if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
        {
            for (UINT adapterIndex = 0;
                SUCCEEDED(factory6->EnumAdapterByGpuPreference(adapterIndex, requestHigherPerformanceAdapter == true ?
                DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&adapter)));
                ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 adapterDescription;
                adapter->GetDesc1(&adapterDescription);

                if (adapterDescription.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver wadapter. If you we want a software adapter, pass in /warp on the command line.
                    continue;
                }

                // Check to see whether the adapter supports DX12, but don't create the actual device yet.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        if (adapter.Get() == nullptr)
        {
            for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
            {
                DXGI_ADAPTER_DESC1 adapterDescription;
                adapter->GetDesc1(&adapterDescription);

                if (adapterDescription.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
                {
                    // Don't select the Basic Render Driver adapter. 
                    continue;
                }

                // Check to see whether the adapter supports DX12.
                if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
                {
                    break;
                }
            }
        }

        *ppAdapter = adapter.Detach(); // Detach disassociates the ComPtr from the interface that it represents.
    }
}