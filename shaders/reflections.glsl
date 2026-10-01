#ifndef REFLECTIONS_GLSL
#define REFLECTIONS_GLSL

#include "random.glsl"
#include "distributions.glsl"
#include "params.glsl"
#include "material.glsl"

struct Reflection{
    vec3 R;
    float coef;
};

vec3 reflect(vec3 L, vec3 N){
    return L - 2*dot(L,N)*N;
}

Reflection sampleLambertReflection(vec3 L, vec3 N, float lambdaUm, Material nextMaterial){
    return Reflection(normalize(normalize(N) + randomUnitVector()), albedoAt(nextMaterial, lambdaUm));
}

Reflection sampleMetalReflection(vec3 L, vec3 N, float lambdaUm, Material nextMaterial){
    return Reflection(reflect(L, N) + randomUnitVector()*nextMaterial.metal_fuzz, albedoAt(nextMaterial, lambdaUm));
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

Reflection sampleGlassReflection(vec3 L, vec3 N, float lambdaUm, Material prev, Material next){
    vec3 n = dot(L, N) < 0.0 ? N : -N;
    float cosI = min(dot(-L, n), 1.0);
//    float eta_prev = iorSellmeier(prev.B, prev.C, lambdaUm);
//    float eta_next = iorSellmeier(next.B, next.C, lambdaUm);
    float eta_prev = prev.eta_test;
    float eta_next = next.eta_test;
    float F = fresnelDielectric(cosI, eta_prev, eta_next);

    if (F > rand())
    { return Reflection(reflect(L, n), 1.0); }

    return Reflection(refraction(L, n, eta_prev, eta_next), albedoAt(next, lambdaUm));
}

Reflection sampleReflection(vec3 L, vec3 N, float lambdaUm, Material previousMaterial, Material nextMaterial){
    if (nextMaterial.id == 1) {return sampleLambertReflection(L,N, lambdaUm, nextMaterial);}
    if (nextMaterial.id == 2) {return sampleMetalReflection(L,N, lambdaUm, nextMaterial);}
    if (nextMaterial.id == 3) {return sampleGlassReflection(L,N, lambdaUm, previousMaterial ,nextMaterial);}
//    if (nextMaterial.id == 4)
    else {return Reflection(vec3(0), 0);}
}

#endif
