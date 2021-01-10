#version 420 core
layout (location = 0) in vec3 aPos;

out vec3 FragPos;
out vec4 ScreenPos;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0f));
	ScreenPos = projection * view * vec4(FragPos, 1.0f);

	gl_Position = ScreenPos;
}