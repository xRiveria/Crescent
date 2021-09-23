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

    // Retrieve buffer count from renderer/swapchain. Heap type based on texture format.
    bool DX12_Texture::CreateResourceInternal()
    {
        // Heap with 2 descriptors for our RTV.
        DX12_Utilities::Heap::Create(m_ResourceHeap_RenderTargetView, 2, RHI_Descriptor_Heap_Flag::RHI_Descriptor_Heap_Flag_None, RHI_Descriptor_Heap_Type::RHI_Descriptor_Heap_Type_RTV);

        // Create frame resources.
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle(static_cast<ID3D12DescriptorHeap*>(m_ResourceHeap_RenderTargetView)->GetCPUDescriptorHandleForHeapStart());

        // Create an RTV for each frame.
        for (UINT i = 0; i < 2; i++)
        {

        }

        return true;
    }
}