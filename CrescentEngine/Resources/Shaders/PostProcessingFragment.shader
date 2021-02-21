#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D TexSrc;

//Post-Processing Effect Toggles
uniform int SSAO;
uniform bool GreyscaleEnabled;
uniform bool InverseEnabled;

//Motion Blur
uniform sampler2D gMotion;

const float offset = 1.0 / 300.0;

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

	vec4 processedColor = vec4(color, 1.0);
	if (InverseEnabled)
	{
		processedColor = vec4(vec3(1.0 - processedColor.xyz), 1.0f);
	}
	if (GreyscaleEnabled)
	{
		float average = 0.2126 * processedColor.r + 0.7152 * processedColor.g + 0.0722 * processedColor.b;
		processedColor = vec4(average, average, average, 1.0);
	}

	FragColor = processedColor;
}