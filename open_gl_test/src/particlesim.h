#include "main.h"
#include "particle.h"
#include "camera.h"

#ifndef PARTICLE_SIM_H
#define PARTICLE_SIM_H

#include "glm/glm.hpp"
#include <memory>

class ParticleSim {
public:
  ParticleSim(nanogui::Screen *screen, GLFWwindow *window);
  ~ParticleSim();

  void init();
  virtual bool isAlive();
  virtual void drawContents();

  virtual bool cursorPosCallbackEvent(double x, double y);
  virtual bool mouseButtonCallbackEvent(int button, int action, int modifiers);
  virtual bool keyCallbackEvent(int key, int scancode, int action, int mods);
  virtual bool dropCallbackEvent(int count, const char **filenames);
  virtual bool scrollCallbackEvent(double x, double y);
  virtual bool resizeCallbackEvent(int width, int height);
 
 // Rendering helpers
 GLuint spherePositionVbo;
 GLuint sphereIndexVbo;
 GLuint sphereNormalVbo;
 std::vector<GLuint> indices;
 std::vector<float> positions;
 std::vector<float> normals;

 GLfloat* g_particule_position_size_data;
 GLfloat* g_particule_color_data;
 GLuint particles_position_buffer;
 GLuint particles_color_buffer;

 GLuint VertexArrayID;
 GLuint programID; // reference to shader
 GLFWwindow *window;

// camera controls

  fluid_camera::Camera camera;
  fluid_camera::Camera canonicalCamera;

  double view_distance;
  double canonical_view_distance;
  double min_view_distance;
  double max_view_distance;

  double scroll_rate;
  virtual void resetCamera();
  virtual glm::mat4  getProjectionMatrix();
  virtual glm::mat4 getViewMatrix();

  virtual Matrix4f  getProjectionMatrix4f();
  virtual Matrix4f getViewMatrix4f();
 private:
  virtual void initGUI(Screen *screen);
  void load_shaders();
  void load_textures();
  
  // Camera methods

//   virtual void resetCamera();
//   virtual glm::mat4 getProjectionMatrix();
//   virtual glm::mat4 getViewMatrix();

  // Default simulation values

  int frames_per_sec = 90;
  int simulation_steps = 30;

  // Screen methods

  Screen *screen;
  void mouseLeftDragged(double x, double y);
  void mouseRightDragged(double x, double y);
  void mouseMoved(double x, double y);

  // Mouse flags

  bool left_down = false;
  bool right_down = false;
  bool middle_down = false;

  // Keyboard flags

  bool ctrl_down = false;

  // Simulation flags

  bool is_paused = true;

  // Screen attributes

  int mouse_x;
  int mouse_y;

  int screen_w;
  int screen_h;

  bool is_alive = true;

  Vector2i default_window_size = Vector2i(1024, 800);

  Particle *particlesContainer;


  // Loading Textures
  glm::vec3 m_gl_texture_1_size;
  GLuint m_gl_texture_1;

};


#endif // PARTICLE_SIM_H
