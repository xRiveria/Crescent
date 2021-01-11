#version 420 core
out vec4 FragColor;

in vec3 FragPos;
in vec4 ScreenPos;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightRadius;

uniform vec3 cameraPosition;

//Include These
const float PI = 3.14159265359;
const float TAU = 6.2831853071;

// GGX/Throwbridge-Reitz
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// ----------------------------------------------------------------------------
// TODO(Joey): get proper names of the options/functions
float GeometryGGXSchlick(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
// NOTE(Joey): for IBL we use a different k (see unreal course notes)
float GeometryGGXSchlickIBL(float NdotV, float roughness)
{
    float a = roughness * roughness;
    float k = a / 2.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometryGGX(float NdotV, float NdotL, float roughness)
{
    float ggx2 = GeometryGGXSchlick(NdotV, roughness);
    float ggx1 = GeometryGGXSchlick(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
float GeometryGGXIBL(float NdotV, float NdotL, float roughness)
{
    float ggx1 = GeometryGGXSchlickIBL(NdotV, roughness);
    float ggx2 = GeometryGGXSchlickIBL(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------
vec3 FresnelSphericalGaussian(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(2.0, (-5.55473 * cosTheta - 6.98316 * cosTheta));
}

void main()
{
    vec2 UV = (ScreenPos.xy / ScreenPos.w) * 0.5 + 0.5;

    vec4 albedoAO = texture(gAlbedo, UV);
    vec4 normalRoughness = texture(gNormal, UV);
    vec4 positionMetallic = texture(gPosition, UV);  //Metallic, Roughness and AO are for future support.

    vec3 worldPosition = positionMetallic.xyz;
    vec3 albedo = albedoAO.rgb;
    vec3 normal = normalRoughness.rgb;
    float roughness = 0.5f;
    float metallic = 0.5f;

    //Lighting Input
    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPosition.xyz - worldPosition);
    vec3 L = normalize(lightPosition - worldPosition);
    vec3 H = normalize(V + L);

    vec3 F0 = vec3(0.04f);
    F0 = mix(F0, albedo, metallic);

    //Calculate Light Radiance (Based on UE4's Light Attenuation Model)
    float distance = length(worldPosition - lightPosition);
    float attenuation = pow(clamp(1.0 - pow(distance / lightRadius, 1.0), 0.0, 1.0), 2.0) / (distance * distance + 1.0);
    vec3 radiance = lightColor * attenuation;

    // cook-torrance brdf
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometryGGX(max(dot(N, V), 0.0), max(dot(N, L), 0.0), roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 nominator = NDF * G * F;
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
    vec3 specular = nominator / denominator;

    // add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    FragColor.rgb = Lo;
    FragColor.a = 1.0;
}