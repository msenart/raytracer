#pragma once

#include "OpenGLIncludes.h"
#include "MeshHelpers.h"
#include "Transform.h"
#include <array>
#include "Parameters.h"

enum struct MaterialType : unsigned int {
    Light = 0,
    Lambertian = 1,
    Metallic = 2,
    Glass = 3,
};

struct Material {
    std::array<float, 16> albedo{};
    MaterialType id;
    float metal_fuzz = 0.f;
    float T = 293.f;
    float intensity = 0;
    std::array<float, 16> n{};
    std::array<float, 16> k{};

    Material() : id(MaterialType::Lambertian){
        albedo.fill(1.f);
        n.fill(1.f);
        k.fill(0.f);
    }
};

struct Mesh {
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Material material);
    Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Material material, Transform transform);

    Material material;
    Transform transform;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct Sphere {
    glm::vec3 pos;
    float radius;
    Material material;

    Sphere(glm::vec3 pos, float radius, Material material);
    Sphere(glm::vec3 pos, float radius);
};