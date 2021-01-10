#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 lightSpaceProjection;
uniform mat4 lightSpaceView;
uniform mat4 model;

void main()
{
	gl_Position = lightSpaceProjection * lightSpaceView * model * vec4(aPos, 1.0f);
}