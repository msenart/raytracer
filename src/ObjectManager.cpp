//
// Created by mathi on 27/09/2026.
//

#include "ObjectManager.h"
#include "Mesh.h"
#include "Camera.h"
#include "Shader.h"
#include "MeshHelpers.h"
#include "BVHNode.h"

ObjectManager::ObjectManager(std::vector<std::shared_ptr<Mesh>> meshes) : meshes(std::move(meshes)) {
}

ObjectManager::ObjectManager()= default;

void ObjectManager::addMesh(const std::shared_ptr<Mesh> &mesh) {
    meshes.push_back(mesh);
}

void ObjectManager::addSphere(std::shared_ptr<Sphere> sphere) {
    spheres.push_back(sphere);
}

void ObjectManager::removeMesh(const unsigned int index) {
    meshes.erase(meshes.begin() + index);
    if (index < mesh_raster.size()) {
        destroyRasterBuffers(mesh_raster[index]);
        mesh_raster.erase(mesh_raster.begin() + index);
    }
    if (selection.kind == Selection::Kind::Mesh) {
        if (selection.index == index) selection = {};
        else if (selection.index > index) selection.index--;
    }
}

void ObjectManager::removeSphere(const unsigned int index) {
    spheres.erase(spheres.begin() + index);
    if (selection.kind == Selection::Kind::Sphere) {
        if (selection.index == index) selection = {};
        else if (selection.index > index) selection.index--;
    }
}

namespace {
    /// (re)uploads data into an SSBO, creating the buffer only the first time so rebuilding doesn't leak.
    void uploadSSBO(unsigned int& buffer, unsigned int binding, size_t size, const void* data) {
        if (buffer == 0) glCreateBuffers(1, &buffer);
        glNamedBufferData(buffer, size, data, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, buffer);
    }
}

void ObjectManager::buildSSBOs() {
    // taking care of regular meshes
    struct IndexAndMeshID {
        unsigned int index;
        unsigned int material_id;

        IndexAndMeshID(unsigned int index, unsigned int material_id) : index(index), material_id(material_id) {}
    };

    // vertices are sent in world space, so the shader doesn't have to transform them for every intersection test.
    struct GPUVertex {
        glm::vec3 position;
        float _pad0 = 0;
        glm::vec3 normal;
        float _pad1 = 0;
        glm::vec4 tangent;
        glm::vec2 uv;
        float _pad2 = 0;
        float _pad3 = 0;

        GPUVertex(const Vertex& vertex, const glm::mat4& model, const glm::mat3& normalMatrix) :
            position(glm::vec3(model * glm::vec4(vertex.position, 1.f))),
            normal(glm::normalize(normalMatrix * vertex.normal)),
            tangent(glm::vec4(glm::mat3(model) * glm::vec3(vertex.tangent), vertex.tangent.w)),
            uv(vertex.uv) {}
    };

    // concatenating vertices and indices into one big array.
    auto _vertices = std::vector<GPUVertex>();
    auto _indices = std::vector<IndexAndMeshID>();
    auto _materials = collectMaterials();

    unsigned int offset = 0;
    for (unsigned int i = 0; i < meshes.size(); i++) {
        const glm::mat4 model = meshes[i]->transform.matrix();
        const glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        for (const auto& vertex: meshes[i]->vertices) {
            _vertices.emplace_back(vertex, model, normalMatrix);
        }
        // a mirroring transform reverses the orientation of the triangles: swapping two indices restores it.
        const bool mirrored = glm::determinant(glm::mat3(model)) < 0.f;
        for (size_t t = 0; t + 2 < meshes[i]->indices.size(); t += 3) {
            _indices.emplace_back(offset + meshes[i]->indices[t], i);
            _indices.emplace_back(offset + meshes[i]->indices[t + (mirrored ? 2 : 1)], i);
            _indices.emplace_back(offset + meshes[i]->indices[t + (mirrored ? 1 : 2)], i);
        }
        offset+=meshes[i]->vertices.size();
    }

    // modern way to use SSBO (just like VBO, VAO, etc...)
    uploadSSBO(vertices_ssbo, 0, _vertices.size() * sizeof(GPUVertex), _vertices.data());
    uploadSSBO(indices_ssbo, 1, _indices.size() * sizeof(IndexAndMeshID), _indices.data());

    // taking care of sphere equations
    struct GPUSphere {
        glm::vec3 center;
        float radius;
        unsigned int material_id;
        float _pad0 = 0;
        float _pad1 = 0;
        float _pad2 = 0;

        GPUSphere(const glm::vec3& center, float radius, unsigned int material_id) : center(center), radius(radius), material_id(material_id) {}
    };
    auto _spheres = std::vector<GPUSphere>();
    for (unsigned int i = 0; i < spheres.size(); i++) {
        // the materials of the spheres come right after the ones of the meshes (see collectMaterials)
        _spheres.emplace_back(spheres[i]->pos, spheres[i]->radius, static_cast<unsigned int>(meshes.size() + i));
    }

    uploadSSBO(spheres_ssbo, 2, _spheres.size() * sizeof(GPUSphere), _spheres.data());
    uploadSSBO(material_ssbo, 3, _materials.size() * sizeof(Material), _materials.data());
}

void ObjectManager::buildBVH() {
    // unsigned int number_total_indices = 0;
    // for (const auto& mesh : meshes) {
    //     number_total_indices += mesh->indices.size();
    // }
    //
    // std::vector<Node> all_leaves;
    // all_leaves.reserve(number_total_indices/3 + spheres.size());
    //
    // const glm::vec3 pad(1e-5f);
    // unsigned int index_offset = 0;
    // for (const auto& mesh : meshes) {
    //     const glm::mat4 M = mesh->transform.matrix();
    //     for (unsigned int t = 0; t < mesh->indices.size()/3; t++) {
    //         glm::vec3 mn(std::numeric_limits<float>::max());
    //         glm::vec3 mx(std::numeric_limits<float>::lowest());
    //         for (int k = 0; k < 3; k++) {
    //             const glm::vec3 p = glm::vec3(M * glm::vec4(mesh->vertices[mesh->indices[3*t+k]].position, 1.f));
    //             mn = glm::min(mn, p);
    //             mx = glm::max(mx, p);
    //         }
    //         all_leaves.emplace_back(AABB{mn - pad, mx + pad}, UINT_MAX, UINT_MAX, index_offset + 3*t, 1);
    //     }
    //     index_offset += mesh->indices.size();
    // }
    //
    // for (unsigned int i = 0; i < spheres.size(); i++) {
    //     const AABB local = spheres[i]->aabb();
    //     all_leaves.emplace_back(AABB{spheres[i]->pos + local.min, spheres[i]->pos + local.max},
    //                             UINT_MAX, UINT_MAX, i, 2);
    // }
    //
    // if (all_leaves.empty()) return;
    //
    // std::vector<Node> bvh_tree = all_leaves;
    // std::vector<unsigned int> active(bvh_tree.size());
    // std::iota(active.begin(), active.end(), 0u);
    //
    // while (active.size() > 1) {
    //     std::vector<bool> used(bvh_tree.size(), false);
    //     std::vector<unsigned int> next_active;
    //
    //     for (unsigned int i : active) {
    //         if (used[i]) continue;
    //         float best = std::numeric_limits<float>::infinity();
    //         unsigned int best_j = UINT_MAX;
    //         for (unsigned int j : active) {
    //             if (i == j || used[j]) continue;
    //             float d = AABB::merge(bvh_tree[i].aabb, bvh_tree[j].aabb).surfaceArea();
    //             if (d < best) { best = d; best_j = j; }
    //         }
    //         if (best_j == UINT_MAX) continue;
    //         used[i] = true;
    //         used[best_j] = true;
    //         unsigned int idx = bvh_tree.size();
    //         bvh_tree.emplace_back(AABB::merge(bvh_tree[i].aabb, bvh_tree[best_j].aabb),
    //                               i, best_j, UINT_MAX, 0);
    //         next_active.push_back(idx);
    //     }
    //     for (unsigned int i : active)
    //         if (!used[i]) next_active.push_back(i);
    //
    //     active = std::move(next_active);
    // }

    std::vector<BVH> bvhs;
    unsigned int offset = 0;
    for (const auto& mesh : meshes) {
        bvhs.push_back(mesh->bvh(offset));
        offset+=mesh->indices.size();
    }
    offset = 0;
    for (const auto& sphere : spheres) {
        bvhs.push_back(sphere->bvh(offset));
        offset+=1;
    }

    while (bvhs.size() > 1) {
        std::vector<BVH> next_bvhs;
        next_bvhs.reserve(bvhs.size() / 2 + 1);
        std::vector<char> used(bvhs.size(), 0);
        for (size_t i = 0; i < bvhs.size(); i++) {
            if (used[i]) continue;
            float best = std::numeric_limits<float>::infinity();
            unsigned int best_j = UINT_MAX;
            for (size_t j = i + 1; j < bvhs.size(); j++) {
                if (used[j]) continue;
                float d = AABB::merge(bvhs[i].root().aabb, bvhs[j].root().aabb).surfaceArea();
                if (d < best) { best = d; best_j = j; }
            }
            if (best_j == UINT_MAX) {
                next_bvhs.push_back(std::move(bvhs[i]));
                continue;
            }
            used[i] = used[best_j] = 1;
            next_bvhs.push_back(BVH::merge(bvhs[i], bvhs[best_j]));
        }
        bvhs = std::move(next_bvhs);
    }
    BVH out_bvh = std::move(bvhs[0]);

    struct GPUNode {
        glm::vec3 min{0};
        unsigned int id_node_next_left{};

        glm::vec3 max{0};
        unsigned int id_node_next_right{};

        unsigned int mesh_id{};
        unsigned int mesh_type{};
        float _pad0{};
        float _pad1{};

        GPUNode() = default;

        explicit GPUNode(const BVHNode &n):
            min(n.aabb.min),
            max(n.aabb.max),
            id_node_next_left(n.id_node_next_left),
            id_node_next_right(n.id_node_next_right),
            mesh_id(n.mesh_id),
            mesh_type(n.mesh_type) {};
    };

    std::vector<GPUNode> gpu_bvh_tree(out_bvh.nodes.size());
    for (unsigned int i = 0; i < out_bvh.nodes.size(); i++) {
        gpu_bvh_tree[i] = GPUNode(out_bvh.nodes[i]);
    }

    uploadSSBO(bvh_ssbo, 4, gpu_bvh_tree.size() * sizeof(GPUNode), gpu_bvh_tree.data());
}

void ObjectManager::rebuildAccelerationStructure() {
    buildBVH(); // it is weird that this function must be before buildSSBOs...
    buildSSBOs();
}

std::vector<Material> ObjectManager::collectMaterials() const {
    // meshes first, then spheres, and the air at the very end (the shader uses the last material as the air).
    std::vector<Material> materials;
    materials.reserve(meshes.size() + spheres.size() + 1);
    for (const auto& mesh : meshes) materials.push_back(mesh->material);
    for (const auto& sphere : spheres) materials.push_back(sphere->material);
    Material air; // albedo 1, n = 1, k = 0
    air.id = MaterialType::Glass;
    materials.push_back(air);
    return materials;
}

void ObjectManager::updateMaterials() {
    if (material_ssbo == 0) return; // the acceleration structure was never built, nothing to update
    const auto materials = collectMaterials();
    glNamedBufferSubData(material_ssbo, 0, materials.size() * sizeof(Material), materials.data());
}

unsigned int ObjectManager::primitivesNumber() const {
    unsigned int total = spheres.size();
    for (const auto& mesh : meshes) total += mesh->indices.size() / 3;
    return total;
}

Mesh* ObjectManager::selectedMesh() const {
    if (selection.kind != Selection::Kind::Mesh || selection.index >= meshes.size()) return nullptr;
    return meshes[selection.index].get();
}

Sphere* ObjectManager::selectedSphere() const {
    if (selection.kind != Selection::Kind::Sphere || selection.index >= spheres.size()) return nullptr;
    return spheres[selection.index].get();
}

bool ObjectManager::pick(const glm::vec3& origin, const glm::vec3& direction) {
    float best = std::numeric_limits<float>::max();
    Selection hit;

    for (unsigned int i = 0; i < meshes.size(); i++) {
        const auto& mesh = meshes[i];
        // working in the local space of the mesh. The direction isn't renormalized, so t is the same in both spaces.
        const glm::mat4 inverse = glm::inverse(mesh->transform.matrix());
        const glm::vec3 o = glm::vec3(inverse * glm::vec4(origin, 1.f));
        const glm::vec3 d = glm::vec3(inverse * glm::vec4(direction, 0.f));
        for (unsigned int t = 0; t + 2 < mesh->indices.size(); t += 3) {
            // Moller-Trumbore
            const glm::vec3& a = mesh->vertices[mesh->indices[t]].position;
            const glm::vec3 e1 = mesh->vertices[mesh->indices[t + 1]].position - a;
            const glm::vec3 e2 = mesh->vertices[mesh->indices[t + 2]].position - a;
            const glm::vec3 p = glm::cross(d, e2);
            const float det = glm::dot(e1, p);
            if (glm::abs(det) < 1e-12f) continue;
            const glm::vec3 s = o - a;
            const float u = glm::dot(s, p) / det;
            if (u < 0.f || u > 1.f) continue;
            const glm::vec3 q = glm::cross(s, e1);
            const float v = glm::dot(d, q) / det;
            if (v < 0.f || u + v > 1.f) continue;
            const float dist = glm::dot(e2, q) / det;
            if (dist > 0.f && dist < best) {
                best = dist;
                hit = {Selection::Kind::Mesh, i};
            }
        }
    }

    for (unsigned int i = 0; i < spheres.size(); i++) {
        const glm::vec3 oc = origin - spheres[i]->pos;
        const float b = glm::dot(oc, direction);
        const float c = glm::dot(oc, oc) - spheres[i]->radius * spheres[i]->radius;
        const float discriminant = b * b - c;
        if (discriminant < 0.f) continue;
        const float root = glm::sqrt(discriminant);
        float dist = -b - root;
        if (dist < 0.f) dist = -b + root;
        if (dist > 0.f && dist < best) {
            best = dist;
            hit = {Selection::Kind::Sphere, i};
        }
    }

    selection = hit;
    return hit.kind != Selection::Kind::None;
}

ObjectManager::RasterBuffers ObjectManager::createRasterBuffers(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    RasterBuffers b;
    b.count = static_cast<int>(indices.size());
    glCreateVertexArrays(1, &b.vao);
    glCreateBuffers(1, &b.vbo);
    glCreateBuffers(1, &b.ebo);
    glNamedBufferData(b.vbo, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glNamedBufferData(b.ebo, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glVertexArrayVertexBuffer(b.vao, 0, b.vbo, 0, sizeof(Vertex));
    glVertexArrayElementBuffer(b.vao, b.ebo);

    // locations match raster.vert: position, normal, tangent, uv
    struct Attribute { GLint count; size_t offset; };
    const Attribute attributes[] = {
        {3, offsetof(Vertex, position)},
        {3, offsetof(Vertex, normal)},
        {4, offsetof(Vertex, tangent)},
        {2, offsetof(Vertex, uv)},
    };
    for (unsigned int i = 0; i < 4; i++) {
        glEnableVertexArrayAttrib(b.vao, i);
        glVertexArrayAttribFormat(b.vao, i, attributes[i].count, GL_FLOAT, GL_FALSE, static_cast<GLuint>(attributes[i].offset));
        glVertexArrayAttribBinding(b.vao, i, 0);
    }
    return b;
}

void ObjectManager::destroyRasterBuffers(RasterBuffers& buffers) {
    glDeleteVertexArrays(1, &buffers.vao);
    glDeleteBuffers(1, &buffers.vbo);
    glDeleteBuffers(1, &buffers.ebo);
    buffers = {};
}

void ObjectManager::ensureRasterBuffers() {
    for (size_t i = mesh_raster.size(); i < meshes.size(); i++) {
        mesh_raster.push_back(createRasterBuffers(meshes[i]->vertices, meshes[i]->indices));
    }
    if (sphere_raster.vao == 0) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        MeshHelpers::generateSphere(vertices, indices, glm::vec2(1.f), 32);
        sphere_raster = createRasterBuffers(vertices, indices);
    }
}

namespace {
    /// Color used to represent a material in the rasterized view.
    glm::vec3 displayColor(const Material& material) {
        float albedo = 0.f;
        for (const float a : material.albedo) albedo += a;
        albedo /= static_cast<float>(material.albedo.size());
        albedo = glm::clamp(albedo, 0.f, 1.f);
        switch (material.id) {
            case MaterialType::Light:      return glm::vec3(1.0f, 0.95f, 0.75f);
            case MaterialType::Metallic:   return glm::vec3(0.75f, 0.75f, 0.8f) * albedo;
            case MaterialType::Glass:      return glm::vec3(0.6f, 0.8f, 1.0f) * albedo;
            default:                       return glm::vec3(0.8f) * albedo;
        }
    }
}

void ObjectManager::drawRaster(const Shader& shader, const Camera& camera) {
    ensureRasterBuffers();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    shader.bind();
    shader.set("view", camera.view());
    shader.set("projection", camera.projection());
    shader.set("cameraPos", camera.position);

    const auto draw = [&](const RasterBuffers& buffers, const glm::mat4& model, const Material& material, bool selected) {
        shader.set("model", model);
        shader.set("color", displayColor(material));
        shader.set("emissive", material.id == MaterialType::Light);
        shader.set("selected", selected);
        glBindVertexArray(buffers.vao);
        glDrawElements(GL_TRIANGLES, buffers.count, GL_UNSIGNED_INT, nullptr);
    };

    for (unsigned int i = 0; i < meshes.size(); i++) {
        const bool selected = selection.kind == Selection::Kind::Mesh && selection.index == i;
        draw(mesh_raster[i], meshes[i]->transform.matrix(), meshes[i]->material, selected);
    }
    for (unsigned int i = 0; i < spheres.size(); i++) {
        const bool selected = selection.kind == Selection::Kind::Sphere && selection.index == i;
        const glm::mat4 model = glm::translate(glm::mat4(1.f), spheres[i]->pos) * glm::scale(glm::mat4(1.f), glm::vec3(spheres[i]->radius));
        draw(sphere_raster, model, spheres[i]->material, selected);
    }

    glBindVertexArray(0);
    shader.unbind();
    glDisable(GL_DEPTH_TEST);
}
