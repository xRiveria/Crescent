#version 330 core
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BrightColor;

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
uniform bool selfCreatedPrimitive = false;
uniform bool bloomEnabled = true;

uniform sampler2D shadowMap;

in vec3 FragPosition;
in vec2 TexCoords;
in vec3 Normals;
in vec4 FragPosLightSpace;

in vec3 TangentLightPosition;
in vec3 TangentViewPosition;
in vec3 TangentFragPosition;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform sampler2D bloomBlur;

uniform float exposureAmount;
uniform sampler2D hdrBuffer;

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal)
{
    vec3 lightDir = normalize(pointLight.lightPosition - FragPosition);

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
    vec3 lightDirection;
    if (selfCreatedPrimitive)
    {
        lightDirection = normalize(light.lightPosition - fragmentPosition);
    }
    else
    {
        lightDirection = normalize(TangentLightPosition - fragmentPosition);
    }

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

    //Exposure
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposureAmount);

    if (bloomEnabled)
    {
        vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
        hdrColor += bloomColor; //Additive Blending
    }

    //Calculate Shadow
    float shadow = ShadowCalculation(FragPosLightSpace, normal);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (mapped + ambient + (1.0 - shadow) * (diffuse + specular));
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

    //Exposure
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposureAmount);
    if (bloomEnabled)
    { 
        vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
        hdrColor += bloomColor; //Additive Blending
    }

    return (mapped + ambient + diffuse + specular);
}

void main()
{
    vec3 normal;
    //Properties
    if (selfCreatedPrimitive)
    {
        normal = normalize(Normals);

        vec3 viewDirection = normalize(viewPosition - FragPosition); //Direction from the camera to the fragment.

        //Phase 1: Directional Lighting
        vec3 result = CalculateDirectionalLight(directionalLight, normal, viewDirection);

        //Phase 2: Point Light
        result += CalculatePointLight(pointLight, normal, FragPosition, viewDirection);

        //Check whether fragment output is higher than threshold. If so, output as brightness color.
        float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
        if (brightness > 1.0)
        {
            BrightColor = vec4(result, 1.0f);
        }
        else
        {
            BrightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }


        FragColor = vec4(result, 1.0f);
    }
    else
    {
        //Obtain normal from normal map in range [0, 1]
        normal = texture(texture_normal1, TexCoords).rgb;
        //Transform to [-1, 1] space.
        normal = normalize(normal * 2.0 - 1.0);

        vec3 viewDirection = normalize(TangentViewPosition - TangentFragPosition); //Direction from the camera to the fragment.

        //Phase 1: Directional Lighting
        vec3 result = CalculateDirectionalLight(directionalLight, normal, viewDirection);

        //Phase 2: Point Light
        result += CalculatePointLight(pointLight, normal, TangentFragPosition, viewDirection);

        //Check whether fragment output is higher than threshold. If so, output as brightness color.
        float brightness = dot(result.rgb, vec3(0.2126, 0.7152, 0.0722));
        if (brightness > 1.0)
        {
            BrightColor = vec4(result, 1.0f);
        }
        else
        {
            BrightColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        }


        FragColor = vec4(result, 1.0f);
    }
}