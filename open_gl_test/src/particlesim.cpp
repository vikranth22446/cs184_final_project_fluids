#include "particlesim.h"

ParticleSim::ParticleSim(Screen *screen){
  this->screen = screen;
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_DEPTH_TEST);
}


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

//   camera.rotate_by(-dy * (PI / screen_h), -dx * (PI / screen_w));
}

void ParticleSim::mouseRightDragged(double x, double y) {
//   camera.move_by(mouse_x - x, y - mouse_y, canonical_view_distance);
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
//   camera.move_forward(y * scroll_rate);
  return true;
}

bool ParticleSim::resizeCallbackEvent(int width, int height) {
  screen_w = width;
  screen_h = height;

//   camera.set_screen_size(screen_w, screen_h);
  return true;
}

// 
void ParticleSim::init() {
  screen->setSize(default_window_size);
  initGUI(screen);  
}


bool ParticleSim::isAlive() { return is_alive; }


void ParticleSim::initGUI(Screen *screen) {
//   Window *window;
  
//   window = new Window(screen, "Simulation");
//   window->setPosition(Vector2i(default_window_size(0) - 245, 15));
//   window->setLayout(new GroupLayout(15, 6, 14, 5));

//   // Spring types

//   new Label(window, "Spring types", "sans-bold");

//   {
//     Button *b = new Button(window, "structural");
//     b->setFlags(Button::ToggleButton);
//     b->setFontSize(14);

//     b = new Button(window, "shearing");
//     b->setFlags(Button::ToggleButton);
//     b->setFontSize(14);
  
//     b = new Button(window, "bending");
//     b->setFlags(Button::ToggleButton);
//     b->setFontSize(14);
    
//   }

//   window = new Window(screen, "Appearance");
//   window->setPosition(Vector2i(15, 15));
//   window->setLayout(new GroupLayout(15, 6, 14, 5));


  // Shader Parameters
}

void ParticleSim::drawContents() {
  glEnable(GL_DEPTH_TEST);
}