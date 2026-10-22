//
// Created by mathi on 27/09/2026.
//

#include "ObjectManager.h"
#include "Mesh.h"

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
}

void ObjectManager::buildSSBOs() {
    // taking care of regular meshes
    struct IndexAndMeshID {
        unsigned int index;
        unsigned int material_id;
        unsigned int transform_index;
        float _pad0 = 0;

        IndexAndMeshID(unsigned int index, unsigned int material_id, unsigned int transform_index) : index(index), material_id(material_id), transform_index(transform_index) {}
    };

    primitives_number = 0;

    struct GPUVertex {
        glm::vec3 position;
        float _pad0 = 0;
        glm::vec3 normal;
        float _pad1 = 0;
        glm::vec4 tangent;
        glm::vec2 uv;
        float _pad2 = 0;
        float _pad3 = 0;

        GPUVertex(Vertex vertex) : position(vertex.position), normal(vertex.normal), tangent(vertex.tangent), uv(vertex.uv) {}
    };

    // concatenating vertices and indices into one big array.
    auto _vertices = std::vector<GPUVertex>();
    auto _indices = std::vector<IndexAndMeshID>();
    auto _materials = std::vector<Material>();
    auto _transforms = std::vector<glm::mat4>();

    unsigned int offset = 0;
    for (unsigned int i = 0; i < meshes.size(); i++) {
        for (const auto& vertex: meshes[i]->vertices) {
            _vertices.emplace_back(vertex);
        }
        for (const auto& index: meshes[i]->indices) {
            _indices.emplace_back(offset + index, i, _transforms.size());
        }
        _transforms.emplace_back(meshes[i]->transform.matrix());
        _materials.push_back(meshes[i]->material);
        offset+=meshes[i]->vertices.size();
    }
    primitives_number+=_indices.size()/3;

    // modern way to use SSBO (just like VBO, VAO, etc...)
    glCreateBuffers(1, &vertices_ssbo);
    glNamedBufferData(vertices_ssbo, _vertices.size() * sizeof(GPUVertex), _vertices.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, vertices_ssbo);

    glCreateBuffers(1, &indices_ssbo);
    glNamedBufferData(indices_ssbo, _indices.size() * sizeof(IndexAndMeshID), _indices.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, indices_ssbo);

    // taking care of sphere equations
    struct GPUSphere {
        unsigned int transform_index;
        float radius;
        unsigned int material_id;
        float _pad0 = 0;

        GPUSphere(unsigned int transform_index, float radius, unsigned int material_id) : transform_index(transform_index), radius(radius), material_id(material_id) {}
    };
    auto _spheres = std::vector<GPUSphere>();
    for (unsigned int i = 0; i < spheres.size(); i++) {
        uint32_t material_id = static_cast<uint32_t>(_materials.size()); // index of the material about to be added
        _materials.push_back(spheres[i]->material);
        _spheres.emplace_back(_transforms.size(), spheres[i]->radius, material_id);
        _transforms.emplace_back(glm::translate(glm::mat4(1), spheres[i]->pos));
    }
    primitives_number+=_spheres.size();

    glCreateBuffers(1, &spheres_ssbo);
    glNamedBufferData(spheres_ssbo, _spheres.size() * sizeof(GPUSphere), _spheres.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, spheres_ssbo);

    // Setting the materials SSBO
    glCreateBuffers(1, &material_ssbo);
    glNamedBufferData(material_ssbo, _materials.size() * sizeof(Material), _materials.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, material_ssbo);

    // Setting the transform SSBO
    glCreateBuffers(1, &transform_ssbo);
    glNamedBufferData(transform_ssbo, _transforms.size() * sizeof(glm::mat4), _transforms.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, transform_ssbo);
}

void ObjectManager::buildBVH() {
    struct Node {
        AABB aabb;
        unsigned int id_node_next_left;
        unsigned int id_node_next_right;
        unsigned int mesh_id;
        unsigned int mesh_type;
        Node() = default;

        Node(const AABB & aabb, unsigned int id_node_next_left, unsigned int id_node_next_right, unsigned int mesh_id, unsigned int mesh_type) :
            aabb(aabb),
            id_node_next_left(id_node_next_left),
            id_node_next_right(id_node_next_right),
            mesh_id(mesh_id),
            mesh_type(mesh_type) {}
    };

    unsigned int number_total_indices = 0;
    for (const auto& mesh : meshes) {
        number_total_indices += mesh->indices.size();
    }

    std::vector<Node> all_leaves;
    all_leaves.reserve(number_total_indices/3 + spheres.size());

    const glm::vec3 pad(1e-5f);
    unsigned int index_offset = 0;
    for (const auto& mesh : meshes) {
        const glm::mat4 M = mesh->transform.matrix();
        for (unsigned int t = 0; t < mesh->indices.size()/3; t++) {
            glm::vec3 mn(std::numeric_limits<float>::max());
            glm::vec3 mx(std::numeric_limits<float>::lowest());
            for (int k = 0; k < 3; k++) {
                const glm::vec3 p = glm::vec3(M * glm::vec4(mesh->vertices[mesh->indices[3*t+k]].position, 1.f));
                mn = glm::min(mn, p);
                mx = glm::max(mx, p);
            }
            all_leaves.emplace_back(AABB{mn - pad, mx + pad}, UINT_MAX, UINT_MAX, index_offset + 3*t, 1);
        }
        index_offset += mesh->indices.size();
    }

    for (unsigned int i = 0; i < spheres.size(); i++) {
        const AABB local = spheres[i]->aabb();
        all_leaves.emplace_back(AABB{spheres[i]->pos + local.min, spheres[i]->pos + local.max},
                                UINT_MAX, UINT_MAX, i, 2);
    }

    if (all_leaves.empty()) return;

    std::vector<Node> bvh_tree = all_leaves;
    std::vector<unsigned int> active(bvh_tree.size());
    std::iota(active.begin(), active.end(), 0u);

    while (active.size() > 1) {
        std::vector<bool> used(bvh_tree.size(), false);
        std::vector<unsigned int> next_active;

        for (unsigned int i : active) {
            if (used[i]) continue;
            float best = std::numeric_limits<float>::infinity();
            unsigned int best_j = UINT_MAX;
            for (unsigned int j : active) {
                if (i == j || used[j]) continue;
                float d = AABB::merge(bvh_tree[i].aabb, bvh_tree[j].aabb).surfaceArea();
                if (d < best) { best = d; best_j = j; }
            }
            if (best_j == UINT_MAX) continue;
            used[i] = true;
            used[best_j] = true;
            unsigned int idx = bvh_tree.size();
            bvh_tree.emplace_back(AABB::merge(bvh_tree[i].aabb, bvh_tree[best_j].aabb),
                                  i, best_j, UINT_MAX, 0);
            next_active.push_back(idx);
        }
        for (unsigned int i : active)
            if (!used[i]) next_active.push_back(i);

        active = std::move(next_active);
    }

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

        explicit GPUNode(const Node &n):
            min(n.aabb.min),
            max(n.aabb.max),
            id_node_next_left(n.id_node_next_left),
            id_node_next_right(n.id_node_next_right),
            mesh_id(n.mesh_id),
            mesh_type(n.mesh_type) {};
    };

    std::vector<GPUNode> gpu_bvh_tree(bvh_tree.size());
    for (unsigned int i = 0; i < bvh_tree.size(); i++) {
        gpu_bvh_tree[i] = GPUNode(bvh_tree[i]);
    }

    glCreateBuffers(1, &bvh_ssbo);
    glNamedBufferData(bvh_ssbo, gpu_bvh_tree.size() * sizeof(GPUNode), gpu_bvh_tree.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, bvh_ssbo);
}

unsigned int ObjectManager::primitivesNumber() const {
    return primitives_number;
}
