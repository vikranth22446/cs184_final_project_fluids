#include "main.h"
#include "particle.h"

#ifndef PARTICLE_SIM_H
#define PARTICLE_SIM_H

#include "glm/glm.hpp"
#include <memory>

class ParticleSim {
public:
  ParticleSim(nanogui::Screen *screen, int max_particles);
  ~ParticleSim();

  void init();

  virtual bool isAlive();
  virtual void drawContents();

  // Screen events

  virtual bool cursorPosCallbackEvent(double x, double y);
  virtual bool mouseButtonCallbackEvent(int button, int action, int modifiers);
  virtual bool keyCallbackEvent(int key, int scancode, int action, int mods);
  virtual bool dropCallbackEvent(int count, const char **filenames);
  virtual bool scrollCallbackEvent(double x, double y);
  virtual bool resizeCallbackEvent(int width, int height);
 
 // Rendering helpers
  int max_particles;
  GLuint spherePositionVbo;
  GLuint sphereIndexVbo;
  std::vector<GLuint> indices;
  std::vector<float> positions;

 GLfloat* g_particule_position_size_data;
 GLfloat* g_particule_color_data;
 GLuint particles_position_buffer;
 GLuint particles_color_buffer;

 GLuint VertexArrayID;
 GLuint programID; // reference to shader
 GLuint CameraRight_worldspace_ID;
 GLuint CameraUp_worldspace_ID;
 GLuint ViewProjMatrixID;

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

  double scroll_rate;

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
};


#endif // PARTICLE_SIM_H
