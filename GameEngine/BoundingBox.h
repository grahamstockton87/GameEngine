#pragma once

#include <glm.hpp>

struct BoundingBox {
	glm::vec3 min;
	glm::vec3 max;
	void clear() {
		min.x = 0; min.y = 0; min.z = 0;
		max.x = 0; max.y = 0; max.z = 0;
	}
};