#pragma once

#include <array>

constexpr double M_PI = 3.14159265358979323846;

inline int WINDOW_WIDTH = 1920;
inline int WINDOW_HEIGHT = 1080;

// you can't change this value, as it permits to have regular evaluation of n and k in the visible domain (visible domain won't be changed too).
inline static constexpr unsigned int N_BINS = 16;

const static std::string SHADER_FOLDER_PATH = "../shaders/";

static constexpr std::array<float, N_BINS> N_DIAMOND {
    2.4602, 2.4496, 2.4392, 2.4320,
    2.4289, 2.4260, 2.4227, 2.4189,
    2.4148, 2.4111, 2.4093, 2.4075,
    2.4065, 2.4064, 2.4063, 2.4062
};
static constexpr std::array<float, N_BINS> K_DIAMOND {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};

static constexpr std::array<float, N_BINS> N_SAPPHIRE{
    1.7560,1.7540,1.7540,1.7540,
    1.7540,1.7530,1.7530,1.7520,
    1.7520,1.7510,1.7510,1.7510,
    1.7510,1.7505,1.7500,1.7500
};

static constexpr std::array<float, N_BINS> K_SAPPHIRE{
    0.019000,0.019500,0.021000,0.021000,
    0.021000,0.020500,0.021000,0.020500,
    0.020000,0.020500,0.020000,0.020000,
    0.020000,0.020000,0.020000,0.020000
};

static constexpr std::array<float, N_BINS> N_RUBY{
    1.7866f, 1.7810f, 1.7775f, 1.7748f,
    1.7727f, 1.7709f, 1.7694f, 1.7681f,
    1.7670f, 1.7660f, 1.7652f, 1.7645f,
    1.7638f, 1.7632f, 1.7626f, 1.7621f
};

static constexpr std::array<float, N_BINS> K_RUBY{
    4.77e-5f, 4.40e-5f, 1.79e-5f, 9.45e-6f,
    1.99e-5f, 5.85e-5f, 8.75e-5f, 7.32e-5f,
    3.34e-5f, 9.95e-6f, 3.10e-6f, 1.61e-6f,
    2.23e-6f, 1.15e-6f, 5.97e-7f, 6.17e-7f
};

static constexpr std::array<float, N_BINS> N_GOLD{
    1.468f, 1.456f, 1.417f, 1.349f,
    1.215f, 0.971f, 0.635f, 0.489f,
    0.382f, 0.299f, 0.249f, 0.205f,
    0.172f, 0.140f, 0.135f, 0.131f
};

static constexpr std::array<float, N_BINS> K_GOLD{
    1.953f, 1.954f, 1.932f, 1.885f,
    1.843f, 1.874f, 2.072f, 2.339f,
    2.594f, 2.837f, 3.074f, 3.304f,
    3.503f, 3.702f, 3.882f, 4.062f
};

static constexpr std::array<float, N_BINS> N_COPPER{
    1.313f, 1.268f, 1.245f, 1.244f,
    1.239f, 1.213f, 1.181f, 1.070f,
    0.911f, 0.720f, 0.494f, 0.294f,
    0.257f, 0.220f, 0.215f, 0.211f
};

static constexpr std::array<float, N_BINS> K_COPPER{
    2.131f, 2.245f, 2.348f, 2.435f,
    2.511f, 2.571f, 2.606f, 2.587f,
    2.620f, 2.696f, 2.962f, 3.246f,
    3.499f, 3.752f, 3.956f, 4.159f
};

static constexpr std::array<float, N_BINS> N_RUTILE{
    2.9960f, 2.8890f, 2.8126f, 2.7555f,
    2.7114f, 2.6764f, 2.6479f, 2.6245f,
    2.6049f, 2.5884f, 2.5742f, 2.5619f,
    2.5513f, 2.5419f, 2.5336f, 2.5263f
};

static constexpr std::array<float, N_BINS> K_RUTILE{
    1.6e-3f, 5.1e-5f, 4.0e-6f, 1.5e-6f,
    8.0e-7f, 6.0e-7f, 5.0e-7f, 4.7e-7f,
    4.8e-7f, 5.0e-7f, 5.2e-7f, 5.4e-7f,
    5.6e-7f, 5.8e-7f, 6.0e-7f, 6.2e-7f
};

static constexpr std::array<float, N_BINS> N_ALEXANDRITE{
    1.7634f, 1.7601f, 1.7574f, 1.7551f,
    1.7531f, 1.7514f, 1.7500f, 1.7487f,
    1.7476f, 1.7466f, 1.7457f, 1.7449f,
    1.7442f, 1.7436f, 1.7430f, 1.7425f
};

static constexpr std::array<float, N_BINS> K_ALEXANDRITE{
    1.91e-5f, 2.37e-5f, 1.43e-5f, 5.7e-6f,
    4.0e-6f,  7.5e-6f,  2.19e-5f, 4.12e-5f,
    3.34e-5f, 1.24e-5f, 4.1e-6f,  2.7e-6f,
    2.2e-6f,  2.0e-6f,  1.8e-6f,  1.9e-6f
};