#include "Engine/obj_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<Vertex> ObjLoader::load(const std::string& filepath, float scale) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << filepath << std::endl;
        return {};
    }

    std::vector<TempVertex> positions;
    std::vector<TempNormal> normals;
    std::vector<TempTexCoord> texCoords;
    std::vector<Vertex> vertices;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            // Vertex position
            TempVertex v;
            iss >> v.x >> v.y >> v.z;
            v.x *= scale;
            v.y *= scale;
            v.z *= scale;
            positions.push_back(v);
        }
        else if (prefix == "vn") {
            // Vertex normal
            TempNormal n;
            iss >> n.nx >> n.ny >> n.nz;
            normals.push_back(n);
        }
        else if (prefix == "vt") {
            // Texture coordinate
            TempTexCoord t;
            iss >> t.u >> t.v;
            texCoords.push_back(t);
        }
        else if (prefix == "f") {
            // Face (can be triangle or quad, we'll triangulate quads)
            std::vector<FaceIndex> faceIndices;
            std::string vertexStr;

            while (iss >> vertexStr) {
                FaceIndex face = {-1, -1, -1};

                // Parse format: v, v/vt, v/vt/vn, or v//vn
                size_t firstSlash = vertexStr.find('/');
                if (firstSlash == std::string::npos) {
                    // Format: v
                    face.vertexIdx = std::stoi(vertexStr) - 1;
                } else {
                    face.vertexIdx = std::stoi(vertexStr.substr(0, firstSlash)) - 1;

                    size_t secondSlash = vertexStr.find('/', firstSlash + 1);
                    if (secondSlash != std::string::npos) {
                        // Format: v/vt/vn or v//vn
                        if (secondSlash != firstSlash + 1) {
                            // Has texture coordinate
                            face.texCoordIdx = std::stoi(vertexStr.substr(firstSlash + 1, secondSlash - firstSlash - 1)) - 1;
                        }
                        face.normalIdx = std::stoi(vertexStr.substr(secondSlash + 1)) - 1;
                    } else {
                        // Format: v/vt
                        face.texCoordIdx = std::stoi(vertexStr.substr(firstSlash + 1)) - 1;
                    }
                }
                faceIndices.push_back(face);
            }

            // Triangulate if quad (assumes convex faces)
            if (faceIndices.size() >= 3) {
                // First triangle
                for (int i = 0; i < 3; i++) {
                    Vertex vertex = {};

                    // Position (required)
                    if (faceIndices[i].vertexIdx >= 0 && faceIndices[i].vertexIdx < positions.size()) {
                        vertex.x = positions[faceIndices[i].vertexIdx].x;
                        vertex.y = positions[faceIndices[i].vertexIdx].y;
                        vertex.z = positions[faceIndices[i].vertexIdx].z;
                    }

                    // Normal (optional)
                    if (faceIndices[i].normalIdx >= 0 && faceIndices[i].normalIdx < normals.size()) {
                        vertex.nx = normals[faceIndices[i].normalIdx].nx;
                        vertex.ny = normals[faceIndices[i].normalIdx].ny;
                        vertex.nz = normals[faceIndices[i].normalIdx].nz;
                    }

                    // Texture coordinate (optional)
                    if (faceIndices[i].texCoordIdx >= 0 && faceIndices[i].texCoordIdx < texCoords.size()) {
                        vertex.u = texCoords[faceIndices[i].texCoordIdx].u;
                        vertex.v = texCoords[faceIndices[i].texCoordIdx].v;
                    }

                    vertices.push_back(vertex);
                }

                // If quad, add second triangle (0, 2, 3)
                if (faceIndices.size() == 4) {
                    int indices[] = {0, 2, 3};
                    for (int idx : indices) {
                        Vertex vertex = {};

                        if (faceIndices[idx].vertexIdx >= 0 && faceIndices[idx].vertexIdx < positions.size()) {
                            vertex.x = positions[faceIndices[idx].vertexIdx].x;
                            vertex.y = positions[faceIndices[idx].vertexIdx].y;
                            vertex.z = positions[faceIndices[idx].vertexIdx].z;
                        }

                        if (faceIndices[idx].normalIdx >= 0 && faceIndices[idx].normalIdx < normals.size()) {
                            vertex.nx = normals[faceIndices[idx].normalIdx].nx;
                            vertex.ny = normals[faceIndices[idx].normalIdx].ny;
                            vertex.nz = normals[faceIndices[idx].normalIdx].nz;
                        }

                        if (faceIndices[idx].texCoordIdx >= 0 && faceIndices[idx].texCoordIdx < texCoords.size()) {
                            vertex.u = texCoords[faceIndices[idx].texCoordIdx].u;
                            vertex.v = texCoords[faceIndices[idx].texCoordIdx].v;
                        }

                        vertices.push_back(vertex);
                    }
                }
            }
        }
    }

    file.close();

    std::cout << "Loaded OBJ: " << filepath << " - " << vertices.size() << " vertices" << std::endl;
    return vertices;
}
