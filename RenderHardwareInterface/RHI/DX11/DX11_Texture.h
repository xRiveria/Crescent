#pragma once
#include "../RHI_Texture2D.h"
#include "DX11_Context.h"

namespace Aurora
{
    class DX11_Texture : public RHI_Texture
    {
    public:
        DX11_Texture(const uint32_t flags = RHI_Texture_Flag::RHI_Texture_SRV, RHI_Texture_Type textureType = RHI_Texture_Type::Texture2D, const char* objectName = nullptr);
        DX11_Texture(const uint32_t width, const uint32_t height, const uint32_t mipCount, const RHI_Format format, const uint32_t flags, RHI_Texture_Type textureType, const char* objectName = nullptr);
    
        virtual ~DX11_Texture() override;

    protected:
        virtual bool CreateResourceInternal() override;

    private:
        bool CreateTexture();
        bool CreateShaderResourceView();
        bool CreateRenderTargetView();
        bool CreateDepthStencilView();
        bool CreateUnorderedAccessView();

        UINT ToDX11_BindFlags(uint32_t flags);
        DXGI_FORMAT ToDX11_TextureFormat(RHI_Format format);
        DXGI_FORMAT ToDX11_SRVFormat(RHI_Format format);
        DXGI_FORMAT ToDX11_DSVFormat(RHI_Format format);
    };
}