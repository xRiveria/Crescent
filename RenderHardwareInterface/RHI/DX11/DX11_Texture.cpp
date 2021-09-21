#include "RHI_PCH.h"
#include "DX11_Texture.h"
#include "DX11_Utilities.h"

namespace Aurora
{
    DX11_Texture::DX11_Texture(const uint32_t flags, RHI_Texture_Type textureType, const char* objectName)
    {
        m_TextureType = textureType;
        m_Flags = flags;
    }

    DX11_Texture::DX11_Texture(const uint32_t width, const uint32_t height, const uint32_t mipCount, const RHI_Format format, const uint32_t flags, RHI_Texture_Type textureType, const char* objectName)
    {
        m_TextureType = textureType;
        m_Width = width;
        m_Height = height;
        m_Viewport = RHI_Viewport(0, 0, static_cast<float>(width), static_cast<float>(height));
        m_Format = format;
        m_MipCount = mipCount;
        m_Flags = flags;

        if (CreateResourceInternal())
        {
            std::cout << "Texture creation success: " << objectName << "\n";
        }
    }

    DX11_Texture::~DX11_Texture()
    {
    }

    bool DX11_Texture::CreateResourceInternal()
    {
        /// Assert devices.

        bool resultTexture = true;
        bool resultSRV = true;
        bool resultUAV = true;
        bool resultRTV = true;
        bool resultDSV = true;

        resultTexture = CreateTexture();

        if (HasSRV())
        {
            resultSRV = CreateShaderResourceView();
        }

        if (HasUAV())
        {
            resultUAV = CreateUnorderedAccessView();
        }

        if (IsRTDepthStencil())
        {
            resultDSV = CreateDepthStencilView();
        }

        if (IsRTColor())
        {
            resultRTV = CreateRenderTargetView();
        }

        return resultTexture && resultSRV && resultUAV && resultRTV && resultDSV;
    }

    bool DX11_Texture::CreateTexture()
    {
        RHI_ASSERT_STATEMENT(m_Width != 0);
        RHI_ASSERT_STATEMENT(m_Height != 0);

        D3D11_TEXTURE2D_DESC textureDescription = {};
        textureDescription.Width = static_cast<UINT>(m_Width);
        textureDescription.Height = static_cast<UINT>(m_Height);
        textureDescription.ArraySize = static_cast<UINT>(m_ArrayLength);
        textureDescription.MipLevels = static_cast<UINT>(m_MipCount);
        textureDescription.Format = ToDX11_TextureFormat(m_Format);
        textureDescription.SampleDesc.Count = 1;
        textureDescription.SampleDesc.Quality = 0;
        textureDescription.Usage = D3D11_USAGE_DEFAULT;
        textureDescription.BindFlags = ToDX11_BindFlags(m_Flags);
        textureDescription.MiscFlags = 0;
        textureDescription.CPUAccessFlags = 0;

        ID3D11Texture2D* texture = nullptr;
        // Create
        if (!DX11_Utilities::ErrorCheck(DX11_Utilities::GetDX11Context()->m_Device->CreateTexture2D(&textureDescription, nullptr, &texture)))
        {
            return false;
        }

        m_Resource = static_cast<void*>(texture);
        std::cout << "Successfully created Texture.\n";
        return true;
    }

    bool DX11_Texture::CreateShaderResourceView()
    {
        RHI_ASSERT_DATA(m_Resource);

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDescription = {};
        srvDescription.Format = ToDX11_SRVFormat(m_Format);

        if (m_TextureType == RHI_Texture_Type::Texture2D)
        {
            srvDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDescription.Texture2D.MostDetailedMip = 0;
            srvDescription.Texture2D.MipLevels = m_MipCount; // 1
        }
        else if (m_TextureType == RHI_Texture_Type::TextureCube)
        {
            /// To support.
            return false;
        }

        if (!DX11_Utilities::ErrorCheck(DX11_Utilities::GetDX11Context()->m_Device->CreateShaderResourceView(static_cast<ID3D11Resource*>(m_Resource), &srvDescription, reinterpret_cast<ID3D11ShaderResourceView**>(m_ResourceView_SRV))))
        {
            return false;
        }

        std::cout << "Successfully created Shader Resource View.\n";
        return true;
    }

    bool DX11_Texture::CreateRenderTargetView()
    {
        RHI_ASSERT_DATA(m_Resource);

        D3D11_RENDER_TARGET_VIEW_DESC rtvDescription = {};
        rtvDescription.Format = ToDX11_TextureFormat(m_Format);
        rtvDescription.ViewDimension = m_TextureType == RHI_Texture_Type::Texture2D ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_UNKNOWN;
        // rtvDescription.Texture2DArray.MipSlice = 0;
        // rtvDescription.Texture2DArray.ArraySize = 1;

        // Create
        // for (uint32_t i = 0; i < m_ArrayLength; i++)
        // {
        //    rtvDescription.Texture2DArray.FirstArraySlice = i;

        if (!DX11_Utilities::ErrorCheck(DX11_Utilities::GetDX11Context()->m_Device->CreateRenderTargetView(static_cast<ID3D11Resource*>(m_Resource), &rtvDescription, reinterpret_cast<ID3D11RenderTargetView**>(m_ResourceView_RenderTargetView))))
        {
            return false;
        }
        //}

        std::cout << "Successfully created Render Target View.\n";
        return true;
    }

    bool DX11_Texture::CreateDepthStencilView()
    {
        RHI_ASSERT_DATA(m_Resource);

        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDescription = {};
        dsvDescription.Format = ToDX11_DSVFormat(m_Format);
        dsvDescription.ViewDimension = m_TextureType == RHI_Texture_Type::Texture2D ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_UNKNOWN;

        if (!DX11_Utilities::ErrorCheck(DX11_Utilities::GetDX11Context()->m_Device->CreateDepthStencilView(static_cast<ID3D11Resource*>(m_Resource), &dsvDescription, reinterpret_cast<ID3D11DepthStencilView**>(m_ResourceView_DepthStencilView))))
        {
            return false;
        }

        std::cout << "Successfully created Depth Stencil View.\n";
        return true;
    }

    bool DX11_Texture::CreateUnorderedAccessView()
    {
        return true;
    }

    UINT DX11_Texture::ToDX11_BindFlags(uint32_t flags)
    {
        UINT d3d11Flags = 0;

        d3d11Flags |= (flags & RHI_Texture_Flag::RHI_Texture_SRV)             ? D3D11_BIND_SHADER_RESOURCE  : 0;
        d3d11Flags |= (flags & RHI_Texture_Flag::RHI_Texture_UAV)             ? D3D11_BIND_UNORDERED_ACCESS : 0;
        d3d11Flags |= (flags & RHI_Texture_Flag::RHI_Texture_RT_DepthStencil) ? D3D11_BIND_DEPTH_STENCIL    : 0;
        d3d11Flags |= (flags & RHI_Texture_Flag::RHI_Texture_RT_Color)        ? D3D11_BIND_RENDER_TARGET    : 0;

        return d3d11Flags;
    }

    DXGI_FORMAT DX11_Texture::ToDX11_TextureFormat(RHI_Format format)
    {
        if (format == RHI_Format::RHI_Format_D32_Float_S8X24_UInt)
        {
            return DXGI_FORMAT_R32G8X24_TYPELESS;
        }

        if (format == RHI_Format::RHI_Format_D32_Float)
        {
            return DXGI_FORMAT_R32_TYPELESS;
        }

        return DX11_Utilities::ToDX11Format[format];
    }

    DXGI_FORMAT DX11_Texture::ToDX11_SRVFormat(RHI_Format format)
    {
        if (format == RHI_Format_D32_Float_S8X24_UInt)
        {
            return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS; // 32 bit red channel (for depth reading), 8 bits and 24 bits unused.
        }
        else if (format == RHI_Format_D32_Float)
        {
            return DXGI_FORMAT_R32_FLOAT; // 32 bit red channel.
        }

        return DX11_Utilities::ToDX11Format[format];
    }

    DXGI_FORMAT DX11_Texture::ToDX11_DSVFormat(RHI_Format format)
    {
        if (format == RHI_Format::RHI_Format_D32_Float_S8X24_UInt)
        {
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        }

        if (format == RHI_Format::RHI_Format_D32_Float)
        {
            return DXGI_FORMAT_D32_FLOAT;
        }

        return DX11_Utilities::ToDX11Format[format];
    }
}