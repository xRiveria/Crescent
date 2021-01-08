#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform bool refractOn = true;

void main()
{
    vec3 I = normalize(Position - cameraPos);
    vec3 R;

    if (refractOn == true)
    {
        float ratio = 1.00 / 1.52;
        R = refract(I, normalize(Normal), ratio);
    }
    else
    {
        R = reflect(I, normalize(Normal));
    }

    FragColor = vec4(texture(skybox, R).rgb, 1.0);
}