#include "glm/glm.hpp"

float dist(glm::vec3 pos1, glm::vec3 pos2); 
float w_poly(float r, float h); 
float w_poly_derivative6(float r, float h);
float w_poly_laplacian(float r, float h); 
float w_viscosity(float r, float h); 
float laplace_viscosity(float r, float h); 
float w_spiky(float r, float h); 
float w_spiky_derivative(float r, float h); 
float density_s(Particle particle, Particle[] particlesContainer); 
glm::vec3 compute_pressure(Particle particle, Particle[] particlesContainer); 
glm::vec3 compute_viscosity(Particle particle, Particle[] particlesContainer); 
glm::vec3 compute_gravity(Particle particle); 
glm::vec3 compute_surface_tension(Particle particle, Particle[] particlesContainer); 
