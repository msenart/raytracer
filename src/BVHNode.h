#pragma once

#include "OpenGLIncludes.h"

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    static float distance(const AABB& aabb1,const AABB& aabb2);

    static AABB merge(const AABB& aabb1,const AABB& aabb2);

    glm::vec3 center() const;

    [[nodiscard]] float surfaceArea() const;
};

struct BVHNode {
    AABB aabb;
    unsigned int id_node_next_left;
    unsigned int id_node_next_right;
    unsigned int mesh_id;
    unsigned int mesh_type;
    BVHNode() = default;

    BVHNode(const AABB & aabb, unsigned int id_node_next_left, unsigned int id_node_next_right, unsigned int mesh_id, unsigned int mesh_type) :
        aabb(aabb),
        id_node_next_left(id_node_next_left),
        id_node_next_right(id_node_next_right),
        mesh_id(mesh_id),
        mesh_type(mesh_type) {}
};


struct BVH {
    std::vector<BVHNode> nodes;

    static BVH merge(const BVH &bvh1, const BVH &bvh2);
    [[nodiscard]] BVHNode root() const {return nodes[nodes.size()-1];};

    /// @brief creates an instance of BVH.
    /// Be careful! the nodes should always be forming a tree, meaning that all nodes are bound correctly and that the last node of the list is the root of the tree.
    explicit BVH(const std::vector<BVHNode> &nodes) : nodes(nodes) {}
};