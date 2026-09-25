#version 450 core

#include "camera.glsl"
#include "sphereSDF.glsl"
#include "params.glsl"
#include "ray.glsl"
#include "reflections.glsl"

out vec4 fragColor;
in vec2 uvScreen;

uniform Camera camera;
uniform uint frame;

Sphere[MAX_MESH] spheres;
uint spheres_size = 0;


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

                if (spheres[s_id].emissive){ // if the mesh was emissive
                    return cur_r.throughput*spheres[s_id].color;
                }
                else { // if the mesh is not emissive
                    vec3 N = normalToSphere(x,cur_r.d,spheres[s_id]);
                    cur_r = Ray(x + 2*eps*N ,sampleLambertReflection(cur_r.d, N) ,cur_r.throughput*spheres[s_id].color);
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

void main() {
    seed = uint(gl_FragCoord.x*100000) * 1973u + uint(gl_FragCoord.y*100000) * 9277u + uint(frame) * 26699u;


    Sphere s0 = Sphere(
    vec3(0.f,0.f,5.f),
    0.5f,
    vec4(1.f, 1.f, 1.f, 1.f),
    false
    );
    spheres[0] = s0;
    spheres_size++;

    Sphere s1 = Sphere(
    vec3(1.f,0.f,5.f),
    0.5f,
    vec4(1.f, 1.f, 1.f, 1.f),
    true
    );
    spheres[1] = s1;
    spheres_size++;

    vec2 uv = uvScreen;
    uv.x *= camera.aspectRatio;
    float z = 1.0 / tan(camera.fov * 0.5);
    vec3 rayDir = normalize(vec3(uv.x, uv.y, -z));
    rayDir = normalize(mat3(inverse(camera.view)) * rayDir);
    Ray r = {camera.position,rayDir,vec4(1,1,1,1)};

    vec4 color = Trace(r);

    fragColor = color;

    // shader testing...
//    vec4 screenPos = camera.projection * camera.view *vec4(uvScreen,0.0,1.0);
//    fragColor = screenPos;
}