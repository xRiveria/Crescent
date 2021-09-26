#include "RHI_PCH.h"
#include "RHI_States.h"
#include "../Renderer/Renderer.h"
#include "DX12/DX12_States.h"

namespace Aurora
{
    std::shared_ptr<RHI_RasterizerState> RHI_RasterizerState::Create(const RHI_Cull_Mode cullMode, const RHI_Fill_Mode fillMode, const bool counterClockwise, const bool depthClippingEnabled, const bool scissorsEnabled, const bool antialiasedLineEnabled, const float depthBias, const float depthBiasClamp, const float depthBiasSlopedScaled, const float lineWidth)
    {
        switch (Renderer::GetCurrentRenderAPI())
        {
            // case RenderAPI::DirectX11: return std::make_shared<DX11_Device>();
            case RenderAPI::DirectX12: return std::make_shared<DX12_RasterizerState>(cullMode, fillMode, counterClockwise, depthClippingEnabled, scissorsEnabled, antialiasedLineEnabled, depthBias, depthBiasClamp, depthBiasSlopedScaled, lineWidth);
            // case RenderAPI::Vulkan:    return std::make_shared<Vulkan_Device>();
            // So on and so forth...
        }

        std::cout << "Unknown Render API. Rasterizer State could not be created.\n";
        return nullptr;
    }

    std::shared_ptr<RHI_DepthStencilState> RHI_DepthStencilState::Create(const bool depthTestingEnabled, const bool depthWritingEnabled, const RHI_Comparison_Function depthComparisonFunction, const bool stencilTestingEnabled, const bool stencilWritingEnabled, const RHI_Comparison_Function stencilComparisonFunction, const RHI_Stencil_Operation stencilPassOperation, const RHI_Stencil_Operation stencilFailOperation, const RHI_Stencil_Operation stencilPassDepthFailOperation)
    {
        switch (Renderer::GetCurrentRenderAPI())
        {
            // case RenderAPI::DirectX11: return std::make_shared<DX11_Device>();
            case RenderAPI::DirectX12: return std::make_shared<DX12_DepthStencilState>(depthTestingEnabled, depthWritingEnabled, depthComparisonFunction, stencilTestingEnabled, stencilWritingEnabled, stencilComparisonFunction, stencilPassOperation, stencilFailOperation, stencilPassDepthFailOperation);
            // case RenderAPI::Vulkan:    return std::make_shared<Vulkan_Device>();
            // So on and so forth...
        }

        std::cout << "Unknown Render API. Depth Stencil State could not be created.\n";
        return nullptr;
    }

    std::shared_ptr<RHI_BlendState> RHI_BlendState::Create(const bool blendEnabled, const RHI_Blend_Function sourceBlendFunction, const RHI_Blend_Function destinationBlendFunction, const RHI_Blend_Operation blendOperation, const RHI_Blend_Function sourceBlendFunctionAlpha, const RHI_Blend_Function destinationBlendFunctionAlpha, const RHI_Blend_Operation blendOperationAlpha, const float blendFactor)
    {
        switch (Renderer::GetCurrentRenderAPI())
        {
            // case RenderAPI::DirectX11: return std::make_shared<DX11_Device>();
            case RenderAPI::DirectX12: return std::make_shared<DX12_BlendState>(blendEnabled, sourceBlendFunction, destinationBlendFunction, blendOperation, sourceBlendFunctionAlpha, destinationBlendFunctionAlpha, blendOperationAlpha, blendFactor);
            // case RenderAPI::Vulkan:    return std::make_shared<Vulkan_Device>();
            // So on and so forth...
        }

        std::cout << "Unknown Render API. Blend State could not be created.\n";
        return nullptr;
    }
}