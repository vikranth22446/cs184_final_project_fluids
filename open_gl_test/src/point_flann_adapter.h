#ifndef POINT_FLANN_ADAPTER_H
#define POINT_FLANN_ADAPTER_H

#include "main.h"
#include "nanoflann.h"

class PointsAdaptor
{
public:
	PointsAdaptor(const Particle *particleContainer, int max_particles) : particleContainer(particleContainer), max_particles(max_particles) { }

	inline unsigned int kdtree_get_point_count() const
	{ return max_particles; }

	inline float kdtree_distance(const float* p1, const unsigned int index_2,
			unsigned int size) const
	{
		return glm::length(glm::vec3(p1[0], p1[1], p1[2]) -
				particleContainer[index_2].pos);
	}

	inline float kdtree_get_pt(const unsigned int i, int dim) const
	{
		if (dim == 0) return particleContainer[i].pos.x;
		else if (dim == 1) return particleContainer[i].pos.y;
		else return particleContainer[i].pos.z;
	}


	template <class BBOX>
	bool kdtree_get_bbox(BBOX&) const { return false; }

private:
	const Particle* particleContainer;
	const int max_particles;
};

#endif