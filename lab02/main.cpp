#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <cstring>

std::vector<glm::vec3> vertices;
std::vector<glm::ivec3> faces;

bool loadOBJ(const std::string &path) {
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
            glm::vec3 auxVertices;
            fscanf(file, "%f %f %f\n", &auxVertices.x, &auxVertices.y, &auxVertices.z);
            vertices.push_back(auxVertices);
        } 
        else if (strcmp(lineHeader, "f") == 0) {
            glm::ivec3 auxFaces;
            fscanf(file, "%d %d %d\n", &auxFaces.x, &auxFaces.y, &auxFaces.z);
            faces.push_back(auxFaces);
        }
    }

    fclose(file);
    return true;
}

int render(std::vector<glm::vec3> vertices, std::vector<glm::ivec3> faces) {
    GLFWwindow* window;

    if (!glfwInit()) {
        return -1;
    }

    window = glfwCreateWindow(640, 480,"Tela", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "ERROR: GLEW Initialization Failed\n";
        return -1;
    }


    return 0;
}


int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <obj file>" << std::endl;
        return 1;
    }

    if (!loadOBJ(argv[1])) {
        return 1;
    }

    render(vertices, faces);

    return 0;
}