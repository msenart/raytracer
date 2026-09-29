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

struct Reflection{
    vec3 R;
    vec4 coef;
};

vec3 reflect(vec3 L, vec3 N){
    return L - 2*dot(L,N)*N;
}

Reflection sampleLambertReflection(vec3 L, vec3 N, Material nextMaterial){
    return Reflection(normalize(normalize(N) + randomUnitVector()), vec4(nextMaterial.color));
}

Reflection sampleMetalReflection(vec3 L, vec3 N, Material nextMaterial){
    return Reflection(reflect(L, N) + randomUnitVector()*nextMaterial.metal_fuzz, vec4(nextMaterial.color));
}

vec3 refraction(vec3 L, vec3 N, float previous_eta, float next_eta){
    vec3 correctedN = dot(L, N) < 0.0 ? N : -N;
    float etai_over_etat = previous_eta/next_eta;
    float cos_theta = min(dot(-L, correctedN),1.0f);
    vec3 r_out_perp = etai_over_etat*(L + cos_theta*correctedN);
    vec3 r_out_parallel = -sqrt(abs(1.0f - dot(r_out_perp,r_out_perp)))*correctedN;
    return normalize(r_out_perp + r_out_parallel);
}

float fresnelDielectric(float cosI, float n1, float n2){
    float sinT = (n1 / n2) * sqrt(max(0.0, 1.0 - cosI * cosI));
    if (sinT >= 1.0) return 1.0;                  // total internal reflection
    float cosT = sqrt(1.0 - sinT * sinT);
    float rs = (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
    float rp = (n2 * cosI - n1 * cosT) / (n2 * cosI + n1 * cosT);
    return 0.5 * (rs * rs + rp * rp);
}

Reflection sampleGlassReflection(vec3 L, vec3 N, Material prev, Material next){
    vec3 n = dot(L, N) < 0.0 ? N : -N;
    float cosI = min(dot(-L, n), 1.0);
    float F = fresnelDielectric(cosI, prev.eta, next.eta);

    if (F > rand())
    { return Reflection(reflect(L, n), vec4(1.0)); }

    return Reflection(refraction(L, n, prev.eta, next.eta), next.color);
}

Reflection sampleReflection(vec3 L, vec3 N, Material previousMaterial, Material nextMaterial){
    if (nextMaterial.id == 1) {return sampleLambertReflection(L,N, nextMaterial);}
    if (nextMaterial.id == 2) {return sampleMetalReflection(L,N, nextMaterial);}
    if (nextMaterial.id == 3) {return sampleGlassReflection(L,N, previousMaterial ,nextMaterial);}
//    if (nextMaterial.id == 4)
    else {return Reflection(vec3(0), vec4(0));}
}

#endif
