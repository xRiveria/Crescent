#pragma once
#include "../RHI_Texture.h"

namespace Aurora
{
    class DX12_Texture : public RHI_Texture
    {
    public:
        DX12_Texture(const uint32_t flags = RHI_Texture_Flag::RHI_Texture_SRV, RHI_Texture_Type textureType = RHI_Texture_Type::Texture2D, const char* objectName = nullptr);
        DX12_Texture(const uint32_t width, const uint32_t height, const uint32_t mipCount, const RHI_Format format, const uint32_t flags, RHI_Texture_Type textureType, const char* objectName = nullptr);

        virtual ~DX12_Texture() override;

    protected:
        virtual bool CreateResourceInternal() override;
    };
}