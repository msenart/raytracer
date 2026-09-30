#ifndef DISTRIBUTIONS_GLSL
#define DISTRIBUTIONS_GLSL

#include "params.glsl"
#include "random.glsl"

// all wavelengths are in µm.

float iorSellmeier(vec3 B, vec3 C, float l) {
    float l2 = l * l;
    return sqrt(1.0 + dot(B, l2 / (vec3(l2) - C)));
}

float sampleWLVisible(){
    return LAMBDA_MIN + rand() * (LAMBDA_MAX - LAMBDA_MIN);
}

const float PI      = 3.14159265359;
const float C2      = 1.438777e-2;          // hc/k  [m*K]

float planckPdfNm(float lambdaUm, float T) {
    float lam = lambdaUm * 1e-6;
    float x   = C2 / (lam * T);
    float em1 = (x < 1e-3) ? x * (1.0 + 0.5 * x) : exp(x) - 1.0;
    float px  = (15.0 / (PI*PI*PI*PI)) * x*x*x / em1;
    return px * x / lambdaUm;
}

#define N_BINS 16

float reflectanceAt(Material m, float lambdaUm) {
    float t = clamp((lambdaUm - LAMBDA_MIN) / (LAMBDA_MAX - LAMBDA_MIN), 0.0, 1.0);
    float f = t * float(N_BINS - 1);
    int i0 = int(floor(f));
    int i1 = min(i0 + 1, N_BINS - 1);
    return mix(m.reflectance[i0], m.reflectance[i1], f - float(i0));
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