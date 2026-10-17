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

HitProperties isRayIntersectingAABB(Ray r, vec3 min, vec3 max){
    return HitProperties(false, UINT_MAX, UINT_MAX, 0, vec2(0), vec3(0));
}

HitProperties bruteforce_traversal(Ray r){
    float min_distance = FLOAT_MAX;
    HitProperties argmin_hp = HitProperties(false, UINT_MAX, UINT_MAX, 0, vec2(0), vec3(0));
    for (uint i = 0; i < indices.length(); i += 3) { // handling triangles (mesh_type = 1)
        HitProperties hp = isRayIntersectingTriangle(r, indices[i].index, indices[i+1].index, indices[i+2].index, false, 1, indices[i].mesh_id); // materials[argmin_mesh].normalInterpolation
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

HitProperties bvh_traversal(Ray r){
    uint cur_node_id = 0;
    uint nodes_id_stack[64];
    uint nodes_id_stack_size = 0;
    nodes_id_stack[0] = nodes.length()-1;

    Node cur_node = nodes[nodes_id_stack[cur_node_id]];
    HitProperties hp = HitProperties(false, UINT_MAX, UINT_MAX, 0, vec2(0), vec3(0));
    // if the root is not hit, do not traverse the tree.
    if (!isRayIntersectingAABB(r, cur_node.min, cur_node.max).hasHit){
        return hp;
    }
    bool isIntersecting = true;
    while (isIntersecting){
        // if the node is a leaf then...
        switch (cur_node.mesh_type){
            case 0:
                break;
            case 1:
                hp = isRayIntersectingTriangle(r, indices[cur_node.mesh_id].index, indices[cur_node.mesh_id + 1].index, indices[cur_node.mesh_id + 2].index, false, 1, cur_node.mesh_id);
                if (hp.hasHit){
                    return hp;
                }
                cur_node_id-=1; // take care of unstacking...
                break;
            case 2:
                hp = isRayIntersectingSphere(r, spheres[cur_node.mesh_id], 2, cur_node.mesh_id);
                if (hp.hasHit){
                    return hp;
                }
                cur_node_id-=1; // take care of unstacking...
                break;
        }

        // sort ids by distance to the ray, conditionally by if it has hit something
        uint ids[2] = { cur_node.id_node_next_left, cur_node.id_node_next_right };
        uint sorted_ids[2];
        uint sorted_ids_size = 0;
        for (uint i = 0; i < 2; i++){
            float min_distance = FLOAT_MAX;
            uint argmin_distance = UINT_MAX;
            for (uint j = 0; j < 2; j++){
                HitProperties hp = isRayIntersectingAABB(r, nodes[ids[i]].min, nodes[ids[i]].max);
                bool in_sorted_ids = false;
                for (uint k = 0; k < sorted_ids_size; k++){
                    if (ids[j] == sorted_ids[k]){
                        in_sorted_ids = true;
                        break;
                    }
                }
                if (hp.hasHit && !in_sorted_ids && hp.t < min_distance){
                    argmin_distance = ids[i];
                    min_distance = hp.t;
                }
            }
            sorted_ids[i] = ids[argmin_distance];
            sorted_ids_size+=1;
        }
        nodes_id_stack_size+=sorted_ids_size;
        // ...and append the result in the stack directly.
        for (uint i = 0; i < sorted_ids_size; i++){
            nodes_id_stack[nodes_id_stack_size-1-i] = sorted_ids[i];
        }
        // Increment cur_node_id
        cur_node_id += sorted_ids_size;
    }
    return HitProperties(false, 0, 0, 0, vec2(0), vec3(0));
}
