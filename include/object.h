#ifndef OBJECT_H
#define OBJECT_H

#include <string>
#include <vector>
#include "../include/vertex.h"

class Object {
public:
    Object(const std::string& vertexPath, float scale);
    std::vector<Vertex> getVertices();

private:
    std::vector<Vertex> vertices;
    float scale;
    int loadObject(const std::string& vertexPath);
    Vertex center;
};

#endif // OBJECT_H
