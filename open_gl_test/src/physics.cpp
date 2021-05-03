#include "math.h"
#include "particle.h"


float dist(glm::vec3 pos1, glm::vec3 pos2) {
  return sqrt(pow((pos1.x - pos2.x), 2) + pow((pos1.y - pos2.y), 2) + pow((pos1.z - pos2.z), 2));
}

float w_poly(float r, float h) {
    if(r > h || r < 0 ) return 0;
    return 315/(64*M_PI * pow(h, 9)) * pow((pow(h, 2) - pow(r, 2)), 3); 
}

float w_poly_derivative6(float r, float h) {
    if(r > h || r < 0 ) return 0;
    return 315/(64*M_PI * pow(h, 9)) * 3 * pow((pow(h, 2) - pow(r, 2)), 2) * (-2 * r); 
}

float w_poly_laplacian(float r, float h) {
  if(r > h || r < 0 ) return 0;
  return 315/(64*M_PI * pow(h, 9)) * 3 * pow((pow(h, 2) - pow(r, 2)), 2) * (-2) + 315/(64*M_PI * pow(h, 9)) * 6 * pow((pow(h, 2) - pow(r, 2)), 1) * (-2 * r); 
}

float w_viscosity(float r, float h) {
  if (r > h || r < 0) return 0; 
  return 15/(2 * M_PI * pow(h, 3)) * (-pow(r, 3)/(2 * pow(h, 3))) * (pow(r, 2)/pow(h,2)) * h/(2*r) - 1; 
}

float laplace_viscosity(float r, float h) {
  if (r > h || r < 0) return 0; 
  return 45/(M_PI * pow(h, 6)) * (h-r); 
}

float w_spiky(float r, float h) {
    if (r > h || r < 0) return 0; 
    return 15/(M_PI * pow(h, 6)) * pow((h - r), 3); 
}

float w_spiky_derivative(float r, float h) {
    if (r > h || r < 0) return 0; 
    return 15/(M_PI * pow(h, 6)) * 3 * pow((h - r), 2) * (-1); 
}

float density_s(Particle *particle, int max_particles, Particle particlesContainer[] ) {
    const int h = 1; 
    float p = 0; 
    for (size_t i = 0; i < max_particles; i++) {
        Particle *other_particle = &particlesContainer[i];
        int r = dist(other_particle->pos, particle->pos); 
        p += other_particle->mass * w_poly(r, h); 
    }
    
    particle->density = p; 
    return p; 
}

glm::vec3 compute_pressure(Particle *particle, int max_particles, Particle particlesContainer[] ) {
    glm::vec3 f_p = glm::vec3(0.0, 0.0, 0.0);
    const int k = 1, h = 1, rho_zero = 1; 
    const int pi = k * (particle->density - rho_zero); 

    for (size_t i = 0; i < max_particles; i++) {
        Particle *other_particle = &particlesContainer[i];
        const int pj = k * (other_particle->density - rho_zero); 
        const int r = dist(other_particle->pos, particle->pos); 
        const int pressure = -other_particle->mass*(pi + pj)/(2*other_particle->density) * w_spiky_derivative(r, h); 
        if (r != 0) {
            const glm::vec3 n = (particle->pos - other_particle->pos) * (float)(1.0/r); 
            f_p += n * (float)pressure; 
        }
    }
    return f_p; 
}

glm::vec3 compute_viscosity(Particle *particle, int max_particles, Particle particlesContainer[]) {
    glm::vec3 f_v = glm::vec3(0.0, 0.0, 0.0); 
    const int h = 1;
    const float mu =  0.01; // viscosity of water at 20 degrees
    for (size_t i = 0; i < max_particles; i++) {
        Particle *other_particle = &particlesContainer[i];
        const float r = dist(other_particle->pos, particle->pos); 
        glm::vec3 viscosity = -other_particle->mass*(other_particle->vel- particle->vel)/other_particle->density * laplace_viscosity(r, h); 
        f_v += viscosity;
    }
    f_v *= mu;
    return f_v;
}

glm::vec3 compute_gravity(Particle particle) {
    const int volume = 1;
    const float mass = particle.density * volume; 
    return glm::vec3(0.0, -9.8 * mass, 0.0); 
}

glm::vec3 compute_surface_tension(Particle *particle, int max_particles, Particle particlesContainer[]) {
    glm::vec3 ft = glm::vec3(0.0, 0.0, 0.0); 
    glm::vec3 c = glm::vec3(0.0, 0.0, 0.0); 
    float k = 0.0;

    // https://www.engineeringtoolbox.com/surface-tension-d_962.html
    const float sigma = 0.0728 / 10; // signam surface tension of water N/m at 20 degrees
    const float h = 1.0;
    for (size_t i = 0; i < max_particles; i++) {
        Particle *other_particle = &particlesContainer[i];
        const float r = dist(particle->pos, other_particle->pos); 
        const float surface_tension = other_particle->mass * 1/(other_particle->density) * w_poly_derivative6(r, h); 
        if(r != 0) {
          const glm::vec3 n = (particle->pos - other_particle->pos) * (float)(1.0/r); 
          c += surface_tension * n;
          k +=  other_particle->mass * 1/(other_particle->density) * w_poly_laplacian(r, h)/r; 
      }
    }

    ft = sigma * c * k;
    return ft; 
}


