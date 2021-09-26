#include "RHI_PCH.h"
#include "DX12_States.h"
#include "DX12_Context.h"
#include "DX12_Utilities.h"

namespace Aurora
{
    DX12_RasterizerState::DX12_RasterizerState(const RHI_Cull_Mode cullMode, const RHI_Fill_Mode fillMode, const bool counterClockwise, const bool depthClippingEnabled, const bool scissorsEnabled, const bool antialiasedLineEnabled, const float depthBias, const float depthBiasClamp, const float depthBiasSlopedScaled, const float lineWidth)
    {   
        // Save Properties
        m_Cull_Mode = cullMode;
        m_Fill_Mode = fillMode;
        m_FrontCounterClockwise = counterClockwise;
        m_DepthClipEnabled = depthClippingEnabled;
        m_ScissorsEnabled = scissorsEnabled;
        m_AntialiasedLineEnabled = antialiasedLineEnabled;
        m_DepthBias = depthBias;
        m_DepthBiasClamp = depthBiasClamp;
        m_DepthBiasSlopeScaled = depthBiasSlopedScaled;
        m_LineWidth = lineWidth;

        // Create Rasterizer Description. As no state object exists for DX12 for rasterizer states, we will save this description as our resource.
        D3D12_RASTERIZER_DESC rasterizerDescription = {};
        rasterizerDescription.CullMode = DX12_Utilities::ToDX12CullMode[cullMode];
        rasterizerDescription.FillMode = DX12_Utilities::ToDX12FillMode[fillMode];
        rasterizerDescription.FrontCounterClockwise = counterClockwise;
        rasterizerDescription.DepthBias = depthBias;
        rasterizerDescription.DepthBiasClamp = depthBiasClamp;
        rasterizerDescription.SlopeScaledDepthBias = depthBiasSlopedScaled;
        rasterizerDescription.MultisampleEnable = false; // Doesn't affect render targets with sample count greater than 1 for D3D Feature Level 10.1 or later.
        rasterizerDescription.AntialiasedLineEnable = antialiasedLineEnabled;
        rasterizerDescription.ForcedSampleCount = 0; // 0 indicates no sample count forcing. https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_rasterizer_desc
        rasterizerDescription.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

        m_Resource = static_cast<void*>(&rasterizerDescription);

        std::cout << "Successfully initialized DX12 Rasterizer State.\n";
        m_IsInitialized = true;
    }

    DX12_DepthStencilState::DX12_DepthStencilState(const bool depthTestingEnabled, const bool depthWritingEnabled, const RHI_Comparison_Function depthComparisonFunction, const bool stencilTestingEnabled, const bool stencilWritingEnabled, const RHI_Comparison_Function stencilComparisonFunction, const RHI_Stencil_Operation stencilPassOperation, const RHI_Stencil_Operation stencilFailOperation, const RHI_Stencil_Operation stencilPassDepthFailOperation)
    {
        // Save Properties
        m_DepthTestEnabled = depthTestingEnabled;
        m_DepthWritingEnabled = depthWritingEnabled;
        m_DepthComparisonFunction = depthComparisonFunction;
        m_StencilTestingEnabled = stencilTestingEnabled;
        m_StencilWritingEnabled = stencilWritingEnabled;
        m_StencilComparisonFunction = stencilComparisonFunction;
        m_StencilPassOperation = stencilPassOperation;
        m_StencilFailOperation = stencilFailOperation;
        m_StencilPassDepthFailOperation = stencilPassDepthFailOperation;
        
        // Create
        D3D12_DEPTH_STENCIL_DESC depthStencilDescription = {};
        // Depth Test
        depthStencilDescription.DepthEnable = static_cast<BOOL>(depthTestingEnabled || depthWritingEnabled);
        depthStencilDescription.DepthWriteMask = depthWritingEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        depthStencilDescription.DepthFunc = DX12_Utilities::ToDX12ComparisonFunction[static_cast<uint32_t>(depthComparisonFunction)];
        // Stencil Test
        depthStencilDescription.StencilEnable = static_cast<BOOL>(stencilTestingEnabled || stencilWritingEnabled);
        depthStencilDescription.StencilReadMask = stencilTestingEnabled  ? m_StencilReadMask : 0;
        depthStencilDescription.StencilWriteMask = stencilWritingEnabled ? m_StencilWriteMask : 0;
        // Stencil Operations if Pixel is Front Facing
        depthStencilDescription.FrontFace.StencilFailOp = DX12_Utilities::ToDX12StencilOperation[static_cast<uint32_t>(stencilFailOperation)];
        depthStencilDescription.FrontFace.StencilPassOp = DX12_Utilities::ToDX12StencilOperation[static_cast<uint32_t>(stencilPassOperation)];
        depthStencilDescription.FrontFace.StencilDepthFailOp = DX12_Utilities::ToDX12StencilOperation[static_cast<uint32_t>(stencilPassDepthFailOperation)];
        depthStencilDescription.FrontFace.StencilFunc = DX12_Utilities::ToDX12ComparisonFunction[static_cast<uint32_t>(stencilComparisonFunction)];
        // Stencil Operations if Pixel is Back Facing
        depthStencilDescription.BackFace = depthStencilDescription.FrontFace;

        m_Resource = static_cast<void*>(&depthStencilDescription);

        std::cout << "Successfully initialized DX12 Depth Stencil State.\n";
        m_IsInitialized = true;
    }

    DX12_BlendState::DX12_BlendState(const bool blendEnabled, const RHI_Blend_Function sourceBlendFunction, const RHI_Blend_Function destinationBlendFunction, const RHI_Blend_Operation blendOperation, const RHI_Blend_Function sourceBlendFunctionAlpha, const RHI_Blend_Function destinationBlendFunctionAlpha, const RHI_Blend_Operation blendOperationAlpha, const float blendFactor)
    {
        // Save Properties
        m_BlendingEnabled = blendEnabled;
        m_SourceBlendFunction = sourceBlendFunction;
        m_DestinationBlendFunction = destinationBlendFunction;
        m_BlendOperation = blendOperation;
        m_SourceBlendFunctionAlpha = sourceBlendFunctionAlpha;
        m_DestinationBlendFunctionAlpha = destinationBlendFunctionAlpha;
        m_BlendOperationAlpha = blendOperationAlpha;
        m_BlendFactor = blendFactor;

        D3D12_BLEND_DESC blendStateDescription = {};
        blendStateDescription.RenderTarget[0].BlendEnable = blendEnabled;
        blendStateDescription.RenderTarget[0].SrcBlend = DX12_Utilities::ToDX12BlendFunction[static_cast<uint32_t>(sourceBlendFunction)];
        blendStateDescription.RenderTarget[0].DestBlend = DX12_Utilities::ToDX12BlendFunction[static_cast<uint32_t>(destinationBlendFunction)];
        blendStateDescription.RenderTarget[0].BlendOp = DX12_Utilities::ToDX12BlendOperation[static_cast<uint32_t>(blendOperation)];
        blendStateDescription.RenderTarget[0].SrcBlendAlpha = DX12_Utilities::ToDX12BlendFunction[static_cast<uint32_t>(sourceBlendFunctionAlpha)];
        blendStateDescription.RenderTarget[0].DestBlendAlpha = DX12_Utilities::ToDX12BlendFunction[static_cast<uint32_t>(destinationBlendFunctionAlpha)];
        blendStateDescription.RenderTarget[0].BlendOpAlpha = DX12_Utilities::ToDX12BlendOperation[static_cast<uint32_t>(blendOperationAlpha)];
        blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        blendStateDescription.AlphaToCoverageEnable = false;
        blendStateDescription.IndependentBlendEnable = false;

        m_Resource = static_cast<void*>(&blendStateDescription);

        std::cout << "Successfully initialized DX12 Blend State.\n";
        m_IsInitialized = true;
    }
}