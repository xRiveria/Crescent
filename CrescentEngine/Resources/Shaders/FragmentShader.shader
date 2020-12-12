#version 330 core

out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 viewPosition;
uniform float ambientStrength;
uniform float specularStrength;

in vec3 FragPosition;
in vec3 Normals;

void main()
{
    vec3 ambient = ambientStrength * lightColor;

    vec3 norm = normalize(Normals);
    vec3 lightDir = normalize(lightPosition - FragPosition);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 viewDirection = normalize(viewPosition - FragPosition);
    vec3 reflectDirection = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor; 
    FragColor = vec4(result, 1.0);
}
