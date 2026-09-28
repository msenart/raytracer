#pragma once

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

    void uploadUniforms(const Shader& shader) const;

    Material material;
    Transform transform;
    glm::vec4 color;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};