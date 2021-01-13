#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aTexCoords;
layout (location = 2) in vec3 aNormal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec3 WorldPos;

void main()
{
	vec4 worldPosition = model * vec4(aPos, 1.0f);
	WorldPos.xyz = worldPosition.xyz;

	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView * model * worldPosition;

	gl_Position = clipPos.xyww; 
}