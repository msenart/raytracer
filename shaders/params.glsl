#ifndef PARAMS_GLSL
#define PARAMS_GLSL

#define N_BINS 16


// PARAMETERS (you can modify them as you want.)
uniform uint MAX_ITER;
uniform float MAX_DISTANCE;
uniform float BACKGROUND_INTENSITY;
uniform float eps;
const uint MULTISAMPLING_SIZE = 9;
uniform uint RANDOM_SAMPLING_SIZE;
uniform float LAMBDA_MIN;
uniform float LAMBDA_MAX;
uniform float ATTENUATION_COEF;
uniform float PI;

// CONSTANTS (you shall not modify them.)

const uint UINT_MAX = 0xFFFFFFFFu;
const float FLOAT_MAX = 1e30;

const float _PAD_0 = 0;
const float _PAD_1 = 0;
const float _PAD_2 = 0;
const float _PAD_3 = 0;
const float _PAD_4 = 0;

#endif