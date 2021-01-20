#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec2 UV;
out vec3 FragPos;
out mat3 TBN;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main()
{
	UV = aUV;
	FragPos = vec3(model * vec4(aPos, 1.0));

	vec3 N = normalize(mat3(model) * aNormal);
	vec3 T = normalize(mat3(model) * aTangent);
	T = normalize(T - dot(N, T) * N);

	vec3 B = normalize(mat3(model) * aBitangent);

	//TBN must form a right handed coordinate system.
	//Some models have symetric UVs. Check and fix.
	if (dot(cross(N, T), B) < 0.0)
	{
		T = T * -1.0;
	}

	TBN = mat3(T, B, N);

	gl_Position = projection * view * vec4(FragPos, 1.0);
}
