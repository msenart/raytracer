#ifndef REFLECTIONS_GLSL
#define REFLECTIONS_GLSL

#include "random.glsl"

struct Material{
    vec4 color;
    uint id;
    float metal_fuzz;
    float eta;
    float _pad0;
};

vec3 reflect(vec3 L, vec3 N){
    return L - 2*dot(L,N)*N;
}

vec3 sampleLambertReflection(vec3 L, vec3 N){
    return normalize(normalize(N) + randomUnitVector());
}

vec3 sampleMetalReflection(vec3 L, vec3 N, float metal_fuzz){
    return reflect(L, N) + randomUnitVector()*metal_fuzz;
}

vec3 refraction(vec3 L, vec3 N, float previous_eta, float next_eta){
    vec3 correctedN = dot(L, N) < 0.0 ? N : -N;
    float etai_over_etat = previous_eta/next_eta;
    float cos_theta = min(dot(-L, correctedN),1.0f);
    vec3 r_out_perp = etai_over_etat*(L + cos_theta*correctedN);
    vec3 r_out_parallel = -sqrt(abs(1.0f - dot(r_out_perp,r_out_perp)))*correctedN;
    return normalize(r_out_perp + r_out_parallel);
}

float reflectance(float cosine, float previous_eta, float next_eta) {
    // Use Schlick's approximation for reflectance.
    float ri = previous_eta/next_eta;
    float r0 = (1 - ri) / (1 + ri);
    r0 = r0*r0;
    return r0 + (1-r0)*pow((1 - cosine),5);
}

vec3 sampleGlassReflection(vec3 L, vec3 N, float previous_eta, float next_eta){
    vec3 correctedN = dot(L, N) < 0.0 ? N : -N;
    float cos_theta = min(dot(-L, correctedN), 1.0);
    float sin_theta = sqrt(1.0 - cos_theta*cos_theta);
    vec3 direction;
    if ((previous_eta/next_eta) * sin_theta > 1.0 || reflectance(cos_theta, previous_eta, next_eta) > rand()){ // cannot refract with Snell-Descartes Law
        direction = reflect(L, N);
    }
    else { // can refract with Snell-Descartes
        direction = refraction(L, N, previous_eta, next_eta);
    }
    return direction;
}

vec3 sampleReflection(vec3 L, vec3 N, Material previousMaterial, Material nextMaterial){
    if (nextMaterial.id == 1) {return sampleLambertReflection(L,N);}
    if (nextMaterial.id == 2) {return sampleMetalReflection(L,N, nextMaterial.metal_fuzz);}
    if (nextMaterial.id == 3) {return sampleGlassReflection(L,N, previousMaterial.eta ,nextMaterial.eta);}
    else {return vec3(0,0,0);}
}

#endif
