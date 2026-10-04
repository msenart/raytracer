#include "Mesh.h"
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices): vertices(std::move(vertices)), indices(std::move(indices)){}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Material material): vertices(std::move(vertices)), indices(std::move(indices)), material(material) {}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Material material, Transform transform): vertices(std::move(vertices)), indices(std::move(indices)), material(material), transform(transform) {}

Sphere::Sphere(glm::vec3 pos, float radius, Material material): pos(pos), radius(radius), material(material) {}

Sphere::Sphere(glm::vec3 pos, float radius): pos(pos), radius(radius), material(Material()) {}

