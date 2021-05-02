#include "main.h"
#ifndef PARTICLE_SIM_H
#define PARTICLE_SIM_H

#include "glm/glm.hpp"
#include <memory>

class ParticleSim {
public:
  ParticleSim(nanogui::Screen *screen);
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
};


#endif // PARTICLE_SIM_H
