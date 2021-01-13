#version 330 core
out vec4 FragColor;

in vec3 WorldPos;

#include ../Constants/Constants.shader
#include ../Constants/Sampling.shader

uniform samplerCube environment;
uniform float roughness;

void main(void)
{
	//The world vector acts as the normal of a tangent surface from the origin, aligned to vWorld. Given this normal, calculate all incoming radiance of the environment.
    vec3 N = normalize(WorldPos);

    //Approximate view/reflection angle as equal to N (Unreal; Split-Sum)
    vec3 R = N;
    vec3 V = N;

    const uint SAMPLE_COUNT = 4096u * 4u;
    vec3 prefilteredColor = vec3(0.0);
    float totalWeight = 0.0;

    //For Debugging, make scope visible to end part.
    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        //Generates a sample vector that's biased towards the preferred alignment direction (importance sampling).
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        vec3 H = ImportanceSampleGGX(Xi, N, roughness);
        vec3 L = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(dot(N, L), 0.0);

        if (NdotL > 0.0)
        {
            //Mote that HDR environment maps are loaded linearly, so there no need for linearizing first.
            prefilteredColor += texture(environment, L).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    prefilteredColor = prefilteredColor / totalWeight;
    FragColor = vec4(prefilteredColor, 1.0);
}