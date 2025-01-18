#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace ShapeGenerator
{
    std::vector<glm::vec3> Cylinder(float radius, float height, int segments);
    std::vector<glm::vec3> Sphere(float radius, int slices, int stacks);
};
