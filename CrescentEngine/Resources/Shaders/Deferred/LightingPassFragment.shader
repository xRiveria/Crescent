#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec4 FragPosLightSpace;
in vec3 TangentLightPosition;
in vec3 TangentViewPosition;
in vec3 TangentFragPosition;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpecular;
uniform sampler2D shadowMap;

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
uniform PointLight pointLight;
uniform DirectionalLight directionalLight;

uniform bool blinn = true;
uniform bool softShadows = true;
uniform float pcfSampleAmount = 15.0f;
uniform float exposureAmount;
uniform sampler2D hdrBuffer;


float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal, vec3 FragPosition)
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

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection, vec3 diffuseColor, vec3 specularColor)
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
    vec3 ambient = light.ambientIntensity * diffuseColor;
    vec3 diffuse = light.diffuseIntensity * diffuseColor;
    vec3 specular = light.specularIntensity * spec * specularColor;

    //Exposure
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposureAmount);

    return (mapped + ambient + diffuse + specular);
}

vec3 CalculatePointLight(PointLight light, vec3 normal, vec3 fragmentPosition, vec3 viewDirection, vec3 diffuseColor, vec3 specularColor)
{
    vec3 lightDirection;
    // if (selfCreatedPrimitive)
   //  {
    lightDirection = normalize(light.lightPosition - fragmentPosition);
    //  }
     // else
      //{
          //lightDirection = normalize(TangentLightPosition - fragmentPosition);
     // }

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
    vec3 ambient = light.ambientIntensity * diffuseColor;
    vec3 diffuse = light.diffuseIntensity * diff * diffuseColor;
    vec3 specular = light.specularIntensity * spec * specularColor;

    //Exposure
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposureAmount);

    //Calculate Shadow
    float shadow = ShadowCalculation(FragPosLightSpace, normal, fragmentPosition);

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (mapped + ambient + (1.0 - shadow) * (diffuse + specular));
}

const int NR_LIGHTS = 1;

void main()
{
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpecular, TexCoords).rgb;
    float Specular = texture(gAlbedoSpecular, TexCoords).a;

    // then calculate lighting as usual
    vec3 lighting = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir = normalize(viewPosition - FragPos);
    for (int i = 0; i < NR_LIGHTS; ++i)
    {
        // diffuse
        vec3 lightDir = normalize(pointLight.lightPosition - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * pointLight.lightPosition;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
        vec3 specular = pointLight.specularIntensity * spec * Specular;
        // attenuation
        //float distance = length(lights[i].Position - FragPos);
        //float attenuation = 1.0 / (1.0 + lights[i].Linear * distance + lights[i].Quadratic * distance * distance);
        //diffuse *= attenuation;
        //specular *= attenuation;
        lighting += diffuse + specular;
    }    
    FragColor = vec4(lighting, 1.0);

    //normal = normalize(Normals);

    //vec3 viewDirection = normalize(viewPosition - FragPosition); //Direction from the camera to the fragment.

    //Phase 1: Directional Lighting
    //vec3 result = CalculateDirectionalLight(directionalLight, normal, viewDirection, Diffuse, vec3(Specular, Specular, Specular));

    //Phase 2: Point Light
    //result += CalculatePointLight(pointLight, normal, FragPosition, viewDirection, Diffuse, vec3(Specular, Specular, Specular));

    //FragColor = vec4(result, 1.0f);
}