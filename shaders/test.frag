#version 450 core

#include "camera.glsl"
#include "sphereSDF.glsl"
#include "params.glsl"
#include "ray.glsl"
#include "reflections.glsl"

uniform sampler2D readTex;

out vec4 fragColor;
in vec2 uvScreen;

uniform Camera camera;
uniform vec2 windowSize;
uniform uint frame;

Sphere[MAX_MESH] spheres;
uint spheres_size = 0;

// Core raytracing mechanic
vec4 Trace(Ray r){
    float t = 0;
    float min_distance = 10000.f;
    uint argmin_distance = MAX_MESH;
    Ray cur_r = r;
    for (uint iter = 0;iter<MAX_ITER;iter++){
        min_distance = 10000.f;
        for (uint s_id = 0; s_id < spheres_size; s_id++){ // sweep all meshes and find the closest distance
            vec3 x = cur_r.o + cur_r.d*t;
            float cur_distance = distanceToSphere(x,spheres[s_id]);
            min_distance = min(min_distance,cur_distance);
            argmin_distance = min_distance == cur_distance ? s_id : argmin_distance;
            if (min_distance <= eps){ // if the ray has hit a mesh

                if (spheres[s_id].material.emissive){ // if the mesh was emissive
                    return cur_r.throughput*spheres[s_id].material.color;
                }
                else { // if the mesh is not emissive
                    vec3 N = normalToSphere(x,cur_r.d,spheres[s_id]);
                    cur_r = Ray(x + 2*eps*N ,sampleReflection(cur_r.d, N,spheres[s_id].material) ,cur_r.throughput*spheres[s_id].material.color);
                    t = 0;
                    argmin_distance = MAX_MESH;
                    break;
                }

            }
            else if (t > MAX_DISTANCE){ // if the ray "has fled away", return background color
                return cur_r.throughput*BACKGROUND_COLOR;
            }
        }
        t+=abs(min_distance);
    }

    // if the algorithm hasn't converged, return the background color
    return cur_r.throughput*BACKGROUND_COLOR;
}


// Basic raytracing
vec4 basicRayTracing(){
    vec2 uv = uvScreen;
    uv = (uv - 0.5) * 2;
    uv.x *= camera.aspectRatio;
    float z = 1.0 / tan(camera.fov * 0.5);
    vec3 rayDir = normalize(vec3(uv.x, uv.y, -z));
    rayDir = normalize(mat3(inverse(camera.view)) * rayDir);
    Ray r = { camera.position, rayDir, vec4(1, 1, 1, 1) };
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
    vec4 sum_color = vec4(0,0,0,0);
    for (uint ray_id = 0; ray_id < MULTISAMPLING_SIZE*MULTISAMPLING_SIZE; ray_id++)
    {
        vec2 uv = uvs[ray_id];
        uv = (uv - 0.5) * 2;
        uv.x *= camera.aspectRatio;
        float z = 1.0 / tan(camera.fov * 0.5);
        vec3 rayDir = normalize(vec3(uv.x, uv.y, -z));
        rayDir = normalize(mat3(inverse(camera.view)) * rayDir);
        Ray r = { camera.position, rayDir, vec4(1, 1, 1, 1) };
        sum_color += Trace(r);
    }

    return sum_color/(MULTISAMPLING_SIZE*MULTISAMPLING_SIZE);
}

// Randomsampling raytracing
vec4 randomRayTracing(){
    vec2 uv = uvScreen + (vec2(rand(), rand()) - 0.5) / windowSize;
    uv = (uv - 0.5) * 2;
    uv.x *= camera.aspectRatio;
    float z = 1.0 / tan(camera.fov * 0.5);
    vec3 rayDir = normalize(vec3(uv.x, uv.y, -z));
    rayDir = normalize(mat3(inverse(camera.view)) * rayDir);
    Ray r = { camera.position, rayDir, vec4(1, 1, 1, 1) };
    return Trace(r);
}

void main() {
    seed = uint(gl_FragCoord.x*100000) * 1973u + uint(gl_FragCoord.y*100000) * 9277u + uint(frame) * 26699u;

    Sphere s0 = Sphere(
            vec3(0.f,0.f,5.f),
            0.5f,
            Material(0,vec4(1,1,1,1),true));
    spheres[0] = s0;
    spheres_size++;

    Sphere s1 = Sphere(
            vec3(1.f,0.f,5.f),
            0.5f,
            Material(1,vec4(0.9,0.9,0.9,1),false));
    spheres[1] = s1;
    spheres_size++;

    Sphere s2 = Sphere(
            vec3(0.f,-101.f,0.f),
            100.f,
            Material(0,vec4(1,1,1,1),false));
    spheres[2] = s2;
    spheres_size++;

    vec4 color = randomRayTracing();

    float lambda = 1.0/float(frame);
    vec4 bufferColor = vec4(texture(readTex,uvScreen).rgb,1);
    fragColor = lambda*color + bufferColor*(1-lambda);

    // shader testing...
//    vec4 screenPos = camera.projection * camera.view *vec4(uvScreen,0.0,1.0);
//    fragColor = screenPos;
}