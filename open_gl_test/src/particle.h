#include "glm/glm.hpp"
#ifndef PARTICLES_H
#define PARTICLES_H
struct Particle {
    glm::vec3 pos;
    glm::vec3 vel;
    float density, mass; 
    float red, green, blue, alpha;
    float size;
    glm::vec3 net_force;

    glm::vec3 pressure_force;
    glm::vec3 external;
    glm::vec3 viscosity;
    glm::vec3 surface_tension;
};
struct ExternalForce {
  glm::vec3 current_starting_force;
  glm::vec3 current_dxdy;
  bool external_mouse_force_enabled = false;
  int num_iterations = 0;
};
#endif /* PARTICLES_H */
