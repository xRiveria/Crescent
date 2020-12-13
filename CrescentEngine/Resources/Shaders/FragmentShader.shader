#version 330 core

out vec4 FragColor;

uniform vec3 viewPosition;

struct Material
{
    vec3 ambientColor;  //What color the surface reflects under ambient lighting, which is usually the same as the surface's color. 
    vec3 diffuseColor;  //Defines the color of the surface under diffuse lighting. The diffuse color is usually also set to the desired surface's color. 
    vec3 specularColor; //Sets the color of the specular highlight on the surface, or possibly even reflect a surface-specific color. 
    float specularScatter; //The shininess impacts the scattering / radius of the specular highlight.
};

struct Light
{
    vec3 lightPosition;
    vec3 ambientIntensity;
    vec3 diffuseIntensity;
    vec3 specularIntensity;
};

uniform Light light;
uniform Material material;

in vec3 FragPosition;
in vec3 Normals;

void main()
{
    //Ambient
    vec3 ambient = light.ambientIntensity * material.ambientColor;

    //Diffuse
    vec3 norm = normalize(Normals);
    vec3 lightDir = normalize(light.lightPosition - FragPosition);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuseIntensity * (diff * material.diffuseColor);

    //Specular
    vec3 viewDirection = normalize(viewPosition - FragPosition);
    vec3 reflectDirection = reflect(-lightDir, norm);
  
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.specularScatter);
    vec3 specular = light.specularIntensity * (spec * material.specularColor);

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
