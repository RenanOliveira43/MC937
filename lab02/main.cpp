#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <fstream>
#include <sstream>

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

GLuint createShaderProgram() {
    const char* vertexShader = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        
        uniform mat4 model;
    
        void main() {
            gl_Position = model * vec4(aPos, 1.0);
        })";
    

    const char* fragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 1.0, 1.0, 1.0); // White color
        })";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShader, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShader, nullptr);
    glCompileShader(fs);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, fs);
    glAttachShader(shaderProgram, vs);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    return shaderProgram;
}

glm::vec3 getCentroModelo() {
    glm::vec3 min = vertices[0].position;
    glm::vec3 max = vertices[0].position;

    for (const auto& vertex : vertices) {
        min = glm::min(min, vertex.position);
        max = glm::max(max, vertex.position);
    }

    return (min + max); 
}


int render() {
    GLFWwindow* window;

    if (!glfwInit()) {
        return -1;
    }

    window = glfwCreateWindow(640, 480,"OBJ Viewer", nullptr, nullptr);
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

    GLuint shaderProgram = createShaderProgram();

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    
        glUseProgram(shaderProgram);
    
        // matriz de escala
        glm::vec3 modelCenter = getCentroModelo();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, -modelCenter); 
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        glBindVertexArray(0);
    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

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

    return render();
}
