#pragma once
#include "../RHI_Device.h"
#include "DX11_Context.h"

namespace Aurora
{
    class DX11_Device : public RHI_Device
    {
    public:
        virtual ~DX11_Device() override;

        virtual void Initialize() override;
        virtual bool IsInitialized() const override { return m_IsInitialized; }

        DX11_Context* GetContext() const;

    private:
        bool m_IsInitialized = false;
    };
}