#version 420 core
out vec4 FragColor;

in vec2 UV;

uniform sampler2D TexAlbedo;

void main()
{
    vec4 diffuseColor = texture(TexAlbedo, UV);
    FragColor = diffuseColor;
}