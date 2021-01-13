#version 330 core
out vec4 FragColor;

in vec3 WorldPos;

#include ../Constants/Constants.shader
#include ../Constants/Sampling.shader

uniform sampler2D environment;

void main()
{
	vec2 uv = SampleSphericalMap(normalize(WorldPos)); //Transforms our sphere to cartesian.
	vec3 color = texture(environment, uv).rgb;
	FragColor = vec4(color, 1.0f);
}