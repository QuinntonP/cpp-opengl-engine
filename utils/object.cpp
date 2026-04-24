#include "../include/object.h"
#include "../include/obj_loader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cmath>

Object::Object(const std::string& vertexPath, float scale, float rotX, float rotY, float rotZ)
    : scale(scale), rotationX(rotX), rotationY(rotY), rotationZ(rotZ)
{
    loadObject(vertexPath);
    if (rotX != 0.0f || rotY != 0.0f || rotZ != 0.0f) {
        applyRotation();
    }
}

int Object::loadObject(const std::string& path) {
    std::cout << "Loading object from: " << path << std::endl;

    // Check file extension
    std::string extension = path.substr(path.find_last_of('.'));
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    if (extension == ".obj") {
        // Load OBJ file
        vertices = ObjLoader::load(path, scale);
        if (vertices.empty()) {
            std::cerr << "Failed to load OBJ file: " << path << "\n";
            return 1;
        }
    }
    else if (extension == ".stl") {
        // Load STL file (original implementation)
        std::ifstream file(path);
        if (!file) {
            std::cerr << "Failed to open file: " << path << "\n";
            return 1;
        }

        std::string word;
        float x, y, z;

        while (file >> word) {
            if (word == "vertex") {
                if (file >> x >> y >> z) {
                    Vertex v;
                    v.x = x * scale;
                    v.y = y * scale;
                    v.z = z * scale;
                    v.nx = 0.0f; v.ny = 0.0f; v.nz = 0.0f; // Will be computed later
                    v.u = 0.0f; v.v = 0.0f; // Will be computed later
                    vertices.push_back(v);
                } else {
                    std::cerr << "Malformed STL vertex\n";
                    return 2;
                }
            }
        }
        file.close();

        // Generate UV coordinates using spherical projection (better for trees)
        generateSphericalUVs();
    }
    else {
        std::cerr << "Unsupported file format: " << extension << "\n";
        std::cerr << "Supported formats: .stl, .obj\n";
        return 1;
    }

    // Calculate center
    if (!vertices.empty()) {
        float xCount = 0.0f, yCount = 0.0f, zCount = 0.0f;
        for (const auto& v : vertices) {
            xCount += v.x;
            yCount += v.y;
            zCount += v.z;
        }

        float xAvg = xCount / vertices.size();
        float yAvg = yCount / vertices.size();
        float zAvg = zCount / vertices.size();

        center = Vertex{xAvg, yAvg, zAvg};

        std::cout << "Center is " << center.x << ":" << center.y << ":" << center.z << std::endl;
        std::cout << "Loaded vertices: " << vertices.size() << std::endl;
    }

    return 0;
}


std::vector<Vertex> Object::getVertices(){
    return vertices;
}

void Object::applyRotation() {
    // Convert degrees to radians
    const float PI = 3.14159265358979323846f;
    float radX = rotationX * PI / 180.0f;
    float radY = rotationY * PI / 180.0f;
    float radZ = rotationZ * PI / 180.0f;

    // Precompute sin and cos values
    float cosX = std::cos(radX), sinX = std::sin(radX);
    float cosY = std::cos(radY), sinY = std::sin(radY);
    float cosZ = std::cos(radZ), sinZ = std::sin(radZ);

    for (auto& v : vertices) {
        float x = v.x, y = v.y, z = v.z;
        float nx = v.nx, ny = v.ny, nz = v.nz;

        // Rotate around X axis
        if (rotationX != 0.0f) {
            float yNew = y * cosX - z * sinX;
            float zNew = y * sinX + z * cosX;
            y = yNew;
            z = zNew;

            float nyNew = ny * cosX - nz * sinX;
            float nzNew = ny * sinX + nz * cosX;
            ny = nyNew;
            nz = nzNew;
        }

        // Rotate around Y axis
        if (rotationY != 0.0f) {
            float xNew = x * cosY + z * sinY;
            float zNew = -x * sinY + z * cosY;
            x = xNew;
            z = zNew;

            float nxNew = nx * cosY + nz * sinY;
            float nzNew = -nx * sinY + nz * cosY;
            nx = nxNew;
            nz = nzNew;
        }

        // Rotate around Z axis
        if (rotationZ != 0.0f) {
            float xNew = x * cosZ - y * sinZ;
            float yNew = x * sinZ + y * cosZ;
            x = xNew;
            y = yNew;

            float nxNew = nx * cosZ - ny * sinZ;
            float nyNew = nx * sinZ + ny * cosZ;
            nx = nxNew;
            ny = nyNew;
        }

        v.x = x; v.y = y; v.z = z;
        v.nx = nx; v.ny = ny; v.nz = nz;
    }
}

void Object::generateCylindricalUVs() {
    if (vertices.empty()) return;

    // Find bounding box to normalize coordinates
    float minY = vertices[0].y, maxY = vertices[0].y;
    float minX = vertices[0].x, maxX = vertices[0].x;
    float minZ = vertices[0].z, maxZ = vertices[0].z;

    for (const auto& v : vertices) {
        if (v.y < minY) minY = v.y;
        if (v.y > maxY) maxY = v.y;
        if (v.x < minX) minX = v.x;
        if (v.x > maxX) maxX = v.x;
        if (v.z < minZ) minZ = v.z;
        if (v.z > maxZ) maxZ = v.z;
    }

    float centerX = (minX + maxX) / 2.0f;
    float centerZ = (minZ + maxZ) / 2.0f;
    float height = maxY - minY;

    // Generate cylindrical UV coordinates
    const float PI = 3.14159265358979323846f;
    for (auto& v : vertices) {
        // V coordinate: normalized height (0 at bottom, 1 at top)
        v.v = height > 0 ? (v.y - minY) / height : 0.5f;

        // U coordinate: angle around Y axis
        float dx = v.x - centerX;
        float dz = v.z - centerZ;
        float angle = std::atan2(dz, dx);
        v.u = (angle + PI) / (2.0f * PI); // Map from [-π, π] to [0, 1]
    }

    std::cout << "Generated cylindrical UVs for " << vertices.size() << " vertices" << std::endl;
}

void Object::generateSphericalUVs() {
    if (vertices.empty()) return;

    // Find bounding box to get center
    float minX = vertices[0].x, maxX = vertices[0].x;
    float minY = vertices[0].y, maxY = vertices[0].y;
    float minZ = vertices[0].z, maxZ = vertices[0].z;

    for (const auto& v : vertices) {
        if (v.x < minX) minX = v.x;
        if (v.x > maxX) maxX = v.x;
        if (v.y < minY) minY = v.y;
        if (v.y > maxY) maxY = v.y;
        if (v.z < minZ) minZ = v.z;
        if (v.z > maxZ) maxZ = v.z;
    }

    float centerX = (minX + maxX) / 2.0f;
    float centerY = (minY + maxY) / 2.0f;
    float centerZ = (minZ + maxZ) / 2.0f;

    // Generate spherical UV coordinates
    const float PI = 3.14159265358979323846f;
    for (auto& v : vertices) {
        // Calculate direction from center
        float dx = v.x - centerX;
        float dy = v.y - centerY;
        float dz = v.z - centerZ;

        float len = std::sqrt(dx*dx + dy*dy + dz*dz);
        if (len < 0.0001f) {
            v.u = 0.5f;
            v.v = 0.5f;
            continue;
        }

        // Normalize direction
        dx /= len;
        dy /= len;
        dz /= len;

        // Spherical coordinates
        // U: longitude (horizontal angle around Y axis)
        v.u = 0.5f + std::atan2(dz, dx) / (2.0f * PI);

        // V: latitude (vertical angle from equator)
        v.v = 0.5f - std::asin(dy) / PI;
    }

    std::cout << "Generated spherical UVs for " << vertices.size() << " vertices" << std::endl;
}

void Object::generatePlanarUVs() {
    if (vertices.empty()) return;

    // Find bounding box
    float minX = vertices[0].x, maxX = vertices[0].x;
    float minZ = vertices[0].z, maxZ = vertices[0].z;

    for (const auto& v : vertices) {
        if (v.x < minX) minX = v.x;
        if (v.x > maxX) maxX = v.x;
        if (v.z < minZ) minZ = v.z;
        if (v.z > maxZ) maxZ = v.z;
    }

    float width = maxX - minX;
    float depth = maxZ - minZ;

    // Generate planar UV coordinates (project from top)
    for (auto& v : vertices) {
        v.u = width > 0 ? (v.x - minX) / width : 0.5f;
        v.v = depth > 0 ? (v.z - minZ) / depth : 0.5f;
    }

    std::cout << "Generated planar UVs for " << vertices.size() << " vertices" << std::endl;
}
