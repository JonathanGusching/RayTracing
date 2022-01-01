#ifndef RAY_HPP
#define RAY_HPP

#include <glm/glm.hpp>

// simple struct for a Ray.
struct Ray
{
	glm::vec3 direction;
	glm::vec3 position;
};

#endif