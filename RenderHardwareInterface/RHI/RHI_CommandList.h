#pragma once
#include <memory>
#include "RHI_Utilities.h"
#include "RHI_Texture.h"
#include "../Math/Vector4.h"

namespace Aurora
{
    class RHI_CommandList
    {
    public:
        RHI_CommandList();
        ~RHI_CommandList();

        // Command List
        bool Begin();
        bool End();
        bool Wait();
        bool Flush();
        bool Reset();

        // Clear
        void ClearRenderTarget(RHI_Texture* texture, const uint32_t colorIndex = 0, const uint32_t depthStencilIndex = 0, 
                               const bool isStorage = false, const Vector4& clearColor = m_RHI_Color_Load, 
                               const float clearDepth = m_RHI_Depth_Load, const uint32_t clearStencil = m_RHI_Stencil_Load);

        // Draw
        bool Draw(uint32_t vertexCount);
        bool DrawInstanced(uint32_t indexCount, uint32_t indexOffset = 0, uint32_t vertexOffset = 0);

        // Vertex Buffer
        // void SetBufferVertex(const RHI_VertexBuffer* buffer, const uint64_t offset = 0);

        // Index Buffer
        // void SetBufferIndex(const RHI_IndexBuffer* buffer, const uint64_t offset = 0);
    };
}
