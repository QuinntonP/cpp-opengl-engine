#ifndef OBJ_PLACER_H
#define OBJ_PLACER_H

#include <vector>
#include <string>

#include "../include/vertex.h"
#include "../include/terrain.h"

class ObjPlacer {
public:
    struct RandomPlacementConfig {
        std::string modelPath;
        int count;
        float scale;
        float scaleVariation;
        float yOffset;
        bool placeAtLowestVertex;
        float minSlope;
        float maxSlope;
        float minHeight;
        float maxHeight;
        float rotX = 0.0f;
        float rotY = 0.0f;
        float rotZ = 0.0f;
        bool randomRotationY = false;
    };

    static void addObjectsRandomly(
        std::vector<Vertex>& mesh,
        const Terrain& terrain,
        const TerrainConfig& terrainConfig,
        const RandomPlacementConfig& config,
        unsigned int seed = 0
    );
};

#endif