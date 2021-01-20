#version 420 core
out vec4 FragColor;

in vec3 FragPos;
in vec4 ScreenPos;

#include ../Constants/Constants.shader
#include ../Constants/BRDF.shader

uniform sampler2D gPositionMetallic;
uniform sampler2D gNormalRoughness;
uniform sampler2D gAlbedoAO;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightRadius;

uniform vec3 cameraPosition;

void main()
{
    vec2 UV = (ScreenPos.xy / ScreenPos.w) * 0.5 + 0.5;

    vec4 albedoAO = texture(gAlbedoAO, UV);
    vec4 normalRoughness = texture(gNormalRoughness, UV);
    vec4 positionMetallic = texture(gPositionMetallic, UV);  //Metallic, Roughness and AO are for future support.

    vec3 worldPosition = positionMetallic.xyz;
    vec3 albedo = albedoAO.rgb;
    vec3 normal = normalRoughness.rgb;
    float roughness = normalRoughness.a;
    float metallic = positionMetallic.a;

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