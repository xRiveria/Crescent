#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;

out vec2 UV;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main()
{
	UV = aUV;
	FragPos = vec3(model * vec4(aPos, 1.0));
	gl_Position = projection * view * model * vec4(aPos, 1.0f);
}