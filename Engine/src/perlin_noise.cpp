#include "Engine/perlin_noise.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>

PerlinNoise::PerlinNoise(uint32_t seed) {
    // Initialize permutation table with values 0-255
    permutation.resize(256);
    std::iota(permutation.begin(), permutation.end(), 0);

    // Shuffle the permutation table based on seed
    std::default_random_engine engine(seed);
    std::shuffle(permutation.begin(), permutation.end(), engine);

    // Duplicate the permutation table to avoid overflow
    permutation.insert(permutation.end(), permutation.begin(), permutation.end());
}

float PerlinNoise::noise(float x, float y, float z) const {
    // Find unit cube that contains point
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;

    // Find relative x, y, z of point in cube
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    // Compute fade curves for x, y, z
    float u = fade(x);
    float v = fade(y);
    float w = fade(z);

    // Hash coordinates of the 8 cube corners
    int A = permutation[X] + Y;
    int AA = permutation[A] + Z;
    int AB = permutation[A + 1] + Z;
    int B = permutation[X + 1] + Y;
    int BA = permutation[B] + Z;
    int BB = permutation[B + 1] + Z;

    // Blend results from 8 corners of cube
    float res = lerp(w,
        lerp(v,
            lerp(u, grad(permutation[AA], x, y, z),
                    grad(permutation[BA], x - 1, y, z)),
            lerp(u, grad(permutation[AB], x, y - 1, z),
                    grad(permutation[BB], x - 1, y - 1, z))),
        lerp(v,
            lerp(u, grad(permutation[AA + 1], x, y, z - 1),
                    grad(permutation[BA + 1], x - 1, y, z - 1)),
            lerp(u, grad(permutation[AB + 1], x, y - 1, z - 1),
                    grad(permutation[BB + 1], x - 1, y - 1, z - 1))));

    return res;
}

float PerlinNoise::fbm(float x, float y, int octaves, float persistence, float lacunarity) const {
    float total = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue = 0.0f; // Used for normalizing result to [-1, 1]

    for (int i = 0; i < octaves; i++) {
        total += noise(x * frequency, y * frequency, 0.0f) * amplitude;

        maxValue += amplitude;

        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / maxValue;
}

float PerlinNoise::fade(float t) {
    // Perlin's fade function: 6t^5 - 15t^4 + 10t^3
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float PerlinNoise::lerp(float t, float a, float b) {
    return a + t * (b - a);
}

float PerlinNoise::grad(int hash, float x, float y, float z) {
    // Convert low 4 bits of hash code into 12 gradient directions
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}
