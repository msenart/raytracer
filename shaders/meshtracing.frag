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

Sphere[MAX_MESH] spheres;
uint spheres_size = 0;

//Material SSBO

layout(std430, binding = 2) readonly buffer MaterialBuffer {
    Material materials[];
};

// Core raytracing mechanic
const Material airMaterial = Material(vec4(1.0), 2u, 0.0, 1.0, 0.0);

vec4 Trace(Ray r) {
    float min_distance = FLOAT_MAX;
    uint argmin_mesh_type = UINT_MAX;
    uint argmin_mesh = UINT_MAX;
    uint argmin_triangle = UINT_MAX; // you'll want this for shading triangles
    Ray cur_r = r;
    HitProperties argmin_hp = HitProperties(false, 0, vec2(0), vec3(0));
    for (uint depth = 0; depth < MAX_ITER; depth++){
        min_distance = FLOAT_MAX;
        argmin_mesh_type = UINT_MAX;
        argmin_mesh = UINT_MAX;
        argmin_triangle = UINT_MAX;
        argmin_hp = HitProperties(false, 0, vec2(0), vec3(0));
        for (uint i = 0; i < indices.length(); i += 3) { // handling triangles (mesh_type = 0)
            HitProperties hp = isRayIntersectingTriangle(cur_r, indices[i].index, indices[i+1].index, indices[i+2].index, false); // materials[argmin_mesh].normalInterpolation
            if (hp.hasHit && hp.t < min_distance) {
                min_distance = hp.t;
                argmin_mesh_type = 0;
                argmin_mesh = indices[i].mesh_id;
                argmin_triangle = i; // remember which triangle, for shading later
                argmin_hp = hp;
            }
        }
        for (uint i = 0; i < spheres_size; i++) { // handling spheres equations (mesh_type = 1)
            HitProperties hp = isRayIntersectingSphere(cur_r,spheres[i]);
            if (hp.hasHit && hp.t < min_distance) {
                min_distance = hp.t;
                argmin_mesh_type = 1;
                argmin_mesh = i;
            }
        }
        if (argmin_mesh < UINT_MAX){ // if the ray has hit a mesh
            if (materials[argmin_mesh].id == 0){ // the mesh is emissive
                return materials[argmin_mesh].color*cur_r.throughput;
            }
            vec3 N = argmin_hp.normal;
            vec3 O = cur_r.o + cur_r.d*argmin_hp.t;
            // we assume that there is NO intersection between the spheres, otherwise it is complicated.
            // when going OUT of the sphere, it is always Material -> Air
            // when going IN the sphere, it is always Air -> Material
            bool isGoingOut = argmin_mesh == cur_r.in_obj_id;
            Material previousMaterial = isGoingOut ? materials[cur_r.in_obj_id] : airMaterial; // Somewhere here, we need a mean to use argmin_mesh_type so to look in the right material.
            Material nextMaterial = isGoingOut ? airMaterial : materials[argmin_mesh];
            vec3 R = sampleReflection(cur_r.d, N, previousMaterial, nextMaterial);

            float isReflection = -sign(dot(R,N)*dot(cur_r.d,N)); // 1 if reflection, -1 if refraction.
            uint new_in_mesh_id = isReflection == 1.f ? cur_r.in_obj_id :
            (isGoingOut ? MAX_MESH : argmin_mesh);
            cur_r = Ray(O + isReflection*2*eps*N,R,cur_r.throughput*nextMaterial.color,new_in_mesh_id);
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
    return Ray(camera.position, worldRayDir, vec4(1, 1, 1, 1), MAX_MESH);
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
    return Ray(worldOrigin, worldDirection, vec4(1, 1, 1, 1), MAX_MESH);
}

// Basic raytracing
vec4 basicRayTracing(){
    vec2 uv = uvScreen;
    uv = (uv - 0.5) * 2;
    uv.x *= camera.aspectRatio;
    float z = 1.0 / tan(camera.fov * 0.5);
    vec3 rayDir = normalize(vec3(uv.x, uv.y, -z));
    rayDir = normalize(mat3(inverse(camera.view)) * rayDir);
    Ray r = { camera.position, rayDir, vec4(1, 1, 1, 1), MAX_MESH};
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
        Ray r = { camera.position, rayDir, vec4(1, 1, 1, 1), MAX_MESH};
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

//    Sphere s0 = Sphere(
//            vec3(0.f,0.f,5.f),
//            0.5f,
//            Material(vec4(1,1,1,1),1,0.f,1.f,0.f));
//    spheres[0] = s0;
//    spheres_size++;
//
//    Sphere s1 = Sphere(
//            vec3(2.f,0.f,5.f),
//            0.5f,
//            Material(vec4(1,1,1,1),3,0.0f,1.2f,0.f));
//    spheres[1] = s1;
//    spheres_size++;
//
//    Sphere s2 = Sphere(
//            vec3(0.f,-101.f,0.f),
//            100.f,
//            Material(vec4(.5f,1,.5f,1),1,0.f,1.f,0.f));
//    spheres[2] = s2;
//    spheres_size++;

    vec4 color = randomRayTracing();

    float lambda = 1.0/float(frame);
    vec4 bufferColor = vec4(texture(readTex,uvScreen).rgb,1);
    fragColor = lambda*color + bufferColor*(1-lambda);

    // shader testing...
    //    vec4 screenPos = camera.projection * camera.view *vec4(uvScreen,0.0,1.0);
    //    fragColor = screenPos;
}