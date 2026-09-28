//
// Created by mathi on 27/09/2026.
//

#include "MeshManager.h"
#include "Mesh.h"
#include <utility>

MeshManager::MeshManager(std::vector<std::shared_ptr<Mesh>> meshes) : meshes(std::move(meshes)) {
}

MeshManager::MeshManager()= default;

void MeshManager::addMesh(const std::shared_ptr<Mesh> &mesh) {
    meshes.push_back(mesh);
}

void MeshManager::addSphere(std::shared_ptr<Sphere> sphere) {
    spheres.push_back(sphere);
}

void MeshManager::removeMesh(const unsigned int index) {
    meshes.erase(meshes.begin() + index);
}

void MeshManager::buildSSBOs() {
    // taking care of regular meshes
    struct IndexAndMeshID {
        unsigned int index;
        unsigned int mesh_id; // TODO this is supposed to be called material_id, this is also the case in shaders
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

    // taking care of sphere equations
    struct GPUSphere {
        glm::vec3 position;
        float radius;
        unsigned int material_id;
        float _pad0[3];

        GPUSphere(glm::vec3 position, float radius, unsigned int material_id) : position(position), radius(radius), material_id(material_id) {}
    };
    auto _spheres = std::vector<GPUSphere>();
    for (auto& sphere : spheres) {
        uint32_t material_id = static_cast<uint32_t>(materials.size()); // index of the material about to be added
        materials.push_back(sphere->material);
        _spheres.emplace_back(sphere->pos, sphere->radius, material_id);
    }

    glCreateBuffers(1, &spheres_ssbo);
    glNamedBufferData(spheres_ssbo, _spheres.size() * sizeof(GPUSphere), _spheres.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, spheres_ssbo);

    // Setting the materials SSBO
    glCreateBuffers(1, &material_ssbo);
    glNamedBufferData(material_ssbo, materials.size() * sizeof(Material), materials.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, material_ssbo);
};