#include "glm/glm.hpp"
#ifndef PARTICLES_H
#define PARTICLES_H
struct Particles {
    void simulate(double frames_per_sec, double simulation_steps);
    void reset();
    void initializeBuffer();
};
struct Particle {
    glm::vec3 pos;
    glm::vec3 vel;
    float density, mass; 
    float red, green, blue, alpha;
    float size;

    glm::vec3 pressure_force;
    glm::vec3 external;
    glm::vec3 viscosity;
    glm::vec3 surface_tension;
};
#endif /* PARTICLES_H */
