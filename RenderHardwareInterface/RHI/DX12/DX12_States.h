#pragma once
#include "../RHI/RHI_States.h"

namespace Aurora
{
    class DX12_RasterizerState : public RHI_RasterizerState
    {
    public:
        DX12_RasterizerState(const RHI_Cull_Mode cullMode,
                             const RHI_Fill_Mode fillMode,
                             const bool counterClockwise,
                             const bool depthClippingEnabled,
                             const bool scissorsEnabled,
                             const bool antialiasedLineEnabled,
                             const float depthBias               = 0.0f,
                             const float depthBiasClamp          = 0.0f,
                             const float depthBiasSlopedScaled   = 0.0f,
                             const float lineWidth               = 0.0f);

        virtual ~DX12_RasterizerState() override {};
    };

    class DX12_DepthStencilState : public RHI_DepthStencilState
    {
    public:
        DX12_DepthStencilState(const bool depthTestingEnabled = true,
                               const bool depthWritingEnabled = true,
                               const RHI_Comparison_Function depthComparisonFunction     = RHI_Comparison_Function::RHI_Comparison_Function_Less_Equal,
                               const bool stencilTestingEnabled = false,
                               const bool stencilWritingEnabled = false,
                               const RHI_Comparison_Function stencilComparisonFunction   = RHI_Comparison_Function::RHI_Comparison_Function_Always,
                               const RHI_Stencil_Operation stencilPassOperation          = RHI_Stencil_Operation::RHI_Stencil_Operation_Replace,
                               const RHI_Stencil_Operation stencilFailOperation          = RHI_Stencil_Operation::RHI_Stencil_Operation_Keep,
                               const RHI_Stencil_Operation stencilPassDepthFailOperation = RHI_Stencil_Operation::RHI_Stencil_Operation_Keep);

        virtual ~DX12_DepthStencilState() override {};
    };

    class DX12_BlendState : public RHI_BlendState
    {
    public:
        DX12_BlendState(const bool blendEnabled = false,
                        const RHI_Blend_Function sourceBlendFunction           = RHI_Blend_Function::RHI_Blend_Function_Source_Alpha,
                        const RHI_Blend_Function destinationBlendFunction      = RHI_Blend_Function::RHI_Blend_Function_Inverse_Source_Alpha,
                        const RHI_Blend_Operation blendOperation               = RHI_Blend_Operation::RHI_Blend_Operation_Add,
                        const RHI_Blend_Function sourceBlendFunctionAlpha      = RHI_Blend_Function::RHI_Blend_Function_One,
                        const RHI_Blend_Function destinationBlendFunctionAlpha = RHI_Blend_Function::RHI_Blend_Function_One,
                        const RHI_Blend_Operation blendOperationAlpha          = RHI_Blend_Operation::RHI_Blend_Operation_Add,
                        const float blendFactor = 0.0f);

        virtual ~DX12_BlendState() override {};
    };
}