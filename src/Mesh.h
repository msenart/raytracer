#pragma once

#include <utility>

#include "OpenGLIncludes.h"
#include "Shader.h"
#include "MeshHelpers.h"
#include "Transform.h"
#include <array>

enum struct MaterialType : unsigned int {
    Light = 0,
    Lambertian = 1,
    Metallic = 2,
    Glass = 3,
};

struct Material {
    std::array<float, 16> albedo{};
    MaterialType id;
    float metal_fuzz;
    float T;
    float intensity = 0;
    glm::vec3 B;
    float eta_test = 1;
    glm::vec3 C;
    float _pad2 = 0;

    Material() :    id(MaterialType::Lambertian),
                    metal_fuzz(0.0f),
                    T(293),
                    B(1.0f),
                    C(1.0f) {
        albedo.fill(1.f);
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