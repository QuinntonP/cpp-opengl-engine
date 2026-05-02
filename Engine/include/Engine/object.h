#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <vector>
#include "vertex.h"

class Object {
public:
    Object(const std::string& vertexPath, float scale, float rotX = 0.0f, float rotY = 0.0f, float rotZ = 0.0f);
    std::vector<Vertex> getVertices();

private:
    std::vector<Vertex> vertices;
    float scale;
    float rotationX, rotationY, rotationZ;  // Rotation in degrees
    int loadObject(const std::string& vertexPath);
    void applyRotation();
    void generateCylindricalUVs();
    void generateSphericalUVs();
    void generatePlanarUVs();
    Vertex center;
};

#endif // OBJECT_H
