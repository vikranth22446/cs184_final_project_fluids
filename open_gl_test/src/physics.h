#ifndef FLUID_PHYSICS_H
#define FLUID_PHYSICS_H
#include "particle.h"
#include "glm/glm.hpp"
#include "nanoflann.h"
#include "point_flann_adapter.h"
typedef nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<float, PointsAdaptor>,PointsAdaptor, 3> Points_KD_Tree_t;

float dist(glm::vec3 pos1, glm::vec3 pos2);
float w_poly(float r);
float w_poly_derivative6(float r);
float w_poly_laplacian(float r);
float w_viscosity(float r);
float laplace_viscosity(float r);
float w_spiky(float r);
float w_spiky_derivative(float r);
float density_s(Particle *particle, Particle particlesContainer[], Points_KD_Tree_t *index);
glm::vec3 compute_pressure(Particle *particle, Particle particlesContainer[], Points_KD_Tree_t *index);
glm::vec3 compute_viscosity(Particle *particle, Particle particlesContainer[], Points_KD_Tree_t *index);
glm::vec3 compute_gravity(Particle *particle);
glm::vec3 compute_surface_tension(Particle *particle, Particle particlesContainer[], Points_KD_Tree_t *index);
void updatePosition(Particle particlesContainer[]);

#endif