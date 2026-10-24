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

glm::vec3 AABB::center() const {
    return (min + max)*0.5f;
}

float AABB::surfaceArea() const {
    const glm::vec3 d = max - min;
    return 2.f * (d.x * d.y + d.y * d.z + d.z * d.x);
}

BVH BVH::merge(const BVH& bvh1, const BVH& bvh2) {
    if (bvh1.nodes.empty()) return bvh2;
    if (bvh2.nodes.empty()) return bvh1;

    const auto offset = static_cast<unsigned int>(bvh1.nodes.size());

    auto nodes = bvh1.nodes;
    nodes.reserve(bvh1.nodes.size() + bvh2.nodes.size() + 1);

    for (auto node : bvh2.nodes) {
        if (node.id_node_next_left  != UINT_MAX) node.id_node_next_left  += offset;
        if (node.id_node_next_right != UINT_MAX) node.id_node_next_right += offset;
        nodes.push_back(node);
    }

    const auto root1 = offset - 1;
    const auto root2 = static_cast<unsigned int>(nodes.size()) - 1;

    nodes.emplace_back(
        AABB::merge(bvh1.nodes.back().aabb, bvh2.nodes.back().aabb),
        root1, root2, UINT_MAX, UINT_MAX);

    return BVH(nodes);
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices): vertices(std::move(vertices)), indices(std::move(indices)){}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Material material): vertices(std::move(vertices)), indices(std::move(indices)), material(material) {}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, Material material, Transform transform): vertices(std::move(vertices)), indices(std::move(indices)), material(material), transform(transform) {}

std::vector<AABB> Mesh::aabbs() const {
    auto out = std::vector<AABB>(indices.size()/3);
    for (unsigned int i = 0; i < indices.size()/3; i += 1) {
        auto v1 = transform.matrix() * glm::vec4(vertices[indices[3*i]].position, 1.0);
        auto v2 = transform.matrix() * glm::vec4(vertices[indices[3*i+1]].position, 1.0);
        auto v3 = transform.matrix() * glm::vec4(vertices[indices[3*i+2]].position, 1.0);
        auto min = glm::vec3(
                    std::min({v1.x, v2.x, v3.x}),
                    std::min({v1.y, v2.y, v3.y}),
                    std::min({v1.z, v2.z, v3.z}));
        auto max = glm::vec3(
                    std::max({v1.x, v2.x, v3.x}),
                    std::max({v1.y, v2.y, v3.y}),
                    std::max({v1.z, v2.z, v3.z}));
        out[i] = {min, max};
    }
    return out;
}

AABB Mesh::aabb() const {
    auto mn = glm::vec3(std::numeric_limits<float>::max());
    auto mx = glm::vec3(std::numeric_limits<float>::lowest());
    for (const auto& v : vertices) {
        glm::vec3 pos = transform.matrix() * glm::vec4(v.position, 1.0);
        mn = glm::vec3(
            std::min(pos.x, mn.x),
            std::min(pos.y, mn.y),
            std::min(pos.z, mn.z));
        mx = glm::vec3(
            std::max(pos.x, mx.x),
            std::max(pos.y, mx.y),
            std::max(pos.z, mx.z));
    }
    return {mn, mx};
}

unsigned int expandBits(unsigned int v)
{
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}

unsigned int morton3D(float x, float y, float z)
{
    x = std::min(std::max(x * 1024.0f, 0.0f), 1023.0f);
    y = std::min(std::max(y * 1024.0f, 0.0f), 1023.0f);
    z = std::min(std::max(z * 1024.0f, 0.0f), 1023.0f);
    unsigned int xx = expandBits((unsigned int)x);
    unsigned int yy = expandBits((unsigned int)y);
    unsigned int zz = expandBits((unsigned int)z);
    return xx * 4 + yy * 2 + zz;
}

unsigned int findSplit(unsigned int first, unsigned int last,
                       const std::vector<std::pair<unsigned int, BVHNode>>& leaves) {
    unsigned int a = leaves[first].first, b = leaves[last].first;
    if (a == b) return (first + last) / 2;

    int bit = 31 - __builtin_clz(a ^ b);

    // log search
    unsigned int lo = first, hi = last;
    while (hi - lo > 1) {
        unsigned int mid = (lo + hi) / 2;
        if ((leaves[mid].first >> bit) & 1) hi = mid; // if the most significant bit of the middle is equal to 1
        else                     lo = mid;
    }
    return lo; // returns the highest index which is = 0
}

BVH Mesh::buildBVH(const std::vector<std::pair<unsigned int, BVHNode>>& leaves,
                   unsigned int first, unsigned int last) {
    if (first == last) {
        return BVH({leaves[first].second});
    }

    unsigned int split = findSplit(first, last, leaves);

    return BVH::merge(buildBVH(leaves, first, split),
                      buildBVH(leaves, split + 1, last));
}

BVH Mesh::bvh(unsigned int offset) const {
    auto all_aabbs = aabbs(); // ordered by indices
    if (all_aabbs.empty()) return BVH{{}};

    auto biggest_aabb = aabb();
    auto diag = glm::max(biggest_aabb.max - biggest_aabb.min, glm::vec3(1e-8f));
    std::vector<std::pair<unsigned int, BVHNode>> all_leaves;
    all_leaves.reserve(all_aabbs.size());

    for (unsigned int i = 0; i < all_aabbs.size(); i++) {
        glm::vec3 center = (all_aabbs[i].center() - biggest_aabb.min) / diag;
        unsigned int morton = morton3D(center.x, center.y, center.z);
        all_leaves.emplace_back(morton, BVHNode{all_aabbs[i], UINT_MAX, UINT_MAX, offset + 3*i, 1});
    }

    // sort all leaves by morton code
    std::sort(all_leaves.begin(), all_leaves.end(),
          [](auto& a, auto& b) { return a.first < b.first; });

    auto bvh = buildBVH(all_leaves,0, all_leaves.size()-1);
    return bvh;
}



Sphere::Sphere(glm::vec3 pos, float radius, Material material): pos(pos), radius(radius), material(material) {}

Sphere::Sphere(glm::vec3 pos, float radius): pos(pos), radius(radius), material(Material()) {}

BVH Sphere::bvh(unsigned int offset) const {
    return BVH({BVHNode(aabb(), UINT_MAX, UINT_MAX, offset, 2)});
}

AABB Sphere::aabb() const {
    const glm::vec3 r(radius);
    return { -r,  r};
}
