#ifndef DISTRIBUTIONS_GLSL
#define DISTRIBUTIONS_GLSL

#include "params.glsl"
#include "random.glsl"
#include "material.glsl"
#include "complex_numbers.glsl"

// all wavelengths are in µm.

// material is the index of the material in the materials buffer.
cx complexIndex(float lambdaUm, uint material) {
    float t = clamp((lambdaUm - LAMBDA_MIN) / (LAMBDA_MAX - LAMBDA_MIN), 0.0, 1.0);
    float f = t * float(N_BINS - 1);
    int i0 = int(floor(f));
    int i1 = min(i0 + 1, N_BINS - 1);
    float eval_n = mix(materials[material].n[i0], materials[material].n[i1], f - float(i0));
    float eval_k = mix(materials[material].k[i0], materials[material].k[i1], f - float(i0));
    return cx(eval_n,eval_k);
}

float iorSellmeier(vec3 B, vec3 C, float lambdaUm) {
    float l2 = lambdaUm * lambdaUm;
    return sqrt(1.0 + dot(B, l2 / (vec3(l2) - C)));
}

float sampleWLVisible(){
    return LAMBDA_MIN + rand() * (LAMBDA_MAX - LAMBDA_MIN);
}

//float sampleWLVisible() {
//    float nm = 538.0 - 138.888889 * atanh(0.85691062 - 1.82750197 * rand());
//    return nm * 0.001;
//}

//float lambdaPdfUm(float lUm) {
//    float c = cosh(0.0072 * (lUm * 1000.0 - 538.0));
//    return 1000.0 * 0.0039398042 / (c * c);
//}

const float C2 = 1.438777e-2;          // hc/k  [m*K]

float planckPdfUm(float lambdaUm, float T) {
    float lam = lambdaUm * 1e-6;
    float x   = C2 / (lam * T);
//    float em1 = (x < eps) ? x * (1.0 + 0.5 * x) : exp(x) - 1.0;
    float em1 = exp(x) - 1.0;
    float px  = (15.0 / (PI*PI*PI*PI)) * x*x*x / em1;
    return px * x / lambdaUm;
}

float albedoAt(uint material, float lambdaUm) {
    float t = clamp((lambdaUm - LAMBDA_MIN) / (LAMBDA_MAX - LAMBDA_MIN), 0.0, 1.0);
    float f = t * float(N_BINS - 1);
    int i0 = int(floor(f));
    int i1 = min(i0 + 1, N_BINS - 1);
    return mix(materials[material].albedo[i0], materials[material].albedo[i1], f - float(i0));
}

float g(float l, float mu, float s1, float s2) {
    float t = (l - mu) / (l < mu ? s1 : s2);
    return exp(-0.5 * t * t);
}

vec3 cieXYZ(float lambdaUm) {
    float l = lambdaUm * 1000.0; // -> nm
    float x = 1.056 * g(l, 599.8, 37.9, 31.0) + 0.362 * g(l, 442.0, 16.0, 26.7)
    - 0.065 * g(l, 501.1, 20.4, 26.2);
    float y = 0.821 * g(l, 568.8, 46.9, 40.5) + 0.286 * g(l, 530.9, 16.3, 31.1);
    float z = 1.217 * g(l, 437.0, 11.8, 36.0) + 0.681 * g(l, 459.0, 26.0, 13.8);
    return vec3(x, y, z);
}

const mat3 XYZ_TO_SRGB = mat3(
        3.2404542, -0.9692660,  0.0556434,
        -1.5371385,  1.8760108, -0.2040259,
        -0.4985314,  0.0415560,  1.0572252
);

vec3 xyzToSrgb(vec3 xyz){
    return XYZ_TO_SRGB*xyz;
}

#endif