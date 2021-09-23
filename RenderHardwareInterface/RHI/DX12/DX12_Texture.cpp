#include "RHI_PCH.h"
#include "DX12_Utilities.h"
#include "DX12_Texture.h"

namespace Aurora
{
    DX12_Texture::DX12_Texture(const uint32_t flags, RHI_Texture_Type textureType, const char* objectName)
    {
        m_TextureType = textureType;
        m_Flags = flags;
    }

    DX12_Texture::DX12_Texture(const uint32_t width, const uint32_t height, const uint32_t mipCount, const RHI_Format format, const uint32_t flags, RHI_Texture_Type textureType, const char* objectName)
    {
        m_TextureType = textureType;
        m_Width = width;
        m_Height = height;
        m_Format = format;
        m_MipCount = mipCount;
        m_Flags = flags;

        if (CreateResourceInternal())
        {
            
        }
    }

    DX12_Texture::~DX12_Texture()
    {

    }

    bool DX12_Texture::CreateResourceInternal()
    {


        return true;
    }
}