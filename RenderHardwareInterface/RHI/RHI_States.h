#pragma once
#include "RHI_Utilities.h"

namespace Aurora
{
    class RHI_RasterizerState
    {
    public:
        RHI_RasterizerState() = default;
        virtual ~RHI_RasterizerState() = default;

    public:
        bool IsInitialized() const { return m_IsInitialized; }
        static std::shared_ptr<RHI_RasterizerState> Create(const RHI_Cull_Mode cullMode,
                                                           const RHI_Fill_Mode fillMode,
                                                           const bool counterClockwise,
                                                           const bool depthClippingEnabled,
                                                           const bool scissorsEnabled,
                                                           const bool antialiasedLineEnabled,
                                                           const float depthBias                = 0.0f,
                                                           const float depthBiasClamp           = 0.0f,
                                                           const float depthBiasSlopedScaled    = 0.0f,
                                                           const float lineWidth                = 0.0f);

    public:
        // Properties
        RHI_Cull_Mode m_Cull_Mode      = RHI_Cull_Mode::RHI_Cull_None;
        RHI_Fill_Mode m_Fill_Mode      = RHI_Fill_Mode::RHI_Fill_Solid;
        bool m_FrontCounterClockwise   = false;                          // Pixel ordering.
        float m_DepthBias              = 0.0f;                           // Depth value added to a given pixel.
        float m_DepthBiasClamp         = 0.0f;                           // Maximum depth bias of a pixel. 
        float m_DepthBiasSlopeScaled   = 0.0f;                           // Scalar on a given pixel's slope.
        bool m_DepthClipEnabled        = false;
        bool m_AntialiasedLineEnabled  = false;
        bool m_ScissorsEnabled         = false;
        float m_LineWidth = 0.0f;

    protected:
        bool m_IsInitialized = false;

        // API
        void* m_Resource = nullptr;
    };

    class RHI_DepthStencilState
    {
    public:
        RHI_DepthStencilState() = default;
        virtual ~RHI_DepthStencilState() = default;

        bool IsInitialized() const { return m_IsInitialized; }
        static std::shared_ptr<RHI_DepthStencilState> Create(const bool depthTestingEnabled = true,
                                                             const bool depthWritingEnabled = true, 
                                                             const RHI_Comparison_Function depthComparisonFunction     = RHI_Comparison_Function::RHI_Comparison_Function_Less_Equal,
                                                             const bool stencilTestingEnabled = false,
                                                             const bool stencilWritingEnabled = false,
                                                             const RHI_Comparison_Function stencilComparisonFunction   = RHI_Comparison_Function::RHI_Comparison_Function_Always,
                                                             const RHI_Stencil_Operation stencilPassOperation          = RHI_Stencil_Operation::RHI_Stencil_Operation_Replace,
                                                             const RHI_Stencil_Operation stencilFailOperation          = RHI_Stencil_Operation::RHI_Stencil_Operation_Keep,
                                                             const RHI_Stencil_Operation stencilPassDepthFailOperation = RHI_Stencil_Operation::RHI_Stencil_Operation_Keep);
           
    public:
        // Properties - Depth
        bool m_DepthTestEnabled = false;
        bool m_DepthWritingEnabled = false;
        RHI_Comparison_Function m_DepthComparisonFunction = RHI_Comparison_Function::RHI_Comparison_Function_Never;  // Function that compares depth data against existing depth data.
        
        // Properties - Stencil
        bool m_StencilTestingEnabled = false;
        bool m_StencilWritingEnabled = false;
        // 8 bits per stencil value, 256 stencil values per pixel.
        uint8_t m_StencilReadMask = 1;
        uint8_t m_StencilWriteMask = 1;
        RHI_Comparison_Function m_StencilComparisonFunction     = RHI_Comparison_Function::RHI_Comparison_Function_Never;     // The operation that compares 
        RHI_Stencil_Operation m_StencilPassOperation            = RHI_Stencil_Operation::RHI_Stencil_Operation_Replace;       // The operation to perform when stencil testing and depth testing both pass.
        RHI_Stencil_Operation m_StencilFailOperation            = RHI_Stencil_Operation::RHI_Stencil_Operation_Keep;          // The operation to perform when stencil testing fails.
        RHI_Stencil_Operation m_StencilPassDepthFailOperation   = RHI_Stencil_Operation::RHI_Stencil_Operation_Keep;          // The stencil operation to perform when stencil testing passes and depth testing fails.

    protected:
        bool m_IsInitialized = false;
        
        // API
        void* m_Resource = nullptr;
    };

    class RHI_BlendState
    {
    public:
        RHI_BlendState() = default;
        virtual ~RHI_BlendState() = default;

        bool IsInitialized() const { return m_IsInitialized; }
        static std::shared_ptr<RHI_BlendState> Create(const bool blendEnabled = false,
                                                      const RHI_Blend_Function sourceBlendFunction           = RHI_Blend_Function::RHI_Blend_Function_Source_Alpha,
                                                      const RHI_Blend_Function destinationBlendFunction      = RHI_Blend_Function::RHI_Blend_Function_Inverse_Source_Alpha,
                                                      const RHI_Blend_Operation blendOperation               = RHI_Blend_Operation::RHI_Blend_Operation_Add,
                                                      const RHI_Blend_Function sourceBlendFunctionAlpha      = RHI_Blend_Function::RHI_Blend_Function_One,
                                                      const RHI_Blend_Function destinationBlendFunctionAlpha = RHI_Blend_Function::RHI_Blend_Function_One,
                                                      const RHI_Blend_Operation blendOperationAlpha          = RHI_Blend_Operation::RHI_Blend_Operation_Add,
                                                      const float blendFactor                                = 0.0f);

    public:
        bool m_BlendingEnabled = false;
        RHI_Blend_Function m_SourceBlendFunction             = RHI_Blend_Function::RHI_Blend_Function_Source_Alpha;
        RHI_Blend_Function m_DestinationBlendFunction        = RHI_Blend_Function::RHI_Blend_Function_Inverse_Source_Alpha;
        RHI_Blend_Operation m_BlendOperation                 = RHI_Blend_Operation::RHI_Blend_Operation_Add;
        RHI_Blend_Function m_SourceBlendFunctionAlpha        = RHI_Blend_Function::RHI_Blend_Function_One;
        RHI_Blend_Function m_DestinationBlendFunctionAlpha   = RHI_Blend_Function::RHI_Blend_Function_One;
        RHI_Blend_Operation m_BlendOperationAlpha            = RHI_Blend_Operation::RHI_Blend_Operation_Add;
        float m_BlendFactor = 1.0f;

    protected:
        bool m_IsInitialized = false;

        // API
        void* m_Resource = nullptr;
    };
}