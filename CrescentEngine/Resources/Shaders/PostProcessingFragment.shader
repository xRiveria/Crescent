#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D TexSrc;

//Post-Processing Effect Toggles
uniform int SSAO;

//Motion Blur
uniform sampler2D gMotion;

void main()
{
	vec3 color = texture(TexSrc, TexCoords).rgb;
	vec3 grayscale = vec3(dot(color, vec3(0.299, 0.587, 0.114)));
	vec2 texelSize = 1.0 / textureSize(TexSrc, 0).xy;

	// HDR tonemapping
	const float exposure = 1.0f;
	color *= exposure;
	// color = aces(color);
	color = color / (color + vec3(1.0));
	// gamma correct
	color = pow(color, vec3(1.0 / 2.2));

	FragColor = vec4(color, 1.0);
}