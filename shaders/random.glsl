#ifndef RANDOM_GLSL
#define RANDOM_GLSL

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

int randint(int min, int max){
    return int(floor(min + rand()*(max - min)));
}

vec3 randomUnitVector(){
    return normalize(vec3((rand()-0.5)*2,(rand()-0.5)*2,(rand()-0.5)*2));
}

#endif