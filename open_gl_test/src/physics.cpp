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

float density_s(Particle *particle, Particle particlesContainer[], Points_KD_Tree_t *index) {
    float p = 0; 
    // if there is a lot of particles try to add this as the density
    if(MAX_PARTICLES > 1024 * 4) {
        std::vector<size_t> ret_indexes(1024);
        std::vector<float> out_dists_sqr(1024);
        nanoflann::KNNResultSet<float> resultSet(1024);
        resultSet.init(&ret_indexes[0], &out_dists_sqr[0]);

        float test_pt[3] = {particle->pos.x, particle->pos.y, particle->pos.z};
        index->findNeighbors(resultSet, &test_pt[0],
                nanoflann::SearchParams(10));
        
            for (size_t i = 0; i < 1024; i++) {
                Particle *other_particle = &particlesContainer[ret_indexes[i]];
                int r = dist(other_particle->pos, particle->pos); 
                p += other_particle->mass * w_poly(r); 
        }
    } else {
        for (size_t i = 0; i < MAX_PARTICLES; i++) {
            Particle *other_particle = &particlesContainer[i];
            int r = dist(other_particle->pos, particle->pos); 
            p += other_particle->mass * w_poly(r); 
        }
    }

    particle->density = p; 
    return p; 
}

glm::vec3 compute_force(Particle *particle, Particle particlesContainer[], Points_KD_Tree_t *index, ExternalForce *external_force) {
    glm::vec3 f_p = glm::vec3(0.0, 0.0, 0.0);
    glm::vec3 f_v = glm::vec3(0.0, 0.0, 0.0);
    // surface tension constants 
    glm::vec3 ft = glm::vec3(0.0, 0.0, 0.0); 
    glm::vec3 c = glm::vec3(0.0, 0.0, 0.0); 
    float k = 0.0; // regular accumulator k

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
        const float r = dist(other_particle->pos, particle->pos); 
        if (r > 2) continue;
        const float pj = pressure_constant_k * (other_particle->density - pressure_rho_zero); 
        const float pressure = -other_particle->mass*(pi + pj)/(2*other_particle->density) * w_spiky_derivative(r); 
        if (r != 0) {
            //pressure
            const glm::vec3 n = (particle->pos - other_particle->pos) * (float)(1.0/r); 
            f_p += n * (float)pressure; 
            // surface tension
            const float surface_tension = other_particle->mass * 1/(other_particle->density) * w_poly_derivative6(r); 
            c += surface_tension * n;
            k +=  other_particle->mass * 1/(other_particle->density) * w_poly_laplacian(r)/r; 
        }
        
        glm::vec3 viscosity = -other_particle->mass*(other_particle->vel - particle->vel)/other_particle->density * laplace_viscosity(r); 
        f_v += viscosity;
	}
    const float volume = 1;
    const float mass = particle->density * volume; 
    glm::vec3 f_e = glm::vec3(0.0, -9.8 * mass, 0.0); 
    ft = surface_tension_sigma * c * k;
    
    if(external_force->external_mouse_force_enabled ) {
        double r = glm::dot(particle->pos, external_force->current_starting_force)/(glm::length(external_force->current_starting_force) * glm::length(particle->pos));
        // double r = glm::length(glm::normalize(particle->pos) - glm::normalize(external_force->current_starting_force));
        glm::vec3 n = external_force->current_dxdy;
        if (r < .3) {
            // glm::vec3 n = glm::vec3(external_force->current_dxdy, 1.0);
            f_e +=  n * mass * 10.0f;  
        }
    }

    if(whirlpool_enabled) {
        glm::vec3 axis = glm::normalize(glm::vec3(1, 0, 0));
        float r = glm::dot(particle->pos, axis);
        glm::vec3 n = glm::vec3(1, 0, 1);
        if (glm::abs(r) > 0.00001){
            f_e += 1.0f * n * mass*r * glm::dot(particle->vel, particle->vel);
        }
    }

    return f_p + f_v + f_e + ft; 
}

// O(n^2)
// 
void updatePosition(Particle particlesContainer[], ExternalForce *external_force) {

	PointsAdaptor f_adaptor(particlesContainer, MAX_PARTICLES);
	Points_KD_Tree_t index(3, f_adaptor,nanoflann::KDTreeSingleIndexAdaptorParams(10));
	index.buildIndex();


    // Overall density might be constant but the density per particle is actually different
    #pragma omp parallel for
    for(int i = 0; i < MAX_PARTICLES; i++) {
        density_s(&particlesContainer[i], particlesContainer, &index);
    }

    // force calculation
    #pragma omp parallel for
    for(int i = 0; i < MAX_PARTICLES; i++) {
        glm::vec3 net_force = compute_force(&particlesContainer[i], particlesContainer, &index, external_force);
        particlesContainer[i].net_force = net_force;
    }
    if(external_force->num_iterations != 0) {
        external_force->num_iterations -= 1;
    } else {
        external_force->external_mouse_force_enabled = false;
    }
    // #pragma omp parallel for
    // for(int i = 0; i < MAX_PARTICLES; i++) {
    //     glm::vec3 gravity_force = compute_gravity(&particlesContainer[i]);
    //     particlesContainer[i].external = gravity_force;
    // }
    // #pragma omp parallel for
    // for(int i = 0; i < MAX_PARTICLES; i++) {
    //     glm::vec3 surface_tension_force = compute_surface_tension(&particlesContainer[i], particlesContainer, &index);
    //     particlesContainer[i].surface_tension = surface_tension_force;
    // }

    // #pragma omp parallel for
    // for(int i = 0; i < MAX_PARTICLES; i++) {
    //     glm::vec3 viscosity_force = compute_viscosity(&particlesContainer[i], particlesContainer, &index);
    //     particlesContainer[i].viscosity = viscosity_force;
    // }

    // Actually updating the points
    #pragma omp parallel for
    for(int i = 0; i < MAX_PARTICLES; i++) {
        Particle *particle = &particlesContainer[i];
        // glm::vec3 net_force = particle->pressure_force + particle->external + particle->external + particle->viscosity;
        glm::vec3 net_force = particle->net_force;
        const float volume = 1;
        const float mass = particle->density * volume; 
        
        // Implicit Euler to compute next time step first
        glm::vec3 new_vel = particle->vel + net_force/mass * delta_t;
        new_vel = glm::clamp(new_vel, glm::vec3(-8, -8, -8), glm::vec3(8, 8, 8));

        glm::vec3 new_pos = particle->pos + new_vel * delta_t;
        // printf("Netforce %f, %f,%f\n", (net_force/mass * delta_t).x,  (net_force/mass * delta_t).y,  (net_force/mass).z);

        particlesContainer[i].vel = new_vel;
        particlesContainer[i].pos = new_pos;
        
        // printf("%f, %f, %f\n", new_vel.x, new_vel.y, new_vel.z);
        if(particle->pos.x < -1.5) {
            particle->pos.x = -1.5f;
            particle->vel.x *= -1*dampening;
        } 
        if(particle->pos.x > 1.5) {
            particle->pos.x = 1.5f;
            particle->vel.x *= -1*dampening;
        } 
        if(particle->pos.y < -1.5) {
            particle->pos.y = -1.5f;
            particle->vel.y *= -1*dampening;
        }
        if(particle->pos.y > 1.5) {
            particle->pos.y = 1.5f;
            particle->vel.y *= -1*dampening;
        } 
        if(particle->pos.z < -1) {
            particle->pos.z = -1.0f;
            particle->vel.z *= -1*dampening;
        }
        if(particle->pos.z > 1) {
            particle->pos.z = 1.0f;
            particle->vel.z *= -1*dampening;
        }

    }
}