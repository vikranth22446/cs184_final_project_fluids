#include "particlesim.h"
#include "shader.h"
#include "controls.hpp"

ParticleSim::ParticleSim(Screen *screen, GLFWwindow *window, int max_particles)
{
  this->screen = screen;
  this->window = window;
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_DEPTH_TEST);

  this->max_particles = max_particles;
  this->particlesContainer = new Particle[this->max_particles];
}
ParticleSim::~ParticleSim()
{
  delete[] this->particlesContainer;
  delete[] g_particule_position_size_data;

  // Cleanup VBO and shader
  glDeleteBuffers(1, &particles_color_buffer);
  glDeleteBuffers(1, &particles_position_buffer);
  glDeleteBuffers(1, &spherePositionVbo);
  glDeleteProgram(programID);
  glDeleteVertexArrays(1, &VertexArrayID);
}

// https://www.songho.ca/opengl/gl_sphere.html
void createSphere(std::vector<GLuint> &indices, std::vector<float> positions, GLuint &spherePositionVbo, GLuint &sphereIndexVbo)
{
  int stacks = 20;
  int slices = 20;
  const float radius = .1;
  const float PI = 3.14f;

  // loop through stacks.
  for (int i = 0; i <= stacks; ++i)
  {

    float V = (float)i / (float)stacks;
    float phi = V * PI;

    // loop through the slices.
    for (int j = 0; j <= slices; ++j)
    {

      float U = (float)j / (float)slices;
      float theta = U * (PI * 2);

      // use spherical coordinates to calculate the positions.
      float x = cos(theta) * sin(phi) * radius;
      float y = cos(phi) * radius;
      float z = sin(theta) * sin(phi) * radius;

      positions.push_back(x);
      positions.push_back(y);
      positions.push_back(z);
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

  // sphereIndexCount = indices.size();
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
  programID = LoadShaders("../src/shaders/Particle.vert", "../src/shaders/Particle.frag");

  // Vertex shader
  CameraRight_worldspace_ID = glGetUniformLocation(programID, "CameraRight_worldspace");
  CameraUp_worldspace_ID = glGetUniformLocation(programID, "CameraUp_worldspace");
  ViewProjMatrixID = glGetUniformLocation(programID, "VP");

  const int MaxParticles = this->max_particles;
  this->g_particule_position_size_data = new GLfloat[MaxParticles * 4];
  this->g_particule_color_data = new GLfloat[MaxParticles * 4];

  createSphere(this->indices, this->positions, this->spherePositionVbo, this->sphereIndexVbo);

  glGenBuffers(1, &particles_position_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
  // Initialize with empty (NULL) buffer : it will be updated later, each frame.
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

  // The VBO containing the colors of the particles
  glGenBuffers(1, &particles_color_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
  // Initialize with empty (NULL) buffer : it will be updated later, each frame.
  glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

  for (int i = 0; i < MaxParticles; i++)
  {
    int particleIndex = i;
    this->particlesContainer[particleIndex].pos = glm::vec3(((float)(rand() - RAND_MAX / 2) / RAND_MAX) * 40, ((float)(rand() - RAND_MAX / 2) / RAND_MAX) * 40, -20.0f);
    this->particlesContainer[particleIndex].red = 1.0;
    this->particlesContainer[particleIndex].green = 0.0;
    this->particlesContainer[particleIndex].blue = 0.0;
    this->particlesContainer[particleIndex].alpha = 1.0;
    this->particlesContainer[particleIndex].size = .2;
  }
}

bool ParticleSim::isAlive() { return is_alive; }

void ParticleSim::initGUI(Screen *screen)
{
}

void ParticleSim::drawContents()
{
  glEnable(GL_DEPTH_TEST);

  computeMatricesFromInputs(window);
  glm::mat4 ProjectionMatrix = getProjectionMatrix();
  glm::mat4 ViewMatrix = getViewMatrix();
  glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);
  glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

  // Simulate all particles
  for (int i = 0; i < this->max_particles; i++)
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
  glBufferData(GL_ARRAY_BUFFER, this->max_particles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, this->max_particles * sizeof(GLfloat) * 4, g_particule_position_size_data);

  glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
  glBufferData(GL_ARRAY_BUFFER, this->max_particles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, this->max_particles * sizeof(GLfloat) * 4, g_particule_color_data);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Use our shader
  glUseProgram(programID);

  // Same as the billboards tutorial
  glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
  glUniform3f(CameraUp_worldspace_ID, ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

  glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

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
      this->max_particles);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
}

//
// CAMERA CONTROLS
//

bool ParticleSim::cursorPosCallbackEvent(double x, double y)
{
  if (left_down && !middle_down && !right_down)
  {
    if (ctrl_down)
    {
      mouseRightDragged(x, y);
    }
    else
    {
      mouseLeftDragged(x, y);
    }
  }
  else if (!left_down && !middle_down && right_down)
  {
    mouseRightDragged(x, y);
  }
  else if (!left_down && !middle_down && !right_down)
  {
    mouseMoved(x, y);
  }

  mouse_x = x;
  mouse_y = y;

  return true;
}

bool ParticleSim::mouseButtonCallbackEvent(int button, int action,
                                           int modifiers)
{
  switch (action)
  {
  case GLFW_PRESS:
    switch (button)
    {
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
    switch (button)
    {
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

void ParticleSim::mouseLeftDragged(double x, double y)
{
  float dx = x - mouse_x;
  float dy = y - mouse_y;

  //   camera.rotate_by(-dy * (PI / screen_h), -dx * (PI / screen_w));
}

void ParticleSim::mouseRightDragged(double x, double y)
{
  //   camera.move_by(mouse_x - x, y - mouse_y, canonical_view_distance);
}

bool ParticleSim::keyCallbackEvent(int key, int scancode, int action,
                                   int mods)
{
  ctrl_down = (bool)(mods & GLFW_MOD_CONTROL);

  if (action == GLFW_PRESS)
  {
    switch (key)
    {
    case GLFW_KEY_ESCAPE:
      is_alive = false;
      break;
    case 'r':
    case 'R':
      //   cloth->reset();
      break;
    case ' ':
      //   resetCamera();
      break;
    case 'p':
    case 'P':
      is_paused = !is_paused;
      break;
    case 'n':
    case 'N':
      if (is_paused)
      {
        is_paused = false;
        drawContents();
        is_paused = true;
      }
      break;
    }
  }

  return true;
}

bool ParticleSim::dropCallbackEvent(int count, const char **filenames)
{
  return true;
}

bool ParticleSim::scrollCallbackEvent(double x, double y)
{
  //   camera.move_forward(y * scroll_rate);
  return true;
}

bool ParticleSim::resizeCallbackEvent(int width, int height)
{
  screen_w = width;
  screen_h = height;

  //   camera.set_screen_size(screen_w, screen_h);
  return true;
}
