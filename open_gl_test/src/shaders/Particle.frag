#version 330 core

// Interpolated values from the vertex shaders
in vec4 particlecolor;

// Ouput data
out vec4 color;

void main(){
	// Output color = color of the texture at the specified UV
	color = particlecolor;

}