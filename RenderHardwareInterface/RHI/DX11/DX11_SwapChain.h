#pragma once
#include "../RHI_SwapChain.h"

namespace Aurora
{
    class DX11_SwapChain : public RHI_SwapChain
    {
    public:
        DX11_SwapChain(uint32_t width, uint32_t height, void* windowHandle, RHI_Device* rhiDevice, uint32_t bufferCount = 2, uint32_t flags = RHI_Present_Mode::RHI_Present_Immediate, RHI_Format format = RHI_Format::RHI_Format_R8G8B8A8_Unorm, const char* objectName = nullptr);
        virtual ~DX11_SwapChain() override;

        virtual void Resize(uint32_t newWidth, uint32_t newHeight, const bool forceResize = false) override;
    };
}