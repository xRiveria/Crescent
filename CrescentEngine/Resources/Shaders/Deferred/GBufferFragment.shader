#version 420 core
layout (location = 0) out vec4 gPositionMetallic;
layout (location = 1) out vec4 gNormalRoughness;
layout (location = 2) out vec4 gAlbedoAO;
layout(location = 3) out vec4 gMotion;

in vec2 UV;
in vec3 FragPos;
in mat3 TBN;

uniform sampler2D TexAlbedo;
uniform sampler2D TexNormal;
uniform sampler2D TexMetallic;
uniform sampler2D TexRoughness;

uniform sampler2D TexAO;

void main()
{
	//Store the per-fragment position vector in the first GBuffer texture.
	gPositionMetallic.rgb = FragPos;
	gPositionMetallic.a = texture(TexMetallic, UV).r;
	//Also store the per-fragment roughness into the GBuffer.
	float roughness = texture(TexRoughness, UV).r;

	//Normals
	vec3 N = texture(TexNormal, UV).rgb;
	N = normalize(N * 2.0 - 1.0);
	N = normalize(TBN * N);

	gNormalRoughness.rgb = normalize(N);
	gNormalRoughness.a = roughness;
	//And the diffuse per-fragment color.
	gAlbedoAO.rgb = texture(TexAlbedo, UV).rgb;
	gAlbedoAO.a = texture(TexAO, UV).r;
}