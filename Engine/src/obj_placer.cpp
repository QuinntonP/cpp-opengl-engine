#include "Engine/obj_placer.h"

#include <random>
#include <cmath>
#include <iostream>

#include "Engine/object.h"

void ObjPlacer::addObjectsRandomly(
    std::vector<Vertex>& mesh,
    const Terrain& terrain,
    const TerrainConfig& terrainConfig,
    const RandomPlacementConfig& config,
    unsigned int seed
) {
    std::mt19937 rng(seed);

    std::uniform_real_distribution<float> xDist(
        0.0f, terrainConfig.width * terrainConfig.gridSpacing
    );
    std::uniform_real_distribution<float> zDist(
        0.0f, terrainConfig.depth * terrainConfig.gridSpacing
    );
    std::uniform_real_distribution<float> scaleDist(
        1.0f - config.scaleVariation,
        1.0f + config.scaleVariation
    );

    int placed = 0;
    int attempts = 0;
    const int maxAttempts = config.count * 10;

    while (placed < config.count && attempts < maxAttempts) {
        attempts++;

        float x = xDist(rng);
        float z = zDist(rng);

        float terrainHeight = terrain.getHeightAt(x, z);

        if (terrainHeight < config.minHeight || terrainHeight > config.maxHeight) {
            continue;
        }

        float offset = 1.0f;

        float h1 = terrain.getHeightAt(x + offset, z);
        float h2 = terrain.getHeightAt(x - offset, z);
        float h3 = terrain.getHeightAt(x, z + offset);
        float h4 = terrain.getHeightAt(x, z - offset);

        float slopeX = std::abs(h1 - h2) / (2.0f * offset);
        float slopeZ = std::abs(h3 - h4) / (2.0f * offset);
        float slope = std::sqrt(slopeX * slopeX + slopeZ * slopeZ);

        if (slope < config.minSlope || slope > config.maxSlope) {
            continue;
        }

        float randomScale = config.scale * scaleDist(rng);

        float rotY = config.rotY;
        if (config.randomRotationY) {
            std::uniform_real_distribution<float> rotDist(0.0f, 360.0f);
            rotY = rotDist(rng);
        }

        Object model(config.modelPath, randomScale, config.rotX, rotY, config.rotZ);
        auto modelVerts = model.getVertices();

        if (modelVerts.empty()) continue;

        float placementHeight = terrainHeight + config.yOffset;

        if (config.placeAtLowestVertex) {
            float lowestY = modelVerts[0].y;
            for (const auto& v : modelVerts) {
                if (v.y < lowestY) lowestY = v.y;
            }
            placementHeight -= lowestY;
        }

        for (auto& v : modelVerts) {
            v.x += x;
            v.y += placementHeight;
            v.z += z;
        }

        mesh.insert(mesh.end(), modelVerts.begin(), modelVerts.end());
        placed++;
    }

    std::cout << "Randomly placed " << placed << "/" << config.count
              << " " << config.modelPath
              << " objects (attempted " << attempts << " times)\n";

    if (placed < config.count) {
        std::cout << "NOTE: Could not place all objects.\n";
    }
}