#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;
layout(location = 3) in ivec4 aBoneIDs0;
layout(location = 4) in ivec4 aBoneIDs1;
layout(location = 5) in vec4 aBoneWeights0;
layout(location = 6) in vec4 aBoneWeights1;

out vec2 TexCoords;
out vec3 Normals;
out vec3 FragPosition;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 uBoneMatrices[100];

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
    mat4 boneMatrix = CalcBoneMatrix();

    TexCoords = aTexCoords;
    FragPosition = vec3(model * boneMatrix * vec4(aPos, 1.0));
    Normals = aNormal;
    gl_Position = projection * view * model * boneMatrix * vec4(aPos, 1.0);
}