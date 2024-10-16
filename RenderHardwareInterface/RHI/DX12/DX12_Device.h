#pragma once
#include "../RHI_Device.h"
#include "DX12_Context.h"

namespace Aurora
{
    class DX12_Device : public RHI_Device
    {
    public:
        virtual ~DX12_Device() override;

        virtual void Initialize() override;

    };
}