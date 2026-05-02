#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "vertex.h"
#include "perlin_noise.h"

/**
 * Configuration parameters for terrain generation
 */
struct TerrainConfig {
    int width = 100;              // Grid width (X-axis) - number of points
    int depth = 100;              // Grid depth (Z-axis) - number of points
    float gridSpacing = 1.0f;     // Distance between grid points in world space
    float heightScale = 10.0f;    // Multiplier for noise values to create height variation

    // Noise parameters for procedural generation
    int noiseOctaves = 4;         // Number of noise layers (more = finer details)
    float noisePersistence = 0.5f; // Amplitude decrease per octave
    float noiseLacunarity = 2.0f;  // Frequency increase per octave
    float noiseFrequency = 0.05f;  // Base noise frequency (lower = larger features)

    uint32_t seed = 12345;        // Random seed for reproducible terrain
};

/**
 * Procedural terrain generator using Perlin noise
 * Generates heightmap-based terrain meshes compatible with the existing rendering pipeline
 */
class Terrain {
public:
    /**
     * Constructor with configuration
     * Automatically generates terrain upon construction
     */
    explicit Terrain(const TerrainConfig& config = TerrainConfig());

    /**
     * Generate terrain mesh from current configuration
     */
    void generate();

    /**
     * Regenerate terrain with a new seed
     * @param newSeed New random seed
     */
    void regenerate(uint32_t newSeed);

    /**
     * Get vertices for rendering
     * @return Vector of vertices compatible with OpenGL rendering
     */
    std::vector<Vertex> getVertices() const;

    /**
     * Get height at world position (useful for collision detection)
     * @param worldX X coordinate in world space
     * @param worldZ Z coordinate in world space
     * @return Height at that position (interpolated)
     */
    float getHeightAt(float worldX, float worldZ) const;

    /**
     * Get current configuration
     */
    const TerrainConfig& getConfig() const { return config; }

    /**
     * Update configuration and regenerate terrain
     * @param newConfig New configuration parameters
     */
    void setConfig(const TerrainConfig& newConfig);

private:
    TerrainConfig config;
    std::unique_ptr<PerlinNoise> noise;
    std::vector<Vertex> vertices;
    std::vector<float> heightMap; // 2D heightmap stored as 1D array

    /**
     * Generate heightmap from Perlin noise
     */
    void generateHeightMap();

    /**
     * Generate mesh triangles from heightmap
     */
    void generateMesh();

    /**
     * Calculate normal vector for a vertex using neighboring heights
     * @param x Grid X coordinate
     * @param z Grid Z coordinate
     * @return Normalized normal vector
     */
    glm::vec3 calculateNormal(int x, int z) const;

    /**
     * Get height from heightmap at grid coordinates
     * @param x Grid X coordinate
     * @param z Grid Z coordinate
     * @return Height value
     */
    float getHeight(int x, int z) const;

    /**
     * Convert 2D grid coordinates to 1D array index
     * @param x Grid X coordinate
     * @param z Grid Z coordinate
     * @return Array index
     */
    int getIndex(int x, int z) const;
};
