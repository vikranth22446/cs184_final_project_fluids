#ifndef FLUID_CONSTANTS_H
#define FLUID_CONSTANTS_H
#include <string>
const int MAX_PARTICLES = 4000;

// https://www.engineeringtoolbox.com/surface-tension-d_962.html
const float surface_tension_sigma =  0.0728;
const float h = 1.0;
const float h_2 = 1.0; // h^2
const float h_3 = 1.0; // h^3
const float h_6 = 1.0; // h^3
const float h_9 = 1.0; // h^9

const float pressure_rho_zero = 1;
const float pressure_constant_k = 1; // probably the gas constant in the paper

const float viscosity_mu = .01;

const float particle_inital_mass = 0.08373333333333335;

const unsigned int num_kd_results = 32;
// Update rate
const float delta_t = 1.0/60.0;

const float BOX_SIZE = 1.0;

// TODO figure out better way to add const* strings instead of #define for type safety. 
#define vertex_shader_file "../src/shaders/Particle.vert"
#define fragment_shader_file  "../src/shaders/Mirror.frag"
#define texture_shader_file "../src/textures/texture2.jpeg"
#endif