struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

cbuffer SceneConstantBuffer : register(b0)
{
    float4 offset;
    float4 color;
    float4 padding[14];
}

Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

PSInput VSMain(float4 position : POSITION, float4 texCoord : TEXCOORD)
{
    PSInput result;
    result.position = position + offset;
    result.uv = texCoord;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return color;
}