#include "Mesh.h"

float AABB::distance(const AABB &a, const AABB &b) {
    const glm::vec3 gap = glm::max(glm::vec3(0.f),
                                   glm::max(a.min - b.max, b.min - a.max));
    return glm::length(gap);
}

AABB AABB::merge(const AABB &aabb1, const AABB &aabb2) {
    const auto min = glm::vec3(
            std::min({aabb1.min.x, aabb2.min.x}),
            std::min({aabb1.min.y, aabb2.min.y}),
            std::min({aabb1.min.z, aabb2.min.z}));
    const auto max = glm::vec3(
        std::max({aabb1.max.x, aabb2.max.x}),
        std::max({aabb1.max.y, aabb2.max.y}),
        std::max({aabb1.max.z, aabb2.max.z}));
    return {min, max};
}

float AABB::surfaceArea() const {
    const glm::vec3 d = max - min;
    return 2.f * (d.x * d.y + d.y * d.z + d.z * d.x);
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices): vertices(std::move(vertices)), indices(std::move(indices)){}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Material material): vertices(std::move(vertices)), indices(std::move(indices)), material(material) {}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Material material, Transform transform): vertices(std::move(vertices)), indices(std::move(indices)), material(material), transform(transform) {}

std::vector<AABB> Mesh::aabb() const {
    auto out = std::vector<AABB>(indices.size()/3);
    for (unsigned int i = 0; i < indices.size()/3; i += 1) {
        auto v1 = vertices[indices[3*i]];
        auto v2 = vertices[indices[3*i+1]];
        auto v3 = vertices[indices[3*i+2]];
        auto min = glm::vec3(
                    std::min({v1.position.x, v2.position.x, v3.position.x}),
                    std::min({v1.position.y, v2.position.y, v3.position.y}),
                    std::min({v1.position.z, v2.position.z, v3.position.z}));
        auto max = glm::vec3(
                    std::max({v1.position.x, v2.position.x, v3.position.x}),
                    std::max({v1.position.y, v2.position.y, v3.position.y}),
                    std::max({v1.position.z, v2.position.z, v3.position.z}));
        out[i] = {min, max};
    }
    return out;
}

Sphere::Sphere(glm::vec3 pos, float radius, Material material): pos(pos), radius(radius), material(material) {}

Sphere::Sphere(glm::vec3 pos, float radius): pos(pos), radius(radius), material(Material()) {}

AABB Sphere::aabb() const {
    const glm::vec3 r(radius);
    return { -r,  r};
}
