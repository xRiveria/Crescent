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

void main()
{
    vec2 UV = (ScreenPos.xy / ScreenPos.w) * 0.5 + 0.5;

    vec4 albedoAO = texture(gAlbedo, UV);
    vec4 normalRoughness = texture(gNormal, UV);
    vec4 positionMetallic = texture(gPosition, UV);  //Metallic, Roughness and AO are for future support.

    vec3 worldPosition = positionMetallic.xyz;
    vec3 albedo = albedoAO.rgb;
    vec3 normal = normalRoughness.rgb;
    //float roughness = normalRoughness.a;
    //float metallic = positionMetallic.a;

    //Calculate Light Radiance (Based on UE4's Light Attenuation Model)
    float distance = length(worldPosition - lightPosition);
    float attenuation = pow(clamp(1.0 - pow(distance / lightRadius, 1.0), 0.0, 1.0), 2.0) / (distance * distance + 1.0);
    vec3 radiance = lightColor * attenuation;

    //Further Calculations
    FragColor.rgb = radiance;
    FragColor.a = 1.0f;
}