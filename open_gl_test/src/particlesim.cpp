#include "particlesim.h"
#include "shader.h"
#include "utils/misc.h"
#include <glm/gtc/matrix_transform.hpp>
#include "glm/gtc/type_ptr.hpp" // import to get value ptr
#include "physics.h"
#include "constants.h"

ParticleSim::ParticleSim(Screen *screen, GLFWwindow *window)
{
  this->screen = screen;
  this->window = window;
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_DEPTH_TEST);

  this->particlesContainer = new Particle[MAX_PARTICLES];
  this->external_force = new ExternalForce();
}

ParticleSim::~ParticleSim()
{
  delete[] this->particlesContainer;
  delete this->external_force;
  delete[] g_particule_position_size_data;
  delete[] g_particule_color_data;

  // Cleanup VBO and shader
  glDeleteBuffers(1, &particles_color_buffer);
  glDeleteBuffers(1, &particles_position_buffer);

  glDeleteBuffers(1, &spherePositionVbo);
  glDeleteBuffers(1, &sphereIndexVbo);
  glDeleteBuffers(1, &sphereNormalVbo);
  glDeleteBuffers(1, &sphereUVVbo);

  glDeleteTextures(1, &m_gl_texture_1);
  glDeleteProgram(programID);
  glDeleteVertexArrays(1, &VertexArrayID);
}

// https://www.songho.ca/opengl/gl_sphere.html
void createSphere(
  std::vector<float> &positions,
  std::vector<GLuint> &indices, 
  std::vector<float> &normals,
  std::vector<float> &uv,
  GLuint &spherePositionVbo, 
  GLuint &sphereIndexVbo,
  GLuint &sphereNormalVbo,
  GLuint &sphereUVVbo
  )
{
  int stacks = 20;
  int slices = 20;
  const float radius = sphere_radius;
// https://stackoverflow.com/questions/31799670/applying-map-of-the-earth-texture-a-sphere/31804515#31804515
  // loop through stacks.
  for (int i = 0; i <= stacks; ++i)
  {

    float V = (float)i / (float)stacks;
    float phi = (V - .5) * PI;

    // loop through the slices.
    for (int j = 0; j <= slices; ++j)
    {

      float U = (float)j / (float)slices;
      float theta = U * (PI * 2);

      float b = phi;
      float a = theta;

      // use spherical coordinates to calculate the positions.
      // float x = cos(theta) * sin(phi) * radius;
      // float y = cos(phi) * radius;
      // float z = sin(theta) * sin(phi) * radius;
      float x = cos(b) * cos(a) * radius;
      float y = cos(b) * sin(a) * radius;
      float z = sin(b) * radius;
      positions.push_back(x);
      positions.push_back(y);
      positions.push_back(z);
      
      uv.push_back(a/(2*PI));
      uv.push_back(b/PI + .5);
      // tx=a/(2.0*PI)
      // ty=(b/PI)+0.5;

      // TODO: idk if this actually how normals work, but it seems to work. Need to check over this sometime
      glm::vec3 v = glm::vec3(x, y, z);
      glm::vec3 n = glm::normalize(v);
      normals.push_back(n[0]);
      normals.push_back(n[1]);
      normals.push_back(n[2]);
    }
  }

  // Calc The Index Positions
  for (int i = 0; i < slices * stacks + slices; ++i)
  {
    indices.push_back(GLuint(i));
    indices.push_back(GLuint(i + slices + 1));
    indices.push_back(GLuint(i + slices));

    indices.push_back(GLuint(i + slices + 1));
    indices.push_back(GLuint(i));
    indices.push_back(GLuint(i + 1));
  }

  // upload geometry to GPU.
  glGenBuffers(1, &spherePositionVbo);
  glBindBuffer(GL_ARRAY_BUFFER, spherePositionVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * positions.size(), positions.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &sphereIndexVbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereIndexVbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), indices.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &sphereNormalVbo);
  glBindBuffer(GL_ARRAY_BUFFER, sphereNormalVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * normals.size(), normals.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &sphereUVVbo);
  glBindBuffer(GL_ARRAY_BUFFER, sphereUVVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * uv.size(), uv.data(), GL_STATIC_DRAW);

  // sphereIndexCount = indices.size();
}

float randfloat() {
  return ((float)(rand() - RAND_MAX / 2) / RAND_MAX);
}

void ParticleSim::init()
{
  screen->setSize(default_window_size);
  initGUI(screen);

  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);

  glGenVertexArrays(1, &VertexArrayID);
  glBindVertexArray(VertexArrayID);

  // Create and compile our GLSL program from the shaders
  programID = LoadShaders(vertex_shader_file, fragment_shader_file);

  this->g_particule_position_size_data = new GLfloat[MAX_PARTICLES * 4];
  this->g_particule_color_data = new GLfloat[MAX_PARTICLES * 4];

  createSphere(
  this->positions, 
  this->indices, 
  this->normals,
  this->uv,
  this->spherePositionVbo,
  this->sphereIndexVbo,
  this->sphereNormalVbo,
  this->sphereUVVbo);

  glGenBuffers(1, &particles_position_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
  // Initialize with empty (NULL) buffer : it will be updated later, each frame.
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

  // The VBO containing the colors of the particles
  glGenBuffers(1, &particles_color_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
  // Initialize with empty (NULL) buffer : it will be updated later, each frame.
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
  
  
  glGenTextures(1, &m_gl_texture_1);
  m_gl_texture_1_size = load_texture(1, m_gl_texture_1, texture_shader_file);

  
  
  glm::vec3 avg_pm_position(0, 0, 0);

  // https://scicomp.stackexchange.com/questions/14450/how-to-get-proper-parameters-of-sph-simulation
  float max_width = 0.0, max_height = 0.0;
  for (int i = 0; i < MAX_PARTICLES; i++)
  {
    int particleIndex = i;
    this->particlesContainer[particleIndex].pos = glm::vec3(randfloat() * BOX_SIZE*.5, randfloat() * BOX_SIZE * 1.5, randfloat() * BOX_SIZE);
    this->particlesContainer[particleIndex].vel = glm::vec3(0.0f);
    this->particlesContainer[particleIndex].mass = particle_inital_mass;
    
    this->particlesContainer[particleIndex].red = 1.0;
    this->particlesContainer[particleIndex].green = 0.0;
    this->particlesContainer[particleIndex].blue = 0.0;
    this->particlesContainer[particleIndex].alpha = 1.0;
    this->particlesContainer[particleIndex].size = .2;
    avg_pm_position += this->particlesContainer[particleIndex].pos;
    max_width = max(max_width, this->particlesContainer[particleIndex].pos[0]);
    max_height = max(max_height, this->particlesContainer[particleIndex].pos[1]);
  }

  avg_pm_position = avg_pm_position * (float)(1.0 / MAX_PARTICLES);

  fluid_camera::CameraInfo camera_info;
  camera_info.hFov = 80;
  camera_info.vFov = 80;
  camera_info.nClip = 0.00001;
  camera_info.fClip = 1000;

  glm::vec3 target = glm::vec3(avg_pm_position.x, avg_pm_position.y / 2,
                               avg_pm_position.z);

  glm::vec3 c_dir = glm::vec3(0., 0., 0.);

  canonical_view_distance = max(max_width, max_height) * 0.9;
  // canonical_view_distance = max(900, 800) * 0.9;
  scroll_rate = canonical_view_distance / 10;

  view_distance = canonical_view_distance * 2;
  min_view_distance = canonical_view_distance / 10.0;
  max_view_distance = canonical_view_distance * 20.0;

  // canonicalCamera is a copy used for view resets

  camera.place(target, acos(c_dir.y), atan2(c_dir.x, c_dir.z), view_distance,
               min_view_distance, max_view_distance);
  canonicalCamera.place(target, acos(c_dir.y), atan2(c_dir.x, c_dir.z),
                        view_distance, min_view_distance, max_view_distance);

  screen_w = default_window_size(0);
  screen_h = default_window_size(1);

  camera.configure(camera_info, screen_w, screen_h);
  canonicalCamera.configure(camera_info, screen_w, screen_h);
}

bool ParticleSim::isAlive() { return is_alive; }

void ParticleSim::initGUI(Screen *screen)
{
}
void ParticleSim::reset() {
  for (int i = 0; i < MAX_PARTICLES; i++)
  {
    int particleIndex = i;
    this->particlesContainer[particleIndex].pos = glm::vec3(randfloat() * BOX_SIZE*.5, randfloat() * BOX_SIZE * 1.5, randfloat() * BOX_SIZE);
    this->particlesContainer[particleIndex].vel = glm::vec3(0.0f);
    this->particlesContainer[particleIndex].mass = particle_inital_mass;
    
    this->particlesContainer[particleIndex].red = 1.0;
    this->particlesContainer[particleIndex].green = 0.0;
    this->particlesContainer[particleIndex].blue = 0.0;
    this->particlesContainer[particleIndex].alpha = 1.0;
    this->particlesContainer[particleIndex].size = .2;
  }
}
void ParticleSim::drawContents()
{
  if(!is_paused) {
    updatePosition(this->particlesContainer, this->external_force);
  }
  glEnable(GL_DEPTH_TEST);

  glm::mat4 model = glm::mat4x4(1.0);
  // glm::mat4 view = this->getViewMatrix();
  // glm::mat4 projection = this->getProjectionMatrix();
  // glm::mat4 viewProjection = projection * view;

  Matrix4f view4f = this->getViewMatrix4f();
  Matrix4f projection4f = this->getProjectionMatrix4f();
  Matrix4f viewProjection4f = projection4f * view4f;

  // Simulate all particles
  #pragma omp parallel for
  for (int i = 0; i < MAX_PARTICLES; i++)
  {
    Particle &p = this->particlesContainer[i]; // shortcut
    this->g_particule_position_size_data[4 * i + 0] = p.pos.x;
    this->g_particule_position_size_data[4 * i + 1] = p.pos.y;
    this->g_particule_position_size_data[4 * i + 2] = p.pos.z;
    this->g_particule_position_size_data[4 * i + 3] = .1;

    this->g_particule_color_data[4 * i + 0] = p.red;
    this->g_particule_color_data[4 * i + 1] = p.green;
    this->g_particule_color_data[4 * i + 2] = p.blue;
    this->g_particule_color_data[4 * i + 3] = p.alpha;
  }

  // Update the buffers that OpenGL uses for rendering.
  // There are much more sophisticated means to stream data from the CPU to the GPU,
  // but this is outside the scope of this tutorial.
  // http://www.opengl.org/wiki/Buffer_Object_Streaming

  glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_PARTICLES * sizeof(GLfloat) * 4, g_particule_position_size_data);

  glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
  glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, MAX_PARTICLES * sizeof(GLfloat) * 4, g_particule_color_data);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Use our shader
  glUseProgram(programID);

  GLint u_model = glGetUniformLocation(programID, "u_model");
  glUniformMatrix4fv(u_model, 1, GL_FALSE, &model[0][0]);

  GLint u_view_projection = glGetUniformLocation(programID, "u_view_projection");
  glUniformMatrix4fv(u_view_projection, 1, GL_FALSE, viewProjection4f.template cast<float>().data());
  
  GLint u_cam_pos = glGetUniformLocation(programID, "u_cam_pos");
  glm::vec3 c_pos = camera.position();
  glUniform3f(u_cam_pos, c_pos.x, c_pos.y, c_pos.z);

  GLint u_light_pos = glGetUniformLocation(programID, "u_light_pos");
  glUniform3f(u_light_pos, 0.5, 2, 2);

  GLint u_light_intensity = glGetUniformLocation(programID, "u_light_intensity");
  glUniform3f(u_light_intensity, 3, 3, 3);

  GLint u_texture_1_size = glGetUniformLocation(programID, "u_texture_1_size");
  glUniform2f(u_texture_1_size, m_gl_texture_1_size.x, m_gl_texture_1_size.y);

  GLint u_texture_1 = glGetUniformLocation(programID, "u_texture_1");
  glUniform1i(u_texture_1, 1); // 1 bc this is the first texture




  // Same as the billboards tutorial
  // glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
  // glUniform3f(CameraUp_worldspace_ID, ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

  // glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);
  glEnableVertexAttribArray(4);
  glBindBuffer(GL_ARRAY_BUFFER, sphereUVVbo);
  glVertexAttribPointer(
      4,        // attribute. No particular reason for 0, but must match the layout in the shader.
      2,        // size
      GL_FLOAT, // type
      GL_FALSE, // normalized?
      0,        // stride
      (void *)0 // array buffer offset
  );
  
  glEnableVertexAttribArray(3);
  glBindBuffer(GL_ARRAY_BUFFER, sphereNormalVbo);
  glVertexAttribPointer(
      3,        // attribute. No particular reason for 0, but must match the layout in the shader.
      3,        // size
      GL_FLOAT, // type
      GL_FALSE, // normalized?
      0,        // stride
      (void *)0 // array buffer offset
  );
  
  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, spherePositionVbo);
  glVertexAttribPointer(
      0,        // attribute. No particular reason for 0, but must match the layout in the shader.
      3,        // size
      GL_FLOAT, // type
      GL_FALSE, // normalized?
      0,        // stride
      (void *)0 // array buffer offset
  );

  // 2nd attribute buffer : positions of particles' centers
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
  glVertexAttribPointer(
      1,        // attribute. No particular reason for 1, but must match the layout in the shader.
      4,        // size : x + y + z + size => 4
      GL_FLOAT, // type
      GL_FALSE, // normalized?
      0,        // stride
      (void *)0 // array buffer offset
  );

  // 3rd attribute buffer : particles' colors
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
  glVertexAttribPointer(
      2,        // attribute. No particular reason for 1, but must match the layout in the shader.
      4,        // size : r + g + b + a => 4
      GL_FLOAT, // type
      GL_FALSE, // normalized?
      0,        // stride
      (void *)0 // array buffer offset
  );

  // These functions are specific to glDrawArrays*Instanced*.
  // The first parameter is the attribute buffer we're talking about.
  // The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
  // http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
  glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
  glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
  glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

  // Draw the particules !
  // This draws many times a small triangle_strip (which looks like a quad).
  // This is equivalent to :
  // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
  // but faster.
  glDrawElementsInstanced(
      GL_TRIANGLES,    // mode
      indices.size(),  // count
      GL_UNSIGNED_INT, // type
      (void *)0,       // element array buffer offset
      MAX_PARTICLES);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
}
// ----------------------------------------------------------------------------
// CAMERA CALCULATIONS
//
// OpenGL 3.1 deprecated the fixed pipeline, so we lose a lot of useful OpenGL
// functions that have to be recreated here.
// ----------------------------------------------------------------------------

void ParticleSim::resetCamera() { camera.copy_placement(canonicalCamera); }
Matrix4f ParticleSim::getProjectionMatrix4f() {
  Matrix4f perspective;
  perspective.setZero();

  double cam_near = camera.near_clip();
  double cam_far = camera.far_clip();

  double theta = camera.v_fov() * PI / 360;
  double range = cam_far - cam_near;
  double invtan = 1. / tanf(theta);

  perspective(0, 0) = invtan / camera.aspect_ratio();
  perspective(1, 1) = invtan;
  perspective(2, 2) = -(cam_near + cam_far) / range;
  perspective(3, 2) = -1;
  perspective(2, 3) = -2 * cam_near * cam_far / range;
  perspective(3, 3) = 0;

  return perspective;
}
Matrix4f ParticleSim::getViewMatrix4f() {
   Matrix4f lookAt;
  Matrix3f R;

  lookAt.setZero();

  // Convert CGL vectors to Eigen vectors
  // TODO: Find a better way to do this!

  glm::vec3 c_pos = camera.position();
  glm::vec3 c_udir = camera.up_dir();
  glm::vec3 c_target = camera.view_point();

  Vector3f eye(c_pos.x, c_pos.y, c_pos.z);
  Vector3f up(c_udir.x, c_udir.y, c_udir.z);
  Vector3f target(c_target.x, c_target.y, c_target.z);

  R.col(2) = (eye - target).normalized();
  R.col(0) = up.cross(R.col(2)).normalized();
  R.col(1) = R.col(2).cross(R.col(0));

  lookAt.topLeftCorner<3, 3>() = R.transpose();
  lookAt.topRightCorner<3, 1>() = -R.transpose() * eye;
  lookAt(3, 3) = 1.0f;

  return lookAt;
}

glm::mat4 ParticleSim::getProjectionMatrix()
{
  float cam_near = camera.near_clip();
  float cam_far = camera.far_clip();
  float aspect_ration = camera.aspect_ratio();
  return glm::perspective(glm::radians(camera.v_fov()), 4.0f / 3.0f, 0.1f, 100.0f);
}

glm::mat4 ParticleSim::getViewMatrix()
{
  float horizontalAngle = 3.14f;
  float verticalAngle = 0.0f;
  glm::vec3 position = glm::vec3(0, 1, 5);
  // Direction : Spherical coordinates to Cartesian coordinates conversion
  glm::vec3 direction(
      cos(verticalAngle) * sin(horizontalAngle),
      sin(verticalAngle),
      cos(verticalAngle) * cos(horizontalAngle));

  // Right vector
  glm::vec3 right = glm::vec3(
      sin(horizontalAngle - 3.14f / 2.0f),
      0,
      cos(horizontalAngle - 3.14f / 2.0f));

  glm::vec3 up = glm::cross(right, direction);

  glm::mat4 lookAtglm = glm::lookAt(
      position,             // Camera is here
      position + direction, // and looks here : at the same position, plus "direction"
      up                    // Head is up (set to 0,-1,0 to look upside-down)
  );

  return lookAtglm;
}

// ----------------------------------------------------------------------------
// EVENT HANDLING
// ----------------------------------------------------------------------------

bool ParticleSim::cursorPosCallbackEvent(double x, double y) {
  if (left_down && !middle_down && !right_down) {
    if (ctrl_down) {
      mouseRightDragged(x, y);
    } else {
      mouseLeftDragged(x, y);
    }
  } else if (!left_down && !middle_down && right_down) {
    mouseRightDragged(x, y);
  } else if (!left_down && !middle_down && !right_down) {
    mouseMoved(x, y);
  }

  mouse_x = x;
  mouse_y = y;

  return true;
}

bool ParticleSim::mouseButtonCallbackEvent(int button, int action,
                                              int modifiers) {
  switch (action) {
  case GLFW_PRESS:
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      left_down = true;
      break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
      middle_down = true;
      break;
    case GLFW_MOUSE_BUTTON_RIGHT:
      right_down = true;
      break;
    }
    return true;

  case GLFW_RELEASE:
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
      left_down = false;
      break;
    case GLFW_MOUSE_BUTTON_MIDDLE:
      middle_down = false;
      break;
    case GLFW_MOUSE_BUTTON_RIGHT:
      right_down = false;
      break;
    }
    return true;
  }

  return false;
}

void ParticleSim::mouseMoved(double x, double y) { y = screen_h - y; }

void ParticleSim::mouseLeftDragged(double x, double y) {
  float dx = x - mouse_x;
  float dy = y - mouse_y;
  if(!pause_camera) {
    camera.rotate_by(-dy * (PI / screen_h), -dx * (PI / screen_w));
  } else {
    bool external_mouse_force_enabled = true;
    glm::vec2 current_xy = glm::vec2(x/screen_w, y/screen_h);
    glm::vec2 current_dxdy = normalize(glm::vec2(dx/screen_w, dy/screen_h));

    glm::mat4 inverse_view_proj = glm::inverse(getProjectionMatrix() * getViewMatrix());
    glm::vec4 current_xy_model_space = inverse_view_proj*glm::vec4(current_xy, 0.0, 1.0);
    glm::vec4 current_dxy_model_space = inverse_view_proj*glm::vec4(current_dxdy, 0.0, 1.0);

    this->external_force->num_iterations = num_external_force_iterations;
    this->external_force->external_mouse_force_enabled = true;
    this->external_force->current_starting_force = glm::vec3(current_xy_model_space.x, current_xy_model_space.y,current_xy_model_space.z);
    this->external_force->current_dxdy =  glm::vec3(current_dxy_model_space.x, current_dxy_model_space.y,current_dxy_model_space.z);
  }
}

void ParticleSim::mouseRightDragged(double x, double y) {
  float dx = mouse_x - x;
  float dy = y - mouse_y;
  if(!pause_camera) {
    camera.move_by(dx, dy, canonical_view_distance);
  } else {
    // bool external_mouse_force_enabled = true;
    // glm::vec2 current_xy = glm::vec2(x/screen_w, y/screen_h);
    // glm::vec2 current_dxdy = glm::vec2(dx/screen_w, dy/screen_h);
    // glm::vec4 current_xyz = getViewMatrix() * glm::vec4(glm::vec2(x/screen_w, y/screen_h), 1.0f, 1.0f);
    // glm::vec3 current_xyz_pos = glm::vec3(current_xyz.x, current_xyz.y, current_xyz.z);

    // this->external_force->external_mouse_force_enabled = true;
    // this->external_force->current_xyz = current_xyz_pos;
    // this->external_force->current_dxdy = current_dxdy;
    bool external_mouse_force_enabled = true;
    glm::vec2 current_xy = glm::vec2(x/screen_w, y/screen_h);
    glm::vec2 current_dxdy = normalize(glm::vec2(dx/screen_w, dy/screen_h));

    glm::mat4 inverse_view_proj = glm::inverse(getProjectionMatrix() * getViewMatrix());
    glm::vec4 current_xy_model_space = inverse_view_proj*glm::vec4(current_xy, 0.0, 1.0);
    glm::vec4 current_dxy_model_space = inverse_view_proj*glm::vec4(current_dxdy, 0.0, 1.0);

    this->external_force->num_iterations = num_external_force_iterations;
    this->external_force->external_mouse_force_enabled = true;
    this->external_force->current_starting_force = glm::vec3(current_xy_model_space.x, current_xy_model_space.y,current_xy_model_space.z);
    this->external_force->current_dxdy =  glm::vec3(current_dxy_model_space.x, current_dxy_model_space.y,current_dxy_model_space.z);
  }
}

bool ParticleSim::keyCallbackEvent(int key, int scancode, int action,
                                      int mods) {
  ctrl_down = (bool)(mods & GLFW_MOD_CONTROL);

  if (action == GLFW_PRESS) {
    switch (key) {
    case GLFW_KEY_ESCAPE:
      is_alive = false;
      break;
    case 'r':
    case 'R':
      // cloth->reset();
      printf("Reset orientation \n");
      reset();
      break;
    case ' ':
      resetCamera();
      break;
    case 'p':
    case 'P':
      is_paused = !is_paused;
      printf("Current Paused %d\n", is_paused);
      break;
    case 'w':
    case 'W':
      this->external_force->whirlpool_enabled = !this->external_force->whirlpool_enabled;
      printf("External force enabled %d\n", this->external_force->whirlpool_enabled);
      break;
    case 'c':
    case 'C':
      pause_camera = !pause_camera;
      printf("Interaction Enabled %d\n", !pause_camera);
      break;
    case 'n':
    case 'N':
      if (is_paused) {
        is_paused = false;
        drawContents();
        is_paused = true;
      }
      break;
    }
  }

  return true;
}

bool ParticleSim::dropCallbackEvent(int count, const char **filenames) {
  return true;
}

bool ParticleSim::scrollCallbackEvent(double x, double y) {
  camera.move_forward(y * scroll_rate);
  return true;
}

bool ParticleSim::resizeCallbackEvent(int width, int height) {
  screen_w = width;
  screen_h = height;

  camera.set_screen_size(screen_w, screen_h);
  return true;
}
