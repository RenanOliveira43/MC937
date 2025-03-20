#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

struct Vertex {
    glm::vec3 position;
};

std::vector<Vertex> vertices;
std::vector<unsigned int> indices;

bool loadOBJ(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << path << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_positions;
    std::vector<unsigned int> temp_indices;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream s(line);
        std::string type;
        s >> type;

        if (type == "v") {
            glm::vec3 position;
            s >> position.x >> position.y >> position.z;
            temp_positions.push_back(position);
        } else if (type == "f") {
            unsigned int vIndex[3];
            for (int i = 0; i < 3; i++) {
                s >> vIndex[i];
                temp_indices.push_back(vIndex[i] - 1);
            }
        }
    }

    for (size_t i = 0; i < temp_indices.size(); i++) {
        vertices.push_back({temp_positions[temp_indices[i]]});
    }
    
    indices = temp_indices;
    return true;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBegin(GL_TRIANGLES);
    for (const auto& vertex : vertices) {
        glVertex3f(vertex.position.x, vertex.position.y, vertex.position.z);
    }
    glEnd();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_obj>" << std::endl;
        return -1;
    }

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "OBJ Loader", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    if (!loadOBJ(argv[1])) {
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}