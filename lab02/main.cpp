#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <cstring>
#include <glm/gtc/matrix_transform.hpp>


struct Vertex {
    glm::vec3 position;
};

std::vector<Vertex> vertices;
std::vector<unsigned int> indices;

bool loadOBJ(const std::string& path) {
    FILE* file = std::fopen(path.c_str(), "r");
    if (!file) {
        std::cerr << "Failed to open OBJ file: " << path << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_positions; 
    std::vector<unsigned int> temp_indices; 

    char line[128];
    while (std::fgets(line, sizeof(line), file)) {
        if (line[0] == 'v') { 
            glm::vec3 vertex;
            std::sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            
            temp_positions.push_back(vertex);
        } 
        else if (line[0] == 'f') {
            unsigned int vIndex[3];
            std::sscanf(line, "f %d %d %d", &vIndex[0], &vIndex[1], &vIndex[2]);
            
            temp_indices.push_back(vIndex[0] - 1);
            temp_indices.push_back(vIndex[1] - 1);
            temp_indices.push_back(vIndex[2] - 1);
        }
    }
    
    vertices.clear();
    for (unsigned int index : temp_indices) {
        if (index < temp_positions.size()) {
            Vertex vertex;
            vertex.position = temp_positions[index];
            vertices.push_back(vertex);
        }
    }
    
    std::fclose(file);

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

    return (min + max) / 2.0f; 
}

int render() {
    GLFWwindow* window;

    if (!glfwInit()) {
        return -1;
    }

    window = glfwCreateWindow(800, 600,"Modelo", nullptr, nullptr);
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

    glm::vec3 modelCenter = getCentroModelo();
    glm::mat4 model = glm::mat4(1.0f);

    float angle = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    
        glUseProgram(shaderProgram);
    
        angle += 0.01f;

        model = glm::translate(glm::mat4(1.0f), -modelCenter);
        model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
        
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
        glBindVertexArray(0);
    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Invalid input" << std::endl;
        return 1;
    }

    if (!loadOBJ(argv[1])) {
        return 1;
    }

    return render();
}
