#version 330 core

out vec4 FragColor;

uniform vec3 viewPosition;

struct Material
{
    sampler2D diffuseMap;
    sampler2D specularMap;
    sampler2D emissionMap;
    //vec3 ambientColor;  //What color the surface reflects under ambient lighting, which is usually the same as the surface's color. 
    //vec3 diffuseColor;  //Defines the color of the surface under diffuse lighting. The diffuse color is usually also set to the desired surface's color. 
    vec3 specularColor; //Sets the color of the specular highlight on the surface, or possibly even reflect a surface-specific color. 
    float specularScatter; //The shininess impacts the scattering / radius of the specular highlight.
};

struct DirectionalLight
{
    vec3 lightDirection;

    vec3 ambientIntensity;
    vec3 diffuseIntensity;
    vec3 specularIntensity;
};

struct PointLight
{
    vec3 lightPosition;

    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;

    vec3 ambientIntensity;
    vec3 diffuseIntensity;
    vec3 specularIntensity;
};

struct SpotLight
{
    vec3 lightPosition;
    vec3 lightDirection;

    float innerLightCutoff;
    float outerLightCutoff;

    vec3 ambientIntensity;
    vec3 diffuseIntensity;
    vec3 specularIntensity;

    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
};

#define NR_POINT_LIGHTS 4

/*
struct Light
{

    //Directional Lights / Spotlights
    vec3 lightDirection;

    //Point Lights / Spotlights
    vec3 lightPosition;
    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;

    //Spotlights
    float cutoff;
    float outerCutoff;

    //Universal
    vec3 ambientIntensity;
    vec3 diffuseIntensity;
    vec3 specularIntensity;
};
*/

//uniform Light light;
uniform Material material;
uniform DirectionalLight directionalLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

in vec3 FragPosition;
in vec3 Normals;
in vec2 TexCoords;

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection)
{
    vec3 lightDirection = normalize(-light.lightDirection); //We inverse it so the direction goes from the fragment to the light source, instead of vice versa which people are more comfortable with.
    //Diffuse Shading
    float diff = max(dot(normal, lightDirection), 0.0);

    //Specular Shading
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.specularScatter);

    //Combine Results
    vec3 ambient = light.ambientIntensity * vec3(texture(material.diffuseMap, TexCoords));
    vec3 diffuse = light.diffuseIntensity * diff * vec3(texture(material.diffuseMap, TexCoords));
    vec3 specular = light.specularIntensity * spec * vec3(texture(material.specularMap, TexCoords));
    return (ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection)
{
    vec3 lightDirection = normalize(light.lightPosition - fragmentPosition); 
    //Diffuse Shading
    float diff = max(dot(normal, lightDirection), 0.0);

    //Specular Shading
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.specularScatter);
    
    //Attenuation
    float distance = length(light.lightPosition - fragmentPosition);
    float attenuation = 1.0 / (light.attenuationConstant + light.attenuationLinear * distance + light.attenuationQuadratic * (distance + distance));

    //Combine Results
    vec3 ambient = light.ambientIntensity * vec3(texture(material.diffuseMap, TexCoords));
    vec3 diffuse = light.diffuseIntensity * diff * vec3(texture(material.diffuseMap, TexCoords));
    vec3 specular = light.specularIntensity * spec * vec3(texture(material.specularMap, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

vec3 CalculateSpotLight(SpotLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection)
{
    vec3 ambient = light.ambientIntensity * texture(material.diffuseMap, TexCoords).rgb;
    
    //Diffuse
    vec3 lightDirection = normalize(light.lightPosition - fragmentPosition);
    float diff = max(dot(normal, lightDirection), 0.0f);
    vec3 diffuse = light.diffuseIntensity * diff * texture(material.diffuseMap, TexCoords).rgb;

    //Specular
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), material.specularScatter);
    vec3 specular = light.specularIntensity * spec * texture(material.specularMap, TexCoords).rgb;

    //Spotlight
    float theta = dot(lightDirection, normalize(-light.lightDirection));
    float epsilon = (light.innerLightCutoff - light.outerLightCutoff);
    float intensity = clamp((theta - light.outerLightCutoff) / epsilon, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;

    //Attenuation
    float distance = length(light.lightPosition - fragmentPosition);
    float attenuation = 1.0 / (light.attenuationConstant + light.attenuationLinear * distance + light.attenuationQuadratic * (distance * distance));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

void main()
{
    //Properties
    vec3 normal = normalize(Normals);
    vec3 viewDirection = normalize(viewPosition - FragPosition); //Direction from the camera to the fragment.

    //Phase 1: Directional Lighting
    vec3 result = CalculateDirectionalLight(directionalLight, normal, viewDirection);

    //Phase 2: Point Lights
    for (int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        result += CalculatePointLight(pointLights[i], normal, FragPosition, viewDirection);
    }

    //Phase 3: Spotlight
    result += CalculateSpotLight(spotLight, normal, FragPosition, viewDirection);

    FragColor = vec4(result , 1.0f);
}
