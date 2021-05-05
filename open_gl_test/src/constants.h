#ifndef FLUID_CONSTANTS_H
#define FLUID_CONSTANTS_H
#include <string>
#include "glm/glm.hpp"

const int MAX_PARTICLES = 4000;

// https://www.engineeringtoolbox.com/surface-tension-d_962.html
// const float surface_tension_sigma =  0.0728;
const float surface_tension_sigma =  0.00728;
// h was chosen randomly
const float h = .5;
const float h_2 = h * h; // h^2
const float h_3 = h_2 * h; // h^3
const float h_6 = h_3 * h_3; // h^3
const float h_9 = h_6 * h_3; // h^9

// also chosen pretty randomly
const float pressure_rho_zero = 80.0f;
const float pressure_constant_k = .5; // probably the gas constant in the paper

// https://www.engineersedge.com/physics/water__density_viscosity_specific_weight_13146.htm
const float viscosity_mu = 1.0f;

const float particle_inital_mass = 0.0002f;

// chosen randomly
const unsigned int num_kd_results = 32;
// Update rate
const float delta_t = 1.0/60.0;

const float dampening = .65;
const float BOX_SIZE = 1.5;

const float sphere_radius = .05;
// TODO figure out better way to add const* strings instead of #define for type safety. 
#define vertex_shader_file "../src/shaders/Particle.vert"
#define fragment_shader_file  "../src/shaders/Mirror.frag"
#define texture_shader_file "../src/textures/texture2.jpeg"

// #define texture_shader_file "../src/textures/water_minecraft.jpeg"
// #define texture_shader_file "../src/textures/glass_with_water.jpeg"
// #define texture_shader_file "../src/textures/water_texture_3.jpeg"
#endif