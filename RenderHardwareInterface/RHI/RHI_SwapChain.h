#pragma once
#include "RHI_Utilities.h"
#include "RHI_Device.h"

namespace Aurora
{
    class RHI_SwapChain
    {
    public:
        RHI_SwapChain() = default;
        virtual ~RHI_SwapChain() = default;

        // Misc
        virtual void Resize(uint32_t newWidth, uint32_t newHeight, const bool forceResize = false) = 0;
        bool IsInitialized() const { return m_IsInitialized; }

        // API
        static std::shared_ptr<RHI_SwapChain> Create(uint32_t width, 
                                                     uint32_t height, 
                                                     void* windowHandle, 
                                                     RHI_Device* rhiDevice, 
                                                     uint32_t bufferCount   = 2, 
                                                     uint32_t flags         = RHI_Present_Mode::RHI_Present_Immediate, 
                                                     RHI_Format format      = RHI_Format::RHI_Format_R8G8B8A8_Unorm, 
                                                     const char* objectName = nullptr);

    protected:
        bool m_IsInitialized = false;
        bool m_IsWindowed = false;

        // Properties
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_BufferCount = 0;
        uint32_t m_Flags = 0;
        RHI_Format m_Format = RHI_Format::RHI_Format_R8G8B8A8_Unorm;

        // API
        void* m_SwapChain = nullptr;

        RHI_Device* m_RHI_Device = nullptr;
    };
}