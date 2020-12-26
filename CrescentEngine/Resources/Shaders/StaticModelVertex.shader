#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out vec2 TexCoords;
out vec3 FragPosition;
out vec3 TangentLightPosition;
out vec3 TangentViewPosition;
out vec3 TangentFragPosition;

out vec3 Normals;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform vec3 lightPosition;
uniform vec3 viewPosition;

out vec4 FragPosLightSpace;

void main()
{
    //Converting from world space to tangent space here.
    vec3 T = normalize(vec3(model * vec4(aTangent, 0.0)));
    vec3 B = normalize(vec3(model * vec4(aBitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(aNormal, 0.0)));
    mat3 TBN = transpose(mat3(T, B, N));

    TangentLightPosition = TBN * lightPosition;
    TangentViewPosition = TBN * viewPosition;
    TangentFragPosition = TBN * vec3(model * vec4(aPos, 1.0));

    //==================================================

    TexCoords = aTexCoords;
    FragPosition = vec3(model * vec4(aPos, 1.0));
    Normals = mat3(transpose(inverse(model))) * aNormal;
    FragPosLightSpace = lightSpaceMatrix * vec4(FragPosition, 1.0);

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}