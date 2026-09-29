#version 450 core

#include "camera.glsl"
#include "sphereEQ.glsl"
#include "params.glsl"
#include "ray.glsl"
#include "reflections.glsl"
#include "triangleEQ.glsl"
#include "vertices.glsl"

uniform sampler2D readTex;

out vec4 fragColor;
in vec2 uvScreen;

uniform Camera camera;
uniform vec2 windowSize;
uniform uint frame;

//Material SSBO

layout(std430, binding = 3) readonly buffer MaterialBuffer {
    Material materials[];
};

// Core raytracing mechanic
const Material airMaterial = Material(vec4(1.0), 2u, 0.0, 1.0, 0.0);

vec4 Trace(Ray r) {
    float min_distance = FLOAT_MAX;
    uint argmin_mesh_type = UINT_MAX;
    uint argmin_mesh = UINT_MAX;
    Ray cur_r = r;
    HitProperties argmin_hp = HitProperties(false, 0, vec2(0), vec3(0));
    for (uint depth = 0; depth < MAX_ITER; depth++){
        min_distance = FLOAT_MAX;
        argmin_mesh_type = UINT_MAX;
        argmin_mesh = UINT_MAX;
        argmin_hp = HitProperties(false, 0, vec2(0), vec3(0));
        for (uint i = 0; i < indices.length(); i += 3) { // handling triangles (mesh_type = 0)
            HitProperties hp = isRayIntersectingTriangle(cur_r, indices[i].index, indices[i+1].index, indices[i+2].index, false); // materials[argmin_mesh].normalInterpolation
            if (hp.hasHit && hp.t < min_distance) {
                min_distance = hp.t;
                argmin_mesh_type = 0;
                argmin_mesh = indices[i].mesh_id;
                argmin_hp = hp;
            }
        }
        for (uint i = 0; i < spheres.length(); i++) { // handling spheres equations (mesh_type = 1)
            HitProperties hp = isRayIntersectingSphere(cur_r,spheres[i]);
            if (hp.hasHit && hp.t < min_distance) {
                min_distance = hp.t;
                argmin_mesh_type = 1;
                argmin_mesh = i;
                argmin_hp = hp;
            }
        }
        if (argmin_mesh < UINT_MAX){ // if the ray has hit a mesh
            if (argmin_mesh_type == 0 && materials[argmin_mesh].id == 0){ // the mesh is emissive
                return materials[argmin_mesh].color*cur_r.throughput;
            }
            else if (argmin_mesh_type == 1 && materials[spheres[argmin_mesh].material_id].id == 0){
                return materials[spheres[argmin_mesh].material_id].color*cur_r.throughput;
            }
            vec3 N = argmin_hp.normal;
            vec3 O = cur_r.o + cur_r.d*argmin_hp.t;
            // we assume that there is NO intersection between the spheres, otherwise it is complicated.
            // when going OUT of the sphere, it is always Material -> Air
            // when going IN the sphere, it is always Air -> Material
            Material previousMaterial;
            Material nextMaterial;
            bool isGoingOut = argmin_mesh == cur_r.in_obj_id && argmin_mesh_type == cur_r.in_objtype_id;
            if (argmin_mesh_type == 0){ // is triangle
                previousMaterial = isGoingOut ? materials[cur_r.in_obj_id] : airMaterial;
                nextMaterial = isGoingOut ? airMaterial : materials[argmin_mesh];
            }
            if (argmin_mesh_type == 1){ // is sphere
                previousMaterial = isGoingOut ? materials[spheres[cur_r.in_obj_id].material_id] : airMaterial;
                nextMaterial = isGoingOut ? airMaterial : materials[spheres[argmin_mesh].material_id];
            }

            Reflection reflection = sampleReflection(cur_r.d, N, previousMaterial, nextMaterial);

            float isReflection = -sign(dot(reflection.R,N)*dot(cur_r.d,N)); // 1 if reflection, -1 if refraction.
            uint new_in_objtype_id = isReflection == 1.f ? cur_r.in_objtype_id :
            (isGoingOut ? UINT_MAX : argmin_mesh_type);
            uint new_in_obj_id = isReflection == 1.f ? cur_r.in_obj_id :
            (isGoingOut ? UINT_MAX : argmin_mesh);
            cur_r = Ray(O + isReflection*2*eps*N,reflection.R,cur_r.throughput*reflection.coef, new_in_objtype_id, new_in_obj_id);
        }
        else { // if the ray has hit nothing
            return BACKGROUND_COLOR*cur_r.throughput;
        }
    }
    return BACKGROUND_COLOR; // or background color
}

// No blurring

Ray setWorldRay(vec3 d){
    mat3 camToWorld = mat3(inverse(camera.view));
    vec3 worldRayDir = normalize(camToWorld * d);
    return Ray(camera.position, worldRayDir, vec4(1, 1, 1, 1), UINT_MAX, UINT_MAX);
}

// Focus blurring

Ray setWorldRayFocusBlur(vec3 d){
    vec3 focusPoint = d*camera.focusDistance;

    vec3 lensOffset = vec3(camera.apertureRadius * randomUnitVectorInDisk(), 0.0);

    vec3 newOrigin = lensOffset;
    vec3 newDir = normalize(focusPoint - lensOffset);

    mat3 camToWorld = mat3(inverse(camera.view));
    vec3 worldOrigin = camera.position + camToWorld * newOrigin;
    vec3 worldDirection = normalize(camToWorld * newDir);
    return Ray(worldOrigin, worldDirection, vec4(1, 1, 1, 1), UINT_MAX, UINT_MAX);
}

// Basic raytracing
vec4 basicRayTracing(){
    vec2 uv = uvScreen;
    uv = (uv - 0.5) * 2;
    uv.x *= camera.aspectRatio;
    float z = 1.0 / tan(camera.fov * 0.5);
    vec3 rayDir = normalize(vec3(uv.x, uv.y, -z));
    rayDir = normalize(mat3(inverse(camera.view)) * rayDir);
    Ray r = Ray(camera.position, rayDir, vec4(1, 1, 1, 1), UINT_MAX, UINT_MAX);
    return Trace(r);
}

// Multisampling raytracing
vec2[MULTISAMPLING_SIZE*MULTISAMPLING_SIZE] generateMultiSamplingUV(vec2 uv){
    vec2 step = vec2(1.0/windowSize.x, 1.0/windowSize.y) / float(MULTISAMPLING_SIZE);
    vec2[MULTISAMPLING_SIZE*MULTISAMPLING_SIZE] msUVs;
    vec2 offset = vec2(-0.5f/windowSize.x,-0.5f/windowSize.y) + vec2(0.5f/(windowSize.x*MULTISAMPLING_SIZE),0.5f/(windowSize.y*MULTISAMPLING_SIZE));
    for (uint x = 0; x < MULTISAMPLING_SIZE; x++){
        for (uint y = 0; y < MULTISAMPLING_SIZE; y++){
            msUVs[MULTISAMPLING_SIZE*x + y] = uv + offset + vec2(step.x * float(x), step.y * float(y));
        }
    }
    return msUVs;
}

vec4 multiSamplingRayTracing(){
    vec2[MULTISAMPLING_SIZE*MULTISAMPLING_SIZE] uvs = generateMultiSamplingUV(uvScreen);
    vec4 sum_color = vec4(0);
    for (uint ray_id = 0; ray_id < MULTISAMPLING_SIZE*MULTISAMPLING_SIZE; ray_id++)
    {
        vec2 uv = uvs[ray_id];
        uv = (uv - 0.5) * 2;
        uv.x *= camera.aspectRatio;
        float z = 1.0 / tan(camera.fov * 0.5);
        vec3 rayDir = normalize(vec3(uv.x, uv.y, -z));
        rayDir = normalize(mat3(inverse(camera.view)) * rayDir);
        Ray r = { camera.position, rayDir, vec4(1, 1, 1, 1), UINT_MAX, UINT_MAX};
        sum_color += Trace(r);
    }

    return sum_color/(MULTISAMPLING_SIZE*MULTISAMPLING_SIZE);
}

// Randomsampling raytracing
vec4 randomRayTracing(){
    vec4 sum_color = vec4(0);
    for (uint i = 0; i < RANDOM_SAMPLING_SIZE; i++){
        vec2 uv = uvScreen + (vec2(rand(), rand()) - 0.5) / windowSize;
        uv = (uv - 0.5) * 2;
        uv.x *= camera.aspectRatio;
        float z = 1.0 / tan(camera.fov * 0.5);
        vec3 rayDir = normalize(vec3(uv.x, uv.y, -z));
        Ray r = setWorldRay(rayDir);
        sum_color+=Trace(r);
    }
    return sum_color/RANDOM_SAMPLING_SIZE;
}

void main() {
    seed = uint(gl_FragCoord.x*100000) * 1973u + uint(gl_FragCoord.y*100000) * 9277u + uint(frame) * 26699u;

    vec4 color = randomRayTracing();

    float lambda = 1.0/float(frame);
    vec4 bufferColor = vec4(texture(readTex,uvScreen).rgb,1);
    fragColor = lambda*color + bufferColor*(1-lambda);

    // shader testing...
    //    vec4 screenPos = camera.projection * camera.view *vec4(uvScreen,0.0,1.0);
    //    fragColor = screenPos;
}