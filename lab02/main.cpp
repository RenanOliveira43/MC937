#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <cstring>


struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

std::vector<Vertex> vertices;
std::vector<unsigned int> indices;

bool loadOBJ(const std::string& path) {
    FILE* file = fopen(path.c_str(), "r");
    if (file == NULL) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return false;
    }

    while (true) {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF) {
            break;
        }

        if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            vertices.push_back({vertex, glm::vec3(0.0f)});
        } 
        else if (strcmp(lineHeader, "f") == 0) {
            unsigned int vertexIndex[3];
            unsigned int normalIndex[3];
            fscanf(file, "%d//%d//%d//%d//%d//%d\n", &vertexIndex[0], &normalIndex[0], &vertexIndex[1], &normalIndex[1], &vertexIndex[2], &normalIndex[2]);

            indices.push_back(vertexIndex[0] - 1);
            indices.push_back(vertexIndex[1] - 1);
            indices.push_back(vertexIndex[2] - 1);
        }
    }
}