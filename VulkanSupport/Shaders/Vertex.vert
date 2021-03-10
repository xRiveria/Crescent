#version 450
#extension GL_ARB_seperate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
	mat4 modelMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main()
{
	gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPosition, 0.0, 1.0);
	fragColor = inColor; 
}