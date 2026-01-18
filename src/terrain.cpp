#include "../include/terrain.h"
#include <cmath>
#include <iostream>

Terrain::Terrain(const TerrainConfig& config)
    : config(config), noise(std::make_unique<PerlinNoise>(config.seed)) {
    generate();
}

void Terrain::generate() {
    generateHeightMap();
    generateMesh();
}

void Terrain::regenerate(uint32_t newSeed) {
    config.seed = newSeed;
    noise = std::make_unique<PerlinNoise>(newSeed);
    generate();
}

std::vector<Vertex> Terrain::getVertices() const {
    return vertices;
}

void Terrain::setConfig(const TerrainConfig& newConfig) {
    config = newConfig;
    noise = std::make_unique<PerlinNoise>(config.seed);
    generate();
}

void Terrain::generateHeightMap() {
    heightMap.resize(config.width * config.depth);

    for (int z = 0; z < config.depth; z++) {
        for (int x = 0; x < config.width; x++) {
            // Convert grid coordinates to world coordinates
            float worldX = x * config.gridSpacing;
            float worldZ = z * config.gridSpacing;

            // Sample Perlin noise with fractal Brownian motion
            float height = noise->fbm(
                worldX * config.noiseFrequency,
                worldZ * config.noiseFrequency,
                config.noiseOctaves,
                config.noisePersistence,
                config.noiseLacunarity
            );

            // Apply height scale and store in heightmap
            heightMap[getIndex(x, z)] = height * config.heightScale;
        }
    }
}

void Terrain::generateMesh() {
    vertices.clear();

    // Reserve space for all vertices (each grid cell = 2 triangles = 6 vertices)
    vertices.reserve((config.width - 1) * (config.depth - 1) * 6);

    // Generate triangles for each grid cell
    for (int z = 0; z < config.depth - 1; z++) {
        for (int x = 0; x < config.width - 1; x++) {
            // Get corner positions in world space
            float x0 = x * config.gridSpacing;
            float x1 = (x + 1) * config.gridSpacing;
            float z0 = z * config.gridSpacing;
            float z1 = (z + 1) * config.gridSpacing;

            // Get heights at corners
            float h00 = getHeight(x, z);
            float h10 = getHeight(x + 1, z);
            float h01 = getHeight(x, z + 1);
            float h11 = getHeight(x + 1, z + 1);

            // Calculate normals at corners
            glm::vec3 n00 = calculateNormal(x, z);
            glm::vec3 n10 = calculateNormal(x + 1, z);
            glm::vec3 n01 = calculateNormal(x, z + 1);
            glm::vec3 n11 = calculateNormal(x + 1, z + 1);

            // Calculate UV coordinates (normalized to [0, 1])
            float u0 = static_cast<float>(x) / (config.width - 1);
            float u1 = static_cast<float>(x + 1) / (config.width - 1);
            float v0 = static_cast<float>(z) / (config.depth - 1);
            float v1 = static_cast<float>(z + 1) / (config.depth - 1);

            // First triangle: (x, z) -> (x+1, z) -> (x, z+1)
            vertices.push_back({x0, h00, z0, n00.x, n00.y, n00.z, u0, v0});
            vertices.push_back({x1, h10, z0, n10.x, n10.y, n10.z, u1, v0});
            vertices.push_back({x0, h01, z1, n01.x, n01.y, n01.z, u0, v1});

            // Second triangle: (x+1, z) -> (x+1, z+1) -> (x, z+1)
            vertices.push_back({x1, h10, z0, n10.x, n10.y, n10.z, u1, v0});
            vertices.push_back({x1, h11, z1, n11.x, n11.y, n11.z, u1, v1});
            vertices.push_back({x0, h01, z1, n01.x, n01.y, n01.z, u0, v1});
        }
    }

    std::cout << "Terrain generated: " << vertices.size() << " vertices" << std::endl;
}

glm::vec3 Terrain::calculateNormal(int x, int z) const {
    // Use central differences to calculate normal from neighboring heights
    // This gives smooth normals across the terrain

    // Get heights of neighbors (with boundary checking)
    float hL = (x > 0) ? getHeight(x - 1, z) : getHeight(x, z);
    float hR = (x < config.width - 1) ? getHeight(x + 1, z) : getHeight(x, z);
    float hD = (z > 0) ? getHeight(x, z - 1) : getHeight(x, z);
    float hU = (z < config.depth - 1) ? getHeight(x, z + 1) : getHeight(x, z);

    // Calculate tangent vectors
    // Tangent along X-axis
    glm::vec3 tangentX(2.0f * config.gridSpacing, hR - hL, 0.0f);
    // Tangent along Z-axis
    glm::vec3 tangentZ(0.0f, hU - hD, 2.0f * config.gridSpacing);

    // Normal is cross product of tangents
    glm::vec3 normal = glm::cross(tangentZ, tangentX);
    return glm::normalize(normal);
}

float Terrain::getHeight(int x, int z) const {
    // Clamp coordinates to valid range
    if (x < 0) x = 0;
    if (x >= config.width) x = config.width - 1;
    if (z < 0) z = 0;
    if (z >= config.depth) z = config.depth - 1;

    return heightMap[getIndex(x, z)];
}

float Terrain::getHeightAt(float worldX, float worldZ) const {
    // Convert world coordinates to grid coordinates
    float gridX = worldX / config.gridSpacing;
    float gridZ = worldZ / config.gridSpacing;

    // Get integer coordinates of grid cell
    int x0 = static_cast<int>(std::floor(gridX));
    int z0 = static_cast<int>(std::floor(gridZ));
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    // Check bounds
    if (x0 < 0 || x1 >= config.width || z0 < 0 || z1 >= config.depth) {
        return 0.0f;
    }

    // Get fractional part for interpolation
    float fx = gridX - x0;
    float fz = gridZ - z0;

    // Bilinear interpolation
    float h00 = getHeight(x0, z0);
    float h10 = getHeight(x1, z0);
    float h01 = getHeight(x0, z1);
    float h11 = getHeight(x1, z1);

    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;

    return h0 * (1.0f - fz) + h1 * fz;
}

int Terrain::getIndex(int x, int z) const {
    return z * config.width + x;
}
