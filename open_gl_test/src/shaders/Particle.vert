#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 centers; // Position of the center of the particule and size of the square
layout(location = 2) in vec4 color; // Position of the center of the particule and size of the square

// Output data ; will be interpolated for each fragment.
out vec4 particlecolor;

// Values that stay constant for the whole mesh.
uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;
uniform mat4 VP; // Model-View-Projection matrix, but without the Model (the position is in BillboardPos; the orientation depends on the camera)

void main()
{	
	float size = centers.w;
	vec3 vertex_shifted_by_center = (centers.xyz + vertex);

	// Output position of the vertex
	gl_Position = VP * vec4(vertex_shifted_by_center, 1.0f);

	// UV of the vertex. No special space for this one.
	particlecolor = color;
}

