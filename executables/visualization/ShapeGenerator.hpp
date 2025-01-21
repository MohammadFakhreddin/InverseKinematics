#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <tuple>

namespace ShapeGenerator
{
    using Vertices = std::vector<glm::vec3>;
    using Indices = std::vector<uint32_t>;
    using Normals = std::vector<glm::vec3>;
    using Mesh = std::tuple<Vertices, Indices, Normals>;

    Mesh Cylinder(float radius, float height, int segments);

    Mesh Sphere(float radius, int slices, int stacks);

};
