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
uniform bool blinn = true;
uniform bool softShadows = true;
uniform float pcfSampleAmount = 15.0f;

uniform sampler2D shadowMap;

in vec3 FragPosition;
in vec2 TexCoords;
in vec3 Normals;
in vec4 FragPosLightSpace;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    vec3 lightDir = normalize(pointLight.lightPosition - FragPosition);
    vec3 normal = normalize(Normals);

    //Perform Perspective Divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    //Transform to [0, 1] range.
    projCoords = projCoords * 0.5 + 0.5; 
    //Get closest depth value from light's perspective, using [0 ,1] range from fragPosLight as coords.
    float closestDepth = texture(shadowMap, projCoords.xy).r;
    //Get depth of current fragment from light's perspective.
    float currentDepth = projCoords.z;
    //Check whether current fragment position is in shadow.
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

    float shadow = 0.0f;

    if (!softShadows)
    {
        shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    }
    else
    {

        //PCF
        shadow = 0.0f;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        for (int x = -1; x <= 1; ++x)
        {
            for (int y = -1; y <= 1; ++y)
            {
                float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
                shadow += currentDepth - bias > pcfDepth ? 1.0f : 0.0f;
            }
        }

        shadow /= pcfSampleAmount;
    }

    if (projCoords.z > 1.0f)
    {
        shadow = 0.0f;
    }

    return shadow;
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection)
{
    vec3 lightDirection = normalize(light.lightPosition - fragmentPosition);
    //Diffuse Shading
    float diff = max(dot(normal, lightDirection), 0.0);

    //Specular Shading
    float spec = 0.0f;
    if (blinn)
    {
        vec3 halfwayDirection = normalize(lightDirection + viewDirection);
        spec = pow(max(dot(normal, halfwayDirection), 0.0), 16.0f);
    }
    else
    {
        vec3 reflectDirection = reflect(-lightDirection, normal);
        spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32.0f);
    }

    //Attenuation
    float distance = length(light.lightPosition - fragmentPosition);
    float attenuation = 1.0 / (light.attenuationConstant + light.attenuationLinear * distance + light.attenuationQuadratic * (distance + distance));

    //Combine Results
    vec3 ambient = light.ambientIntensity * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuseIntensity * diff * vec3(texture(texture_diffuse1, TexCoords));

    vec3 specular = light.specularIntensity * spec * vec3(texture(texture_specular1, TexCoords));

    //Calculate Shadow
    float shadow = ShadowCalculation(FragPosLightSpace);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;


    return (ambient + (1.0 - shadow) * (diffuse + specular));
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection)
{
    vec3 lightDirection = normalize(-light.lightDirection); //We inverse it so the direction goes from the fragment to the light source, instead of vice versa which people are more comfortable with.
    //Diffuse Shading
    float diff = max(dot(normal, lightDirection), 0.0);

    //Specular Shading
    float spec = 0.0f;
    if (blinn)
    {
        vec3 halfwayDirection = normalize(lightDirection + viewDirection);
        spec = pow(max(dot(normal, halfwayDirection), 0.0), 16.0f);
    }
    else
    {
        vec3 reflectDirection = reflect(-lightDirection, normal);
        spec = pow(max(dot(viewDirection, reflectDirection), 0.0), 32.0f);
    }

    //Combine Results
    vec3 ambient = light.ambientIntensity * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuseIntensity * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specularIntensity * spec * vec3(texture(texture_specular1, TexCoords));
    return (ambient + diffuse + specular);
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