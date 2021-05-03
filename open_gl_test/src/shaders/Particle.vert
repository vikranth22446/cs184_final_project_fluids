#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertex;
layout(location = 1) in vec4 centers; // Position of the center of the particule and size of the square
layout(location = 2) in vec4 color; // Position of the center of the particule and size of the square

// Output data ; will be interpolated for each fragment.
out vec4 particlecolor;

// Values that stay constant for the whole mesh.
uniform vec3 u_cam_pos;
uniform vec3 u_light_pos;

uniform mat4 u_model;
uniform mat4 u_view_projection; // Model-View-Projection matrix, but without the Model (the position is in BillboardPos; the orientation depends on the camera)

float m_normal_scaling = 2.0;
float m_height_scaling = 0.1;

void main()
{	
	float size = centers.w;
	vec3 vertex_shifted_by_center = (centers.xyz + vertex);

	// Output position of the vertex
	gl_Position = u_view_projection * vec4(vertex_shifted_by_center, 1.0f);

	// UV of the vertex. No special space for this one.
	// particlecolor = color;
	particlecolor = vec4(u_cam_pos, 1.0f);
}

