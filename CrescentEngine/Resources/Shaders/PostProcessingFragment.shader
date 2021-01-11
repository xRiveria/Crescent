#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D TexSource;

uniform sampler2D gMotion;

void main()
{
	vec3 color = texture(TexSource, TexCoords).rgb;

	//HDR Tonemapping
	//const float exposure = 1.0f;
	////color *= exposure;

	//color = color / (color + vec3(1.0f));
	//Gamma Correct
	//color = pow(color, vec3(1.0 / 2.2));

	FragColor = vec4(color, 1.0f);
}