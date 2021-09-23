#pragma once
#include "../RHI_Device.h"

namespace Aurora
{
    class Vulkan_Device : public RHI_Device
    {
    public:
        virtual void Initialize() override;
        virtual ~Vulkan_Device() override;
    };
}