#include "math.h"
#include "particle.h"
#include "physics.h"
#include <iostream>
#include "point_flann_adapter.h"

float dist(glm::vec3 pos1, glm::vec3 pos2) {
  return glm::length(pos1 - pos2);
}

float w_poly(float r) {
    if(r > h || r < 0 ) return 0;
    return 315/(64*M_PI * h_9) * pow((h_2 - pow(r, 2)), 3); 
}

float w_poly_derivative6(float r) {
    if(r > h || r < 0 ) return 0;
    return 315/(64*M_PI * h_9) * 3 * pow((h_2 - pow(r, 2)), 2) * (-2 * r); 
}

float w_poly_laplacian(float r) {
  if(r > h || r < 0 ) return 0;
  return 315/(64*M_PI * h_9) * 3 * pow((h_2 - pow(r, 2)), 2) * (-2) + 315/(64*M_PI * h_9) * 6 * pow((h_2 - pow(r, 2)), 1) * (-2 * r); 
}

float w_viscosity(float r) {
  if (r > h || r < 0) return 0; 
  return 15/(2 * M_PI * h_3) * (-pow(r, 3)/(2 * h_3)) * (pow(r, 2)/h_2) * h/(2*r) - 1; 
}

float laplace_viscosity(float r) {
  if (r > h || r < 0) return 0; 
  return 45/(M_PI * h_6) * (h-r); 
}

float w_spiky(float r) {
    if (r > h || r < 0) return 0; 
    return 15/(M_PI * h_6) * pow((h - r), 3); 
}

float w_spiky_derivative(float r) {
    if (r > h || r < 0) return 0; 
    return 15/(M_PI  *h_6) * 3 * pow((h - r), 2) * (-1); 
}

float density_s(Particle *particle, Particle particlesContainer[] ) {
    float p = 0; 
    for (size_t i = 0; i < MAX_PARTICLES; i++) {
        Particle *other_particle = &particlesContainer[i];
        int r = dist(other_particle->pos, particle->pos); 
        p += other_particle->mass * w_poly(r); 
    }
    particle->density = p; 
    return p; 
}

glm::vec3 compute_pressure(Particle *particle, Particle particlesContainer[], Points_KD_Tree_t *index) {
    glm::vec3 f_p = glm::vec3(0.0, 0.0, 0.0);
    const float pi = pressure_constant_k * (particle->density - pressure_rho_zero); 

	// Perform Knn Search
	std::vector<size_t> ret_indexes(num_kd_results);
	std::vector<float> out_dists_sqr(num_kd_results);
	nanoflann::KNNResultSet<float> resultSet(num_kd_results);
	resultSet.init(&ret_indexes[0], &out_dists_sqr[0]);

	float test_pt[3] = {particle->pos.x, particle->pos.y, particle->pos.z};
	index->findNeighbors(resultSet, &test_pt[0],
			nanoflann::SearchParams(10));
    
	for (size_t i = 0; i < num_kd_results; i++) {
		Particle *other_particle = &particlesContainer[ret_indexes[i]];
        const float pj = pressure_constant_k * (other_particle->density - pressure_rho_zero); 
        const float r = dist(other_particle->pos, particle->pos); 
        const float pressure = -other_particle->mass*(pi + pj)/(2*other_particle->density) * w_spiky_derivative(r); 
        if (r != 0) {
            const glm::vec3 n = (particle->pos - other_particle->pos) * (float)(1.0/r); 
            f_p += n * (float)pressure; 
        }
	}

    return f_p; 
}

glm::vec3 compute_viscosity(Particle *particle, Particle particlesContainer[], Points_KD_Tree_t *index) {
    glm::vec3 f_v = glm::vec3(0.0, 0.0, 0.0); 
    	// Perform Knn Search
	std::vector<size_t> ret_indexes(num_kd_results);
	std::vector<float> out_dists_sqr(num_kd_results);
	nanoflann::KNNResultSet<float> resultSet(num_kd_results);
	resultSet.init(&ret_indexes[0], &out_dists_sqr[0]);

	float test_pt[3] = {particle->pos.x, particle->pos.y, particle->pos.z};
	index->findNeighbors(resultSet, &test_pt[0],
			nanoflann::SearchParams(10));
    
	for (size_t i = 0; i < num_kd_results; i++) {
		Particle *other_particle = &particlesContainer[ret_indexes[i]];
        const float r = dist(other_particle->pos, particle->pos); 
        glm::vec3 viscosity = -other_particle->mass*(other_particle->vel - particle->vel)/other_particle->density * laplace_viscosity(r); 
        f_v += viscosity;
    }

    f_v *= viscosity_mu;
    return f_v;
}

glm::vec3 compute_gravity(Particle *particle) {
    const float volume = 1;
    const float mass = particle->density * volume; 
    return glm::vec3(0.0, -9.8 * mass, 0.0); 
}

glm::vec3 compute_surface_tension(Particle *particle, Particle particlesContainer[], Points_KD_Tree_t *index) {
    glm::vec3 ft = glm::vec3(0.0, 0.0, 0.0); 
    glm::vec3 c = glm::vec3(0.0, 0.0, 0.0); 
    float k = 0.0; // regular accumulator k

	std::vector<size_t> ret_indexes(num_kd_results);
	std::vector<float> out_dists_sqr(num_kd_results);
	nanoflann::KNNResultSet<float> resultSet(num_kd_results);
	resultSet.init(&ret_indexes[0], &out_dists_sqr[0]);

	float test_pt[3] = {particle->pos.x, particle->pos.y, particle->pos.z};
	index->findNeighbors(resultSet, &test_pt[0],
			nanoflann::SearchParams(10));
    
	for (size_t i = 0; i < num_kd_results; i++) {
		Particle *other_particle = &particlesContainer[ret_indexes[i]];
        const float r = dist(particle->pos, other_particle->pos); 
        const float surface_tension = other_particle->mass * 1/(other_particle->density) * w_poly_derivative6(r); 
        if(r != 0) {
          const glm::vec3 n = (particle->pos - other_particle->pos) * (float)(1.0/r); 
          c += surface_tension * n;
          k +=  other_particle->mass * 1/(other_particle->density) * w_poly_laplacian(r)/r; 
      }
    }
    ft = surface_tension_sigma * c * k;
    return ft; 
}


void updatePosition(Particle particlesContainer[]) {
	PointsAdaptor f_adaptor(particlesContainer, MAX_PARTICLES);
	Points_KD_Tree_t index(3, f_adaptor,nanoflann::KDTreeSingleIndexAdaptorParams(10));
	index.buildIndex();


    // Overall density might be constant but the density per particle is actually different
    #pragma omp parallel for
    for(int i = 0; i < MAX_PARTICLES; i++) {
        density_s(&particlesContainer[i], particlesContainer);
    }

    // force calculation
    #pragma omp parallel for
    for(int i = 0; i < MAX_PARTICLES; i++) {
        glm::vec3 pressure_force = compute_pressure(&particlesContainer[i], particlesContainer, &index);
        particlesContainer[i].pressure_force = pressure_force;
    }
    
    #pragma omp parallel for
    for(int i = 0; i < MAX_PARTICLES; i++) {
        glm::vec3 gravity_force = compute_gravity(&particlesContainer[i]);
        particlesContainer[i].external = gravity_force;
    }
    #pragma omp parallel for
    for(int i = 0; i < MAX_PARTICLES; i++) {
        glm::vec3 surface_tension_force = compute_surface_tension(&particlesContainer[i], particlesContainer, &index);
        particlesContainer[i].surface_tension = surface_tension_force;
    }

    #pragma omp parallel for
    for(int i = 0; i < MAX_PARTICLES; i++) {
        glm::vec3 viscosity_force = compute_viscosity(&particlesContainer[i], particlesContainer, &index);
        particlesContainer[i].viscosity = viscosity_force;
    }

    // Actually updating the points
    #pragma omp parallel for
    for(int i = 0; i < MAX_PARTICLES; i++) {
        Particle *particle = &particlesContainer[i];
        glm::vec3 net_force = particle->pressure_force + particle->external + particle->external + particle->viscosity;
        
        const float volume = 1;
        const float mass = particle->density * volume; 
        
        // Implicit Euler to compute next time step first
        glm::vec3 new_vel = particle->vel + net_force/mass * delta_t;
        glm::vec3 new_pos = particle->pos + new_vel * delta_t;
        // printf("Netforce %f, %f,%f\n", (net_force/mass * delta_t).x,  (net_force/mass * delta_t).y,  (net_force/mass).z);

        particlesContainer[i].vel = new_vel;
        particlesContainer[i].pos = new_pos;

        if(particle->pos.x < -1.5) {
            particle->pos.x = -1.5f;
            particle->vel.x *= -1*.8;
        } 
        if(particle->pos.x > 1.5) {
            particle->pos.x = 1.5f;
            particle->vel.x *= -1*.8;
        } 
        if(particle->pos.y < -1.5) {
            particle->pos.y = -1.5f;
            particle->vel.y *= -1*.8;
        }
        if(particle->pos.y > 1) {
            particle->pos.y = 1.0f;
            particle->vel.y *= -1*.8;
        } 
        if(particle->pos.z < -1) {
            particle->pos.z = -1.0f;
            particle->vel.z *= -1*.8;
        }
        if(particle->pos.z > 1) {
            particle->pos.z = 1.0f;
            particle->vel.z *= -1*.8;
        }


    }
}