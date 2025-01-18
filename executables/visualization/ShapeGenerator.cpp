#include "ShapeGenerator.hpp"

#include <ext/scalar_constants.hpp>

//======================================================================================================================

std::vector<glm::vec3> ShapeGenerator::Cylinder(float const radius, float const height, int const segments)
{
    std::vector<glm::vec3> vertices;

    float const anglePerSegment = 2.0f * glm::pi<float>() / (float)segments;
    float const halfHeight = height * 0.5f;

    for (int i = 0; i <= segments; i++)
    {
        float const angle = (float)i * anglePerSegment;
        float const x = radius * cos(angle);
        float const y = radius * sin(angle);

        // Bottom circle
        vertices.emplace_back(x, y, -halfHeight);

        // Top circle
        vertices.emplace_back(x, y, +halfHeight);
    }

    return vertices;
}

//======================================================================================================================

std::vector<glm::vec3> ShapeGenerator::Sphere(float radius, int const slices, int const stacks)
{
    std::vector<glm::vec3> vertices;

    // Top cap
    vertices.emplace_back(0.0f, radius, 0.0f); // North pole
    for (int i = 0; i <= slices; ++i) {
        float theta = 2.0f * glm::pi<float>() * (float)i / (float)slices;
        float x = radius * sin(0.0f) * cos(theta);
        float y = radius * cos(0.0f);
        float z = radius * sin(0.0f) * sin(theta);

        vertices.emplace_back(x, y, z);
    }

    // Middle part of the sphere
    for (int i = 1; i < stacks; ++i) {
        float const phi = glm::pi<float>() * (float)i / (float)stacks; // Polar angle
        for (int j = 0; j <= slices; ++j) {
            float const theta = 2.0f * glm::pi<float>() * (float)j / (float)slices; // Azimuthal angle
            float const x = radius * sin(phi) * cos(theta);
            float const y = radius * cos(phi);
            float const z = radius * sin(phi) * sin(theta);

            vertices.emplace_back(x, y, z);
        }
    }

    // Bottom cap
    vertices.emplace_back(0.0f, -radius, 0.0f); // South pole
    for (int i = 0; i <= slices; ++i) {
        float const theta = 2.0f * glm::pi<float>() * (float)i / (float)slices;
        float x = radius * sin(glm::pi<float>()) * cos(theta);
        float y = radius * cos(glm::pi<float>());
        float z = radius * sin(glm::pi<float>()) * sin(theta);
        float u = (float)i / (float)slices;
        float v = 0.0f;
        float nx = x / radius;
        float ny = y / radius;
        float nz = z / radius;

        vertices.emplace_back(x, y, z);
    }

    return vertices;
}

//======================================================================================================================
