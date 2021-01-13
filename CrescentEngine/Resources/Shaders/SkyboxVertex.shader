#version 420 core
layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 view;

out vec3 WorldPos;

void main()
{
	WorldPos = aPos;

	mat4 rotView = mat4(mat3(view)); //Strip translation so the cube doesn't move with our player.
	vec4 clipPos = projection * rotView * vec4(WorldPos, 1.0f);

	gl_Position = clipPos.xyww;
}