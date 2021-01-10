#version 420 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;

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
    vec4 albedoAO = texture(gAlbedo, TexCoords);
    vec4 normalRoughness = texture(gNormal, TexCoords);
    vec4 positionMetallic = texture(gPosition, TexCoords);  //Metallic, Roughness and AO are for future support.

    vec3 worldPosition = positionMetallic.xyz;
    vec3 albedo = albedoAO.xyz;
    vec3 normal = normalRoughness.xyz;
    //float roughness = normalRoughness.a;
    //float metallic = positionMetallic.a;

    //Lighting Input
    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPosition.xyz - worldPosition);
    vec3 L = normalize(-lightDirection);
    vec3 H = normalize(V + L);

    //Light Shadow
    vec4 fragPosLightSpace = lightShadowViewProjection * vec4(worldPosition, 1.0);
    float shadow = ShadowFactor(lightShadowMap, fragPosLightSpace, N, L);

    //Lighting 


    FragColor.rgb = vec3(shadow);
    FragColor.a = 1.0f;
}