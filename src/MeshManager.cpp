//
// Created by mathi on 27/09/2026.
//

#include "MeshManager.h"
#include "Mesh.h"
#include <utility>

MeshManager::MeshManager(std::vector<std::shared_ptr<Mesh>> meshes) : meshes(std::move(meshes)) {
}

MeshManager::MeshManager()= default;

void MeshManager::addMesh(std::shared_ptr<Mesh> mesh) {
    meshes.push_back(mesh);
}

void MeshManager::removeMesh(const unsigned int index) {
    meshes.erase(meshes.begin() + index);
}

void MeshManager::buildSSBOs() {

    struct IndexAndMeshID {
        unsigned int index;
        unsigned int mesh_id;
        float _pad0;
        float _pad1;

        IndexAndMeshID(unsigned int index, unsigned int mesh_id) : index(index), mesh_id(mesh_id) {}
    };

    // concatenating vertices and indices into one big array.
    auto vertices = std::vector<Vertex>();
    auto indices = std::vector<IndexAndMeshID>();
    auto materials = std::vector<Material>();
    unsigned int offset = 0;
    for (unsigned int i = 0; i < meshes.size(); i++) {
        for (const auto& vertex: meshes[i]->vertices) {
            Vertex v = vertex;
            v.position = glm::vec3(meshes[i]->transform.matrix() * glm::vec4(v.position,1.0f));
            v.normal = glm::normalize(glm::inverseTranspose(glm::mat3(meshes[i]->transform.matrix())) * v.normal);
            v.tangent = glm::vec4(glm::normalize(glm::mat3(meshes[i]->transform.matrix()) * glm::vec3(v.tangent)), v.tangent.w);
            vertices.push_back(v);
        }
        for (const auto& index: meshes[i]->indices) {
            indices.emplace_back(offset + index, i);
        }
        materials.push_back(meshes[i]->material);
        offset+=meshes[i]->vertices.size();
    }

    // modern way to use SSBO (just like VBO, VAO, etc...)
    glCreateBuffers(1, &vertices_ssbo);
    glNamedBufferData(vertices_ssbo, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertices_ssbo);

    glCreateBuffers(1, &indices_ssbo);
    glNamedBufferData(indices_ssbo, indices.size() * sizeof(IndexAndMeshID), indices.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indices_ssbo);

    glCreateBuffers(1, &material_ssbo);
    glNamedBufferData(material_ssbo, materials.size() * sizeof(Material), materials.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, material_ssbo);
};