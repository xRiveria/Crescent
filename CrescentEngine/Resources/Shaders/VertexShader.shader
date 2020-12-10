#version 330 core

layout (location = 0) in vec3 inputPosition;
layout(location = 1)  in vec3 inputNormals;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 Normals;
out vec3 FragPosition;

void main()
{
	gl_Position = projection * view * model * vec4(inputPosition, 1.0f);
	FragPosition = vec3(model * vec4(inputPosition, 1.0));
	Normals = inputNormals;
}