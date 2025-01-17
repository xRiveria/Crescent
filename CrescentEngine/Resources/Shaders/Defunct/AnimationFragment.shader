#version 330 core
out vec4 FragColor;

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

struct DirectionalLight
{
    vec3 lightDirection;

    vec3 ambientIntensity;
    vec3 diffuseIntensity;
    vec3 specularIntensity;
};

uniform vec3 viewPosition;
//uniform SpotLight spotLight;
uniform PointLight pointLight;
uniform DirectionalLight directionalLight;

in vec3 FragPosition;
in vec2 TexCoords;
in vec3 Normals;

uniform float exposureAmount;
uniform sampler2D hdrBuffer;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection)
{
    vec3 lightDirection = normalize(light.lightPosition - fragmentPosition);
    //Diffuse Shading
    float diff = max(dot(normal, lightDirection), 0.0);

    //Specular Shading
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32.0f);

    //Attenuation
    float distance = length(light.lightPosition - fragmentPosition);
    float attenuation = 1.0 / (light.attenuationConstant + light.attenuationLinear * distance + light.attenuationQuadratic * (distance + distance));

    //Combine Results
    vec3 ambient = light.ambientIntensity * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuseIntensity * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specularIntensity * spec * vec3(texture(texture_specular1, TexCoords));

    //Exposure
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposureAmount);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (mapped + ambient + diffuse + specular);
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection)
{
    vec3 lightDirection = normalize(-light.lightDirection); //We inverse it so the direction goes from the fragment to the light source, instead of vice versa which people are more comfortable with.
    //Diffuse Shading
    float diff = max(dot(normal, lightDirection), 0.0);

    //Specular Shading
    vec3 reflectDirection = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32.0f);

    //Exposure
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposureAmount);

    //Combine Results
    vec3 ambient = light.ambientIntensity * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuseIntensity * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specularIntensity * spec * vec3(texture(texture_specular1, TexCoords));

    return (mapped + ambient + diffuse + specular);
}

void main()
{
    //Properties
    vec3 normal = normalize(Normals);
    vec3 viewDirection = normalize(viewPosition - FragPosition); //Direction from the camera to the fragment.

    //Phase 1: Directional Lighting
    vec3 result = CalculateDirectionalLight(directionalLight, normal, viewDirection);

    //Phase 2: Point Light
    result += CalculatePointLight(pointLight, normal, FragPosition, viewDirection);

    FragColor = vec4(result, 1.0f);
}