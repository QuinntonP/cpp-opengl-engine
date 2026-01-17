#include "../include/object.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

Object::Object(const std::string& vertexPath, float scale): scale(scale)
{
    loadObject(vertexPath);
}

int Object::loadObject(const std::string& path) {
    std::cout << "Loading object from: " << path << std::endl;

    std::ifstream file(path);
    if (!file) {
        std::cerr << "Failed to open file: " << path << "\n";
        return 1;
    }

    std::string word;
    float x, y, z; 

    int vCount = 0;
    float xCount = 0.0f, yCount = 0.0f, zCount = 0.0f;
    float xAvg, yAvg, zAvg;

    while (file >> word) {
        if (word == "vertex") {
            if (file >> x >> y >> z) {
                vertices.push_back(Vertex{x * scale, y * scale, z * scale});
                vCount++;
                xCount += (x * scale);
                yCount += (y * scale);
                zCount += (z * scale);

                std::cout << scale << std::endl;
            } else {
                std::cerr << "Malformed STL vertex\n";
                return 2;
            }
        }
    }

    xAvg = xCount / vCount;
    yAvg = yCount / vCount;
    zAvg = zCount / vCount;

    center = Vertex{xAvg, yAvg, zAvg};

    std::cout << "Center is " << center.x << ":" << center.y << ":" << center.z << std::endl;

    std::cout << "Loaded vertices: " << vertices.size() << std::endl;
    return 0;
}


std::vector<Vertex> Object::getVertices(){
    return vertices;
}
