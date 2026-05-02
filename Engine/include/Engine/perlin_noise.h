#pragma once
#include <vector>
#include <cstdint>

/**
 * Perlin Noise generator for procedural terrain generation
 * Based on Ken Perlin's improved noise algorithm
 */
class PerlinNoise {
public:
    /**
     * Constructor with optional seed for reproducible random terrain
     * @param seed Random seed (default: 0)
     */
    explicit PerlinNoise(uint32_t seed = 0);

    /**
     * Get noise value at (x, y, z) coordinates
     * @return Value in range [-1, 1]
     */
    float noise(float x, float y, float z = 0.0f) const;

    /**
     * Fractal Brownian Motion - combines multiple octaves of noise
     * for more natural-looking terrain
     * @param x X coordinate
     * @param y Y coordinate
     * @param octaves Number of noise layers to combine
     * @param persistence Amplitude decrease per octave (typically 0.5)
     * @param lacunarity Frequency increase per octave (typically 2.0)
     * @return Value in range [-1, 1]
     */
    float fbm(float x, float y, int octaves, float persistence, float lacunarity) const;

private:
    std::vector<int> permutation; // Permutation table (512 elements)

    // Perlin's fade function: 6t^5 - 15t^4 + 10t^3
    static float fade(float t);

    // Linear interpolation
    static float lerp(float t, float a, float b);

    // Gradient function
    static float grad(int hash, float x, float y, float z);
};
