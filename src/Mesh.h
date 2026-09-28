#pragma once

#include <utility>

#include "OpenGLIncludes.h"
#include "Shader.h"
#include "MeshHelpers.h"
#include "Transform.h"

enum struct MaterialType : unsigned int {
    Light = 0,
    Lambertian = 1,
    Metallic = 2,
    Glass = 3,
};

struct Material {
    glm::vec4 color;
    MaterialType id;
    float metal_fuzz;
    float eta;
    float _pad0;

    Material() :
    color(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)),
    id(MaterialType::Lambertian),
    metal_fuzz(0.0f),
    eta(1.0f),
    _pad0(0)
    {

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