#version 420 core
out vec4 FragColor;

in vec2 TexCoords;

#include ../Constants/Constants.shader
#include ../Constants/BRDF.shader

uniform sampler2D gPositionMetallic;
uniform sampler2D gNormalRoughness;
uniform sampler2D gAlbedoAO;

uniform vec3 lightDirection;
uniform vec3 lightColor;

uniform vec3 cameraPosition;

uniform sampler2D lightShadowMap;
uniform mat4 lightShadowViewProjection;
uniform bool ShadowsEnabled;

float ShadowFactor(sampler2D shadowMap, vec4 fragPosLightSpace, vec3 N, vec3 L)
{
    if (ShadowsEnabled)
    {
        // perspective divide
        vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
        // transform to [0,1] range
        projCoords = projCoords * 0.5 + 0.5;
        // closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
        float closestDepth = texture(shadowMap, projCoords.xy).r;
        // depth of current fragment from light's perspective
        float currentDepth = projCoords.z;
        // shadow bias
        float bias = max(0.05 * (1.0 - dot(N, L)), 0.005);
        // PCF
        float shadow = 0.0;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        for (int x = -2; x <= 2; ++x)
        {
            for (int y = -2; y <= 2; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
            }
        }
        shadow /= 25.0;
        // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
        if (projCoords.z > 1.0)
            shadow = 0.0;

        return shadow;
    }
    else
    {
        return 1.0;
    }
}

void main()
{
    vec4 albedoAO = texture(gAlbedoAO, TexCoords);
    vec4 normalRoughness = texture(gNormalRoughness, TexCoords);
    vec4 positionMetallic = texture(gPositionMetallic, TexCoords);

    vec3 worldPos = positionMetallic.xyz;
    vec3 albedo = albedoAO.rgb;
    vec3 normal = normalRoughness.rgb;
    float roughness = normalRoughness.a;
    float metallic = positionMetallic.a;

    // lighting input
    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPosition.xyz - worldPos); // view-space camera is (0, 0, 0): (0, 0, 0) - viewPos = -viewPos
    vec3 L = normalize(-lightDirection);
    vec3 H = normalize(V + L);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // calculate light radiance    
    vec3 radiance = lightColor;

    // light shadow
    vec4 fragPosLightSpace = lightShadowViewProjection * vec4(worldPos, 1.0);
    float shadow = ShadowFactor(lightShadowMap, fragPosLightSpace, N, L);

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
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL * (1.0 - shadow);

    FragColor.rgb = Lo;
    // FragColor.rgb = vec3(shadow);
    FragColor.a = 1.0;
}