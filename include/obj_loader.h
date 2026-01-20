#pragma once
#include <vector>
#include <string>
#include "vertex.h"

/**
 * OBJ file loader
 * Loads Wavefront .obj files and converts them to our Vertex format
 */
class ObjLoader {
public:
    /**
     * Load an OBJ file
     * @param filepath Path to the .obj file
     * @param scale Uniform scale to apply to all vertices
     * @return Vector of vertices
     */
    static std::vector<Vertex> load(const std::string& filepath, float scale = 1.0f);

private:
    struct TempVertex {
        float x, y, z;
    };

    struct TempNormal {
        float nx, ny, nz;
    };

    struct TempTexCoord {
        float u, v;
    };

    struct FaceIndex {
        int vertexIdx;
        int texCoordIdx;
        int normalIdx;
    };
};
