#include "particle.h"
#include "PointCloudAdapter.h"
#include "nanoflann.h"
#include "glm/glm.hpp"
// struct ParticleContainer{
// 	Particle *particleContainer;
// };
void createPointCloud(Particle particleContainer[]) {
	PointsAdaptor f_adaptor(particleContainer, 4);

	typedef nanoflann::KDTreeSingleIndexAdaptor<nanoflann::L2_Simple_Adaptor<float, PointsAdaptor>,PointsAdaptor, 3> Points_KD_Tree_t;
	Points_KD_Tree_t index(3, f_adaptor,nanoflann::KDTreeSingleIndexAdaptorParams(10));
	index.buildIndex();


	// Perform Knn Search
	const unsigned int num_results = 32;
	// size_t ret_index;
	std::vector<size_t> ret_indexes(num_results);
	std::vector<float> out_dists_sqr(num_results);
	nanoflann::KNNResultSet<float> resultSet(num_results);
	resultSet.init(&ret_indexes[0], &out_dists_sqr[0]);
	float test_pt[3] = {.5, .5, -1};
	index.findNeighbors(resultSet, &test_pt[0],
			nanoflann::SearchParams(10));
	for (size_t i = 0; i < num_results; i++) {
		std::cout << "Nearest Vertex: "
				<< particleContainer[ret_indexes[i]].pos.x << ", "
			<< particleContainer[ret_indexes[i]].pos.y << ", "
			<< particleContainer[ret_indexes[i]].pos.z << '\n';
	}

	std::cout << "\n";
	float test_pt2[3] = {1.0, 1.0, 2.0};
	index.findNeighbors(resultSet, &test_pt2[0],
			nanoflann::SearchParams(10));
	for (size_t i = 0; i < num_results; i++) {
		std::cout << "Nearest Vertex: "
				<< particleContainer[ret_indexes[i]].pos.x << ", "
			<< particleContainer[ret_indexes[i]].pos.y << ", "
			<< particleContainer[ret_indexes[i]].pos.z << '\n';
	}

}


void testPointCloud() {
Particle particleContainer[4];
Particle p1;
p1.pos = glm::vec3(1.0, 1.0, 1.0);
particleContainer[0] = p1;

Particle p2;
p2.pos = glm::vec3(-1.0, -1.0, -1.0);
particleContainer[1] = p2;

Particle p3;
p3.pos = glm::vec3(.5, .5, .5);
particleContainer[2] = p3;

Particle p4;
p4.pos = glm::vec3(1.0, 0.0, 0.0);
particleContainer[3] = p4;
// glm::vec3 test = glm::vec3(.5, .5, -1);

// std::cout << "Nearest Vertex: "
// 	<< glm::length(test- p1.pos) << ", "
// 	<< glm::length(test- p2.pos) << ", "
// 	<< glm::length(test- p3.pos) << ", "
// 	<< glm::length(test- p4.pos) << '\n';
createPointCloud(particleContainer);
}