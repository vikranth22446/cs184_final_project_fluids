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
};
#endif /* PARTICLES_H */
