#include "hitproperties.glsl"
#include "ray.glsl"
#include "params.glsl"
#include "triangleEQ.glsl"
#include "sphereEQ.glsl"

struct Node {
    vec3 min;
    uint id_node_next_left;
    vec3 max;
    uint id_node_next_right;
    uint mesh_id;
    uint mesh_type;
    float _pad0;
    float _pad1;
};

layout(std430, binding = 4) readonly buffer NodesBuffer {
    Node nodes[];
};

HitProperties isRayIntersectingAABB(Ray r, vec3 bmin, vec3 bmax){
    vec3 invD = 1.0 / r.d;
    vec3 t0 = (bmin - r.o) * invD;
    vec3 t1 = (bmax - r.o) * invD;

    vec3 tsmaller = min(t0, t1);
    vec3 tbigger  = max(t0, t1);

    float tNear = max(max(tsmaller.x, tsmaller.y), tsmaller.z);
    float tFar  = min(min(tbigger.x,  tbigger.y),  tbigger.z);

    bool hit = tNear <= tFar && tFar > 0.0;

    HitProperties hp = HitProperties(false, UINT_MAX, UINT_MAX, 0.0, vec2(0), vec3(0));
    if (hit) {
        hp.hasHit = true;
        hp.t = max(tNear, 0.0);   // 0 if the origin is inside the box.
    }
    return hp;
}

HitProperties bruteforce_traversal(Ray r){
    float min_distance = FLOAT_MAX;
    HitProperties argmin_hp = HitProperties(false, UINT_MAX, UINT_MAX, 0, vec2(0), vec3(0));
    for (uint i = 0; i < indices.length(); i += 3) { // handling triangles (mesh_type = 1)
        HitProperties hp = isRayIntersectingTriangle(r, indices[i].index, indices[i+1].index, indices[i+2].index, indices[i].transform_index, false, 1, indices[i].material_id); // materials[argmin_mesh].normalInterpolation
        if (hp.hasHit && hp.t < min_distance) {
            min_distance = hp.t;
            argmin_hp = hp;
        }
    }
    for (uint i = 0; i < spheres.length(); i++) { // handling spheres equations (mesh_type = 2)
        HitProperties hp = isRayIntersectingSphere(r,spheres[i], 2, i);
        if (hp.hasHit && hp.t < min_distance) {
            min_distance = hp.t;
            argmin_hp = hp;
        }
    }
    return argmin_hp;
}

#define MAX_CHILDREN 2

HitProperties bvh_traversal(Ray r){
    HitProperties best = HitProperties(false, UINT_MAX, UINT_MAX, 0, vec2(0), vec3(0));
    float closest = FLOAT_MAX;

    uint stack[64];
    uint sp = 0;

    uint root = nodes.length() - 1;
    if (!isRayIntersectingAABB(r, nodes[root].min, nodes[root].max).hasHit) {
        return best;
    }
    stack[sp++] = root;

    while (sp > 0) {
        Node node = nodes[stack[--sp]];

        if (node.mesh_type == 1) {
            HitProperties hp = isRayIntersectingTriangle(r,
                    indices[node.mesh_id].index,
                    indices[node.mesh_id + 1].index,
                    indices[node.mesh_id + 2].index,
                    indices[node.mesh_id].transform_index,
                    false, 1, indices[node.mesh_id].material_id);
            if (hp.hasHit && hp.t < closest) {
                closest = hp.t; best = hp;
            }
        }
        else if (node.mesh_type == 2) {
            HitProperties hp = isRayIntersectingSphere(r, spheres[node.mesh_id], 2, node.mesh_id);
            if (hp.hasHit && hp.t < closest) {
                closest = hp.t; best = hp;
            }
        }
        else {
            // loop over children keep those who are hit.
            uint  ids[MAX_CHILDREN];
            float ts[MAX_CHILDREN];
            uint  count = 0;
            uint children[2] = {node.id_node_next_left, node.id_node_next_right};
            for (uint c = 0; c < 2; c++) {
                uint cid = children[c];
                HitProperties h = isRayIntersectingAABB(r, nodes[cid].min, nodes[cid].max);
                if (h.hasHit && h.t < closest) {
                    ids[count] = cid;
                    ts[count]  = h.t;
                    count++;
                }
            }

            // insertion sort, from the furthest to the closest
            for (uint a = 1; a < count; a++) {
                uint  ida = ids[a];
                float ta  = ts[a];
                uint b = a;
                while (b > 0 && ts[b - 1] < ta) {
                    ids[b] = ids[b - 1];
                    ts[b]  = ts[b - 1];
                    b--;
                }
                ids[b] = ida;
                ts[b]  = ta;
            }

            // stacking : the closest is at the top, the furthest at the bottom
            for (uint a = 0; a < count; a++) {
                stack[sp++] = ids[a];
            }
        }
    }
    return best;
}
