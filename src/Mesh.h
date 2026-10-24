#pragma once

#include "OpenGLIncludes.h"
#include "MeshHelpers.h"
#include "Transform.h"
#include <array>

#include "BVHNode.h"
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

    /// @brief returns the list of all the primitives' aabb in the world space.
    [[nodiscard]] std::vector<AABB> aabbs() const;

    /// @brief returns the aabb of the whole mesh in the world space.
    [[nodiscard]] AABB aabb() const;

    [[nodiscard]] BVH bvh(unsigned int offset) const;

    Material material;
    Transform transform;
    std::vector<BVHNode> bvh_nodes;
    std::vector<unsigned int> bvh_tree;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

private:
    static BVH buildBVH(const std::vector<std::pair<unsigned int, BVHNode>> &leaves, unsigned int first, unsigned int last);
};

struct Sphere {
    glm::vec3 pos;
    float radius;
    Material material;

    /// @brief returns the aabb of the sphere in the world space.
    [[nodiscard]] AABB aabb() const;

    Sphere(glm::vec3 pos, float radius, Material material);
    Sphere(glm::vec3 pos, float radius);

    [[nodiscard]] BVH bvh(unsigned int offset) const;
};