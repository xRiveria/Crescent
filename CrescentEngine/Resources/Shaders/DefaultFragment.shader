#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 UV;

uniform sampler2D TexAlbedo;

void main()
{
    vec3 diffuseColor = texture(TexAlbedo, UV);
    FragColor = vec4(diffuseColor, 1.0f);
}