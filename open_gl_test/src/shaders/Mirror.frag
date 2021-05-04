#version 330
// These are the inputs which are the outputs of the vertex shader.

uniform vec3 u_cam_pos;

uniform sampler2D u_texture_1;

in vec4 v_position;
in vec4 v_normal;
in vec4 particlecolor;


// Ouput data
out vec4 color;

void main() {
  vec4 w0 = vec4(u_cam_pos, 1.0) - v_position;
  w0 = normalize(w0);
  vec4 n = normalize(v_normal);
  
  // reflection function
  // https://math.stackexchange.com/questions/13261/how-to-get-a-reflection-vector
  vec4 wi = w0 - 2 * dot(w0, n) * n * .1;
  vec2 wi_3 = vec2(wi[0], wi[1]);
  // texture only accepts the 3d vector for wi
//   wi_3 = vec2(.5, .5);

  color = texture(u_texture_1, wi_3);

  // YOUR CODE HERE
  // out_color = (vec4(1, 1, 1, 0) + v_normal) / 2;
  color.a = 1;
}
