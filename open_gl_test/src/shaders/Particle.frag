#version 330 core

// Interpolated values from the vertex shaders
// uniform vec3 u_cam_pos;
// uniform vec4 u_color;

// Properties of the single point light
uniform vec3 u_light_pos;
uniform vec3 u_light_intensity;

// These are the inputs which are the outputs of the vertex shader.
in vec4 v_position;
in vec4 v_normal;
in vec4 particlecolor;

// Ouput data
out vec4 color;

void main(){
  // YOUR CODE HERE
  vec4 u_light_pos4 = vec4(u_light_pos  * 2, 1.0);
  vec4 u_light_intensity4 = vec4(u_light_intensity, 1.0);

  float r = length(u_light_pos4 - v_position);
  float dot_prod = dot(normalize(v_normal), normalize(u_light_pos4 - v_position));
  vec4 L_d = (u_light_intensity4)/(r * r) * max(0.0, dot_prod);
  // clamp between 0.0 and 1.0 bc it's color
  L_d = clamp(L_d, 0.0, 1.0); 
  
  // (Placeholder code. You will want to replace it.)
  // out_color = L_d * (vec4(1, 1, 1, 0) + v_normal) / 2;
  // out_color.a = 1;

  color = v_normal;
//   color = L_d;
//   color.a = 1;
}
