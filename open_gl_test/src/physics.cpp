#include "math.h"
#include "particle.h"
#include "physics.h"
#include <iostream>
#include "point_flann_adapter.h"

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
    const float h = 1.0f; 
    float p = 0; 
    for (size_t i = 0; i < max_particles; i++) {
        Particle *other_particle = &particlesContainer[i];
        int r = dist(other_particle->pos, particle->pos); 
        p += other_particle->mass * w_poly(r, h); 
    }
    particle->density = p; 
    return p; 
}

glm::vec3 compute_pressure(Particle *particle, int max_particles, Particle particlesContainer[], Points_KD_Tree_t *index) {
    glm::vec3 f_p = glm::vec3(0.0, 0.0, 0.0);
    const float k = 1, h = 1, rho_zero = 1; 
    const float pi = k * (particle->density - rho_zero); 

	// Perform Knn Search
	const unsigned int num_results = 32;
	std::vector<size_t> ret_indexes(num_results);
	std::vector<float> out_dists_sqr(num_results);
	nanoflann::KNNResultSet<float> resultSet(num_results);
	resultSet.init(&ret_indexes[0], &out_dists_sqr[0]);

	float test_pt[3] = {particle->pos.x, particle->pos.y, particle->pos.z};
	index->findNeighbors(resultSet, &test_pt[0],
			nanoflann::SearchParams(10));
    
	for (size_t i = 0; i < num_results; i++) {
		Particle *other_particle = &particlesContainer[ret_indexes[i]];
        const float pj = k * (other_particle->density - rho_zero); 
        const float r = dist(other_particle->pos, particle->pos); 
        const float pressure = -other_particle->mass*(pi + pj)/(2*other_particle->density) * w_spiky_derivative(r, h); 
        if (r != 0) {
            const glm::vec3 n = (particle->pos - other_particle->pos) * (float)(1.0/r); 
            f_p += n * (float)pressure; 
        }
	}


    // for (size_t i = 0; i < max_particles; i++) {
        // Particle *other_particle = &particlesContainer[i];
        // const float pj = k * (other_particle->density - rho_zero); 
        // const float r = dist(other_particle->pos, particle->pos); 
        // const float pressure = -other_particle->mass*(pi + pj)/(2*other_particle->density) * w_spiky_derivative(r, h); 
        // if (r != 0) {
        //     const glm::vec3 n = (particle->pos - other_particle->pos) * (float)(1.0/r); 
        //     f_p += n * (float)pressure; 
        // }
    // }



    return f_p; 
}

glm::vec3 compute_viscosity(Particle *particle, int max_particles, Particle particlesContainer[], Points_KD_Tree_t *index) {
    glm::vec3 f_v = glm::vec3(0.0, 0.0, 0.0); 
    const float h = 1;
    const float mu =  0.01; // viscosity of water at 20 degrees
    	// Perform Knn Search
	const unsigned int num_results = 32;
	std::vector<size_t> ret_indexes(num_results);
	std::vector<float> out_dists_sqr(num_results);
	nanoflann::KNNResultSet<float> resultSet(num_results);
	resultSet.init(&ret_indexes[0], &out_dists_sqr[0]);

	float test_pt[3] = {particle->pos.x, particle->pos.y, particle->pos.z};
	index->findNeighbors(resultSet, &test_pt[0],
			nanoflann::SearchParams(10));
    
	for (size_t i = 0; i < num_results; i++) {
		Particle *other_particle = &particlesContainer[ret_indexes[i]];
        const float r = dist(other_particle->pos, particle->pos); 
        glm::vec3 viscosity = -other_particle->mass*(other_particle->vel - particle->vel)/other_particle->density * laplace_viscosity(r, h); 
        f_v += viscosity;
    }

    // for (size_t i = 0; i < max_particles; i++) {
    //     Particle *other_particle = &particlesContainer[i];
    //     const float r = dist(other_particle->pos, particle->pos); 
    //     glm::vec3 viscosity = -other_particle->mass*(other_particle->vel - particle->vel)/other_particle->density * laplace_viscosity(r, h); 
    //     f_v += viscosity;
    // }
    f_v *= mu;
    return f_v;
}

glm::vec3 compute_gravity(Particle *particle) {
    const float volume = 1;
    const float mass = particle->density * volume; 
    return glm::vec3(0.0, -9.8 * mass, 0.0); 
}

glm::vec3 compute_surface_tension(Particle *particle, int max_particles, Particle particlesContainer[], Points_KD_Tree_t *index) {
    glm::vec3 ft = glm::vec3(0.0, 0.0, 0.0); 
    glm::vec3 c = glm::vec3(0.0, 0.0, 0.0); 
    float k = 0.0;

    // https://www.engineeringtoolbox.com/surface-tension-d_962.html
    const float sigma = 0.0728 ; // signam surface tension of water N/m at 20 degrees
    const float h = 1.0;

	const unsigned int num_results = 32;
	std::vector<size_t> ret_indexes(num_results);
	std::vector<float> out_dists_sqr(num_results);
	nanoflann::KNNResultSet<float> resultSet(num_results);
	resultSet.init(&ret_indexes[0], &out_dists_sqr[0]);

	float test_pt[3] = {particle->pos.x, particle->pos.y, particle->pos.z};
	index->findNeighbors(resultSet, &test_pt[0],
			nanoflann::SearchParams(10));
    
	for (size_t i = 0; i < num_results; i++) {
		Particle *other_particle = &particlesContainer[ret_indexes[i]];
        const float r = dist(particle->pos, other_particle->pos); 
        const float surface_tension = other_particle->mass * 1/(other_particle->density) * w_poly_derivative6(r, h); 
        if(r != 0) {
          const glm::vec3 n = (particle->pos - other_particle->pos) * (float)(1.0/r); 
          c += surface_tension * n;
          k +=  other_particle->mass * 1/(other_particle->density) * w_poly_laplacian(r, h)/r; 
      }
    }


    // for (size_t i = 0; i < max_particles; i++) {
    //     Particle *other_particle = &particlesContainer[i];
    //     const float r = dist(particle->pos, other_particle->pos); 
    //     const float surface_tension = other_particle->mass * 1/(other_particle->density) * w_poly_derivative6(r, h); 
    //     if(r != 0) {
    //       const glm::vec3 n = (particle->pos - other_particle->pos) * (float)(1.0/r); 
    //       c += surface_tension * n;
    //       k +=  other_particle->mass * 1/(other_particle->density) * w_poly_laplacian(r, h)/r; 
    //   }
    // }

    ft = sigma * c * k;
    return ft; 
}


void updatePosition(int max_particles, Particle particlesContainer[]) {
    const float delta_t = 1.0/60.0;

	PointsAdaptor f_adaptor(particlesContainer, max_particles);
	Points_KD_Tree_t index(3, f_adaptor,nanoflann::KDTreeSingleIndexAdaptorParams(10));
	index.buildIndex();


    // Overall density might be constant but the density per particle is actually different
    #pragma omp parallel for
    for(int i = 0; i < max_particles; i++) {
        density_s(&particlesContainer[i], max_particles, particlesContainer);
    }

    // force calculation
    #pragma omp parallel for
    for(int i = 0; i < max_particles; i++) {
        glm::vec3 pressure_force = compute_pressure(&particlesContainer[i], max_particles, particlesContainer, &index);
        particlesContainer[i].pressure_force = pressure_force;
    }
    
    #pragma omp parallel for
    for(int i = 0; i < max_particles; i++) {
        glm::vec3 gravity_force = compute_gravity(&particlesContainer[i]);
        particlesContainer[i].external = gravity_force;
    }
    #pragma omp parallel for
    for(int i = 0; i < max_particles; i++) {
        glm::vec3 surface_tension_force = compute_surface_tension(&particlesContainer[i], max_particles, particlesContainer, &index);
        particlesContainer[i].surface_tension = surface_tension_force;
    }

    #pragma omp parallel for
    for(int i = 0; i < max_particles; i++) {
        glm::vec3 viscosity_force = compute_viscosity(&particlesContainer[i], max_particles, particlesContainer, &index);
        particlesContainer[i].viscosity = viscosity_force;
    }

    // Actually updating the points
    #pragma omp parallel for
    for(int i = 0; i < max_particles; i++) {
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

        // const net_force = vvadd_multiple([particle.pressure_force, particle.viscosity_force, particle.gravity_force, particle.surface_tension])
        // const next_acc = vmuls(net_force, 1/particle_mass * delta_t * delta_t)
        // var new_vel = vvadd(vel, vmuls(next_acc, delta_t))
        // var new_pos = vvadd(pos, vmuls(new_vel, delta_t))
        
    }
}