#version 450 core

#include "camera.glsl"
#include "sphereSDF.glsl"
#include "params.glsl"

out vec4 fragColor;
in vec2 uvScreen;

uniform Camera camera;

Sphere[MAX_MESH] spheres;
uint spheres_size = 0;

struct Ray {
    vec3 o;
    vec3 d;
    vec4 throughput;
};


vec4 Trace(Ray r){
    float t = 0;
    float min_distance = 10000.f;
    for (uint iter = 0;iter<MAX_ITER;iter++){
        for (uint s_id = 0; s_id < spheres_size; s_id++){ // sweep all meshes and find the closest distance
            vec3 x = r.o + r.d*t;
            float cur_distance = distanceToSphere(x,spheres[s_id]);
            min_distance = min(min_distance,cur_distance);
            if (min_distance <= eps){ // if the ray has hit a mesh
                return vec4(normalToSphere(x,spheres[s_id]),1.0);
            }
            else if (t > MAX_DISTANCE){ // if the ray "has fled away", return background color
                return BACKGROUND_COLOR;
            }
        }
        t+=abs(min_distance);
    }

    // if the algorithm hasn't converged, return the background color
    return BACKGROUND_COLOR;
}

void main() {
    Sphere s = {
    vec3(0.f,0.f,5.f),
    0.5f,
    vec4(1.f, 1.f, 1.f, 1.f)
    };
    spheres[0] = s;
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