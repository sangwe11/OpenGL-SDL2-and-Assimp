#version 140

// Texture coords
in vec2 uvIn;

// Final color out
out vec4 finalColor;

// Material data
uniform vec3 diffuseColor;

// Texture data
uniform sampler2D diffuseTexture;

// Textures flags
uniform bool hasDiffuseTexture;

// Gamma correction
vec3 gamma = vec3(1.0/2.0);

void main()
{
	// Fragment final color
	vec3 outColor = vec3(0.0);
	
	// Material properties
	vec3 color;
	
	// Get diffuse color
	if(hasDiffuseTexture)
		color = pow(texture(diffuseTexture, uvIn).rgb, gamma);
	else
		color = diffuseColor;
		
	// Final color
	finalColor = vec4(color, 1.0);
}