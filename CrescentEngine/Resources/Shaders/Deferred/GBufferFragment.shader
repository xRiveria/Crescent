#version 420 core
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;

in vec2 UV;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D TexAlbedo;

void main()
{
	gPosition = vec4(FragPos, 1.0f);
	gNormal = vec4(Normal, 1.0f);
	gAlbedo = texture(TexAlbedo, UV);
}