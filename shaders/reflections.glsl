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

vec3 refraction(vec3 L, vec3 n, float cosI, float eta, float sin2T){
    float cosT = sqrt(1.0 - sin2T);
    return eta * L + (eta * cosI - cosT) * n;
}

Reflection sampleLambertReflection(vec3 L, vec3 N, float lambdaUm, Material nextMaterial){
    return Reflection(normalize(normalize(N) + randomUnitVector()), albedoAt(nextMaterial, lambdaUm));
}

Reflection sampleMetalReflection(vec3 L, vec3 N, float lambdaUm, Material nextMaterial){
    return Reflection(reflect(L, N) + randomUnitVector()*nextMaterial.metal_fuzz, albedoAt(nextMaterial, lambdaUm));
}

float fresnelDielectric(float cosI, float n1, float n2){
    float sinT = (n1 / n2) * sqrt(max(0.0, 1.0 - cosI * cosI));
    if (sinT >= 1.0) return 1.0;                  // total internal reflection
    float cosT = sqrt(1.0 - sinT * sinT);
    float rs = (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
    float rp = (n2 * cosI - n1 * cosT) / (n2 * cosI + n1 * cosT);
    return 0.5 * (rs * rs + rp * rp);
}

//Reflection sampleGlassReflection(vec3 L, vec3 N, float lambdaUm, Material prev, Material next){
//    vec3 n = dot(L, N) < 0.0 ? N : -N;
//    float cosI = min(dot(-L, n), 1.0);
//    float eta_prev = iorSellmeier(prev.B, prev.C, lambdaUm);
//    float eta_next = iorSellmeier(next.B, next.C, lambdaUm);
//    float F = fresnelDielectric(cosI, eta_prev, eta_next);
//    if (F > rand())
//    { return Reflection(reflect(L, n), 1.0); }
//
//    return Reflection(refraction(L, n, eta_prev, eta_next), albedoAt(next, lambdaUm));
//}

float fresnelDielectric(float cosI, cx n1, cx n2){
    cx sinT = cmul(cdiv(n1,n2), sqrt(max(0.0, 1.0 - cosI * cosI)));
    cx cosT = csqrt(cdiff(cx(1.0, 0.0), cmul(sinT, sinT)));
    cx rs = cdiv(cdiff(cmul(n1,cosI), cmul(n2,cosT)), csum(cmul(n1,cosI), cmul(n2,cosT)));
    cx rp = cdiv(cdiff(cmul(n2,cosI), cmul(n1,cosT)), csum(cmul(n2,cosI), cmul(n1,cosT)));
    return 0.5 * (norm2(rs) + norm2(rp));
}

Reflection sampleGlassReflection(vec3 L, vec3 N, float t, float lambdaUm, Material prev, Material next){
    // evaluate reflection and refraction
    vec3 n = dot(L, N) < 0.0 ? N : -N;
    float cosI = min(dot(-L, n), 1.0);
    cx eta_prev = complexIndex(lambdaUm, prev.n, prev.k);
    cx eta_next = complexIndex(lambdaUm, next.n, next.k);
    float F = fresnelDielectric(cosI, eta_prev, eta_next);
    if (F > rand())
    { return Reflection(reflect(L, n), 1.0); }
    else if (abs(eta_next.b) > 0.1){ // if the ray hasn't been reflected and k >> 0, then the ray has been absorbed.
        return Reflection(reflect(L, n), 0.0);
    }
    float eta   = eta_prev.a / eta_next.a;
    float sin2T = eta * eta * (1.0 - cosI * cosI);
    if (sin2T >= 1.0) { return Reflection(reflect(L, n), 1.0); }

    // evaluate intensity fall over a long distance with beer-lambert
    float distance_attenuation = exp(-(4*PI*eta_prev.b/(lambdaUm*10E-6))*t*ATTENUATION_COEF);

    return Reflection(refraction(L, n, cosI, eta, sin2T), distance_attenuation);
}

Reflection sampleReflection(vec3 L, vec3 N, float t, float lambdaUm, Material previousMaterial, Material nextMaterial){
    if (nextMaterial.id == 1) {return sampleLambertReflection(L,N, lambdaUm, nextMaterial);}
    if (nextMaterial.id == 2) {return sampleMetalReflection(L,N, lambdaUm, nextMaterial);}
    if (nextMaterial.id == 3) {return sampleGlassReflection(L,N, t, lambdaUm, previousMaterial ,nextMaterial);}
    else {return Reflection(vec3(0), 0);}
}

#endif
