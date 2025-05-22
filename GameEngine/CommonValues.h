#ifndef COMMONVALUES_H
#define COMMONVALUES_H

#include <stdio.h>
#include "stb_image.h"
#include "BoundingBox.h"
#include "Triangle.h"
#include "Mesh.h"
#include "Camera.h"


#include <glm.hpp>
#include <iostream>

const int MAX_POINT_LIGHTS = 3;
const int MAX_SPOT_LIGHTS = 3;

inline std::ostream& operator<<(std::ostream& os, const glm::vec3& vec) {
    os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return os;
}


#endif // !1