#pragma once
#include "../RHI_Device.h"

namespace Aurora
{
    class Vulkan_Device : public RHI_Device
    {
    public:
        virtual void Initialize() override;
        
        virtual bool IsInitialized() const override { return m_IsInitialized; }

    private:
        bool m_IsInitialized = false;
    };
}