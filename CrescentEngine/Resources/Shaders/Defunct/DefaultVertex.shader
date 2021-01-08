#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;

out vec2 UV;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main()
{
	UV = aUV;
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}