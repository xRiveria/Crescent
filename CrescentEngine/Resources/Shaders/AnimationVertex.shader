#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;
layout(location = 5) in ivec4 aBoneIDs0;
layout(location = 6) in ivec4 aBoneIDs1;
layout(location = 7) in vec4 aBoneWeights0;
layout(location = 8) in vec4 aBoneWeights1;

out vec2 TexCoords;
out vec3 Normals;
out vec3 FragPosition;
out vec3 TangentLightPosition;
out vec3 TangentViewPosition;
out vec3 TangentFragPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 uBoneMatrices[100];

uniform vec3 lightPosition;
uniform vec3 viewPosition;

mat4 CalcBoneMatrix() {
    mat4 boneMatrix = mat4(0);
    for (int i = 0; i < 4; i++) {
        boneMatrix += uBoneMatrices[aBoneIDs0[i]] * aBoneWeights0[i];
        boneMatrix += uBoneMatrices[aBoneIDs1[i]] * aBoneWeights1[i];
    }
    return boneMatrix;
}

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

    mat4 boneMatrix = CalcBoneMatrix();

    TexCoords = aTexCoords;
    FragPosition = vec3(model * boneMatrix * vec4(aPos, 1.0));
    Normals = aNormal;
    gl_Position = projection * view * model * boneMatrix * vec4(aPos, 1.0);
}