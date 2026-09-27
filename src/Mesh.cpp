#include "Mesh.h"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    : vertices(std::move(vertices)), indices(std::move(indices)){

}

void Mesh::uploadUniforms(const Shader& shader) const {
}

