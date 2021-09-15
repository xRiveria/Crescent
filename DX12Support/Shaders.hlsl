struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

PSInput VSMain(float4 position : POSITION, float4 texCoord : TEXCOORD)
{
    PSInput result;
    result.position = position;
    result.uv = texCoord;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return g_Texture.Sample(g_Sampler, input.uv);
}