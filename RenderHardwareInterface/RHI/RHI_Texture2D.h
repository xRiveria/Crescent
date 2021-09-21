#pragma once
#include "RHI_Texture.h"

namespace Aurora
{
    class RHI_Texture2D : public RHI_Texture
    {
    public:
        // Creates a texture without data, intended for manual loading.
        static std::shared_ptr<RHI_Texture> Create(const uint32_t flags = RHI_Texture_Flag::RHI_Texture_SRV, const char* objectName = nullptr)
        {
            return RHI_Texture::Create(flags, RHI_Texture_Type::Texture2D, objectName);
        }

        // Creates a texture without data (intended for use as a render target).
        static std::shared_ptr<RHI_Texture> Create(const uint32_t width, const uint32_t height, const uint32_t mipCount, const RHI_Format format, const uint32_t flags, const char* objectName = nullptr)
        {
            return RHI_Texture::Create(width, height, mipCount, format, flags, RHI_Texture_Type::Texture2D, objectName);
        }

        virtual ~RHI_Texture2D() override = default;
    };
}