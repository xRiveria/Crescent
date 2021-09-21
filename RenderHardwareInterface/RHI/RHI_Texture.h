#pragma once
#include <memory>
#include "RHI_Utilities.h"
#include "RHI_Viewport.h"

namespace Aurora
{
    enum RHI_Texture_Type
    {
        Unknown,
        Texture2D,
        TextureCube
    };

    enum RHI_Texture_Flag : uint32_t
    {
        RHI_Texture_SRV                         = 1 << 0,
        RHI_Texture_UAV                         = 1 << 1,
        RHI_Texture_RT_Color                    = 1 << 2,
        RHI_Texture_RT_DepthStencil             = 1 << 3,
        RHI_Texture_RT_DepthStencil_ReadOnly    = 1 << 4
    };

    enum RHI_Texture_ShaderView_Type : uint8_t
    {
        RHI_Texture_ShaderView_ColorDepth,
        RHI_Texture_ShaderView_Stencil,
        RHI_Texture_ShaderView_UnorderedAccess
    };

    class RHI_Texture
    {
    public:
        RHI_Texture() = default;
        virtual ~RHI_Texture() = default;

        // Flags
        bool HasSRV() const { return m_Flags & RHI_Texture_Flag::RHI_Texture_SRV; }
        bool HasUAV() const { return m_Flags & RHI_Texture_Flag::RHI_Texture_UAV; }
        bool IsRTColor() const { return m_Flags & RHI_Texture_Flag::RHI_Texture_RT_Color; }
        bool IsRTDepthStencil() const { return m_Flags & RHI_Texture_Flag::RHI_Texture_RT_DepthStencil; }

        // API
        static std::shared_ptr<RHI_Texture> Create(const uint32_t flags = RHI_Texture_Flag::RHI_Texture_SRV, RHI_Texture_Type textureType = RHI_Texture_Type::Texture2D, const char* objectName = nullptr);
        static std::shared_ptr<RHI_Texture> Create(const uint32_t width, const uint32_t height, const uint32_t mipCount, const RHI_Format format, const uint32_t flags, RHI_Texture_Type textureType, const char* objectName = nullptr);

    protected:
        virtual bool CreateResourceInternal() = 0;

    protected:
        uint32_t m_Width = 0;
        uint32_t m_Height = 0;
        uint32_t m_ChannelCount = 0;
        uint32_t m_MipCount = 1;
        RHI_Format m_Format = RHI_Format::RHI_Format_Undefined;
        uint32_t m_Flags = 0;
        RHI_Viewport m_Viewport;
        RHI_Texture_Type m_TextureType = RHI_Texture_Type::Unknown;
        uint32_t m_ArrayLength = 1;

        RHI_Device* m_RHI_Device = nullptr;

        // API
        void* m_Resource = nullptr;
        void* m_ResourceView_SRV = nullptr;
        void* m_ResourceView_UAV = nullptr;
        void* m_ResourceView_RenderTargetView = nullptr;
        void* m_ResourceView_DepthStencilView = nullptr;
    };
}