#ifndef RANDOM_GLSL
#define RANDOM_GLSL

#include "params.glsl"

uint seed;

uint hash(uint seed) {
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float rand() {
    seed = hash(seed);
    return float(seed) / float(0xffffffffu);
}

float rand(float a, float b){
    return a + (b-a)*rand();
}

int randint(int min, int max){
    return int(floor(min + rand()*(max - min)));
}

vec3 randomUnitVector(){
    float z = rand(-1,1);
    float theta = rand(0,2*PI);
    float r = sqrt(1-z*z);
    return normalize(vec3(r*cos(theta), r*sin(theta), z));
}

vec2 randomUnitVectorInDisk(){
    float r = rand();
    float theta = rand()*2*PI;
    return vec2(r*cos(theta),r*sin(theta));
}


#endif