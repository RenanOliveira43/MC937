#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>

#include "aabb.cpp"

struct Objeto {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<std::array<unsigned, 3>> triangles;
    glm::mat4 modelMat;
    GLuint vao, vbo[2];
    GLuint ebo;
};

double lastX = 0.0, lastY = 0.0;
float yaw = 0.0f, pitch = 0.0f;
bool firstMouse = true;
bool mousePressed = false;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!mousePressed) return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = ypos - lastY;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.3f;
    yaw += xoffset * sensitivity;
    pitch += yoffset * sensitivity;

    pitch = glm::clamp(pitch, -89.0f, 89.0f);
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mousePressed = true;
            firstMouse = true;
        }
        else if (action == GLFW_RELEASE) {
            mousePressed = false;
        }
    }
}

bool loadOBJ(const std::string& path, Objeto& obj) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Erro ao abrir arquivo: " << path << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;

    std::unordered_map<std::string, unsigned> index_map;
    unsigned current_index = 0;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);

        if (line.rfind("v ", 0) == 0) {
            glm::vec3 pos;
            iss.ignore(2);
            iss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        }
        else if (line.rfind("vn ", 0) == 0) {
            glm::vec3 norm;
            iss.ignore(3);
            iss >> norm.x >> norm.y >> norm.z;
            temp_normals.push_back(glm::normalize(norm));
        }
        else if (line.rfind("f ", 0) == 0) {
            iss.ignore(2);
            std::string v_str[3];
            iss >> v_str[0] >> v_str[1] >> v_str[2];

            std::array<unsigned, 3> tri;

            for (int i = 0; i < 3; ++i) {
                if (index_map.find(v_str[i]) == index_map.end()) {
                    int vi = 0, ti = 0, ni = 0;
                    int matches = std::sscanf(v_str[i].c_str(), "%d/%d/%d", &vi, &ti, &ni);

                    if (matches < 1) {
                        std::cerr << "Formato inválido de face: " << v_str[i] << std::endl;
                        continue;
                    }

                    if (vi < 0) vi = (int)temp_positions.size() + vi + 1;
                    if (vi <= 0 || vi > (int)temp_positions.size()) {
                        std::cerr << "Índice de posição fora do intervalo: " << vi << std::endl;
                        continue;
                    }

                    glm::vec3 position = temp_positions[vi - 1];
                    obj.vertices.push_back(position);

                    if (matches == 3 && ni > 0 && ni <= (int)temp_normals.size()) {
                        glm::vec3 normal = temp_normals[ni - 1];
                        obj.normals.push_back(normal);
                    } else {
                        obj.normals.push_back(glm::vec3(0.0f));
                    }

                    index_map[v_str[i]] = current_index++;
                }

                tri[i] = index_map[v_str[i]];
            }

            if (obj.normals[tri[0]] == glm::vec3(0.0f) &&
                obj.normals[tri[1]] == glm::vec3(0.0f) &&
                obj.normals[tri[2]] == glm::vec3(0.0f)) {

                glm::vec3 v0 = obj.vertices[tri[0]];
                glm::vec3 v1 = obj.vertices[tri[1]];
                glm::vec3 v2 = obj.vertices[tri[2]];

                glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

                obj.normals[tri[0]] = normal;
                obj.normals[tri[1]] = normal;
                obj.normals[tri[2]] = normal;
            }

            obj.triangles.push_back(tri);
        }
    }

    return true;
}

bool interceptaTriangulo(const std::array<unsigned, 3>& triA, const std::vector<glm::vec3>& coordsA, const glm::mat4& transformA, const std::array<unsigned, 3>& triB, const std::vector<glm::vec3>& coordsB, const glm::mat4& transformB) {
    glm::vec3 A0 = glm::vec3(transformA * glm::vec4(coordsA[triA[0]], 1.0f));
    glm::vec3 A1 = glm::vec3(transformA * glm::vec4(coordsA[triA[1]], 1.0f));
    glm::vec3 A2 = glm::vec3(transformA * glm::vec4(coordsA[triA[2]], 1.0f));

    glm::vec3 B0 = glm::vec3(transformB * glm::vec4(coordsB[triB[0]], 1.0f));
    glm::vec3 B1 = glm::vec3(transformB * glm::vec4(coordsB[triB[1]], 1.0f));
    glm::vec3 B2 = glm::vec3(transformB * glm::vec4(coordsB[triB[2]], 1.0f));

    glm::vec3 N1 = glm::cross(A1 - A0, A2 - A0);
    float d1 = -glm::dot(N1, A0);

    float distB0 = glm::dot(N1, B0) + d1;
    float distB1 = glm::dot(N1, B1) + d1;
    float distB2 = glm::dot(N1, B2) + d1;

    if ((distB0 > 0 && distB1 > 0 && distB2 > 0) ||
        (distB0 < 0 && distB1 < 0 && distB2 < 0))
        return false;

    glm::vec3 N2 = glm::cross(B1 - B0, B2 - B0);
    float d2 = -glm::dot(N2, B0);

    float distA0 = glm::dot(N2, A0) + d2;
    float distA1 = glm::dot(N2, A1) + d2;
    float distA2 = glm::dot(N2, A2) + d2;

    if ((distA0 > 0 && distA1 > 0 && distA2 > 0) || (distA0 < 0 && distA1 < 0 && distA2 < 0)){
        return false;
    }

    glm::vec3 D = glm::cross(N1, N2);

    int axis;
    if (fabs(D.x) > fabs(D.y) && fabs(D.x) > fabs(D.z)){
        axis = 0;
    }
    else if (fabs(D.y) > fabs(D.z)) {
        axis = 1;
    }
    else {
        axis = 2;
    }

    auto project = [axis](const glm::vec3& v) {
        return (axis == 0) ? v.x : (axis == 1) ? v.y : v.z;
    };

    float a0 = project(A0), a1 = project(A1), a2 = project(A2);
    float b0 = project(B0), b1 = project(B1), b2 = project(B2);

    float minA = std::min({a0, a1, a2});
    float maxA = std::max({a0, a1, a2});
    float minB = std::min({b0, b1, b2});
    float maxB = std::max({b0, b1, b2});

    return maxA >= minB && maxB >= minA;
}

bool verificaColisao(AABBNode* nodeA, AABBNode* nodeB, const glm::mat4& transformA, const glm::mat4& transformB) {
    if (!nodeA || !nodeB) return false;

    if (!nodeA->transformed_aabb.intersects(nodeB->transformed_aabb))
        return false;

    if (nodeA->isLeaf() && nodeB->isLeaf()) {
        for (const auto& triA : nodeA->mesh.triangles) {
            for (const auto& triB : nodeB->mesh.triangles) {
                if (interceptaTriangulo(triA, *nodeA->mesh.coordinates, transformA, triB, *nodeB->mesh.coordinates, transformB)) {
                    std::cout << "Colisão detectada entre triângulos!" << std::endl;
                    std::cout << "Triângulo A: " << triA[0] << ", " << triA[1] << ", " << triA[2] << std::endl;
                    std::cout << "Triângulo B: " << triB[0] << ", " << triB[1] << ", " << triB[2] << std::endl;
                    return true;
                }
            }
        }
        return false;
    }

    if (!nodeA->isLeaf() && !nodeB->isLeaf()) {
        return verificaColisao(nodeA->left_child.get(), nodeB->left_child.get(), transformA, transformB) ||
               verificaColisao(nodeA->left_child.get(), nodeB->right_child.get(), transformA, transformB) ||
               verificaColisao(nodeA->right_child.get(), nodeB->left_child.get(), transformA, transformB) ||
               verificaColisao(nodeA->right_child.get(), nodeB->right_child.get(), transformA, transformB);
    } 
    else if (!nodeA->isLeaf()) {
        return verificaColisao(nodeA->left_child.get(), nodeB, transformA, transformB) ||
               verificaColisao(nodeA->right_child.get(), nodeB, transformA, transformB);
    } 
    else { 
        return verificaColisao(nodeA, nodeB->left_child.get(), transformA, transformB) ||
               verificaColisao(nodeA, nodeB->right_child.get(), transformA, transformB);
    }
}

glm::vec3 calcularTamanho(const std::vector<glm::vec3>& vertices) {
    glm::vec3 min = vertices[0];
    glm::vec3 max = vertices[0];

    for (const auto& v : vertices) {
        min = glm::min(min, v);
        max = glm::max(max, v);
    }

    return max - min;
}

float comprimentoMaximo(const glm::vec3& tamanho) {
    return std::max({tamanho.x, tamanho.y, tamanho.z});
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <modelo.obj>" << std::endl;
        return 1;
    }

    std::vector<Objeto> objetos;

    Objeto obj1, obj2;
    if (!loadOBJ(argv[1], obj1) || !loadOBJ(argv[2], obj2)) {
        return 1;
    }

    objetos.push_back(obj1);
    objetos.push_back(obj2);

    std::cout << "Vertices: " << obj1.vertices.size() << " | Triângulos: " << obj1.triangles.size() << "\n";
    std::cout << "Vertices: " << obj2.vertices.size() << " | Triângulos: " << obj2.triangles.size() << "\n";

    // OPENGL 
    GLFWwindow* window;

    if (!glfwInit()) {
        return -1;
    }

    window = glfwCreateWindow(800, 600, "Modelo", nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK) {
        std::cerr << "ERROR: GLEW Initialization Failed\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    for (auto& obj : objetos) {
        glGenVertexArrays(1, &obj.vao);
        glBindVertexArray(obj.vao);

        glGenBuffers(2, obj.vbo);

        // VBO de posições
        glBindBuffer(GL_ARRAY_BUFFER, obj.vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, obj.vertices.size() * sizeof(glm::vec3), obj.vertices.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // VBO de normais
        glBindBuffer(GL_ARRAY_BUFFER, obj.vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, obj.normals.size() * sizeof(glm::vec3), obj.normals.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        // EBO de índices (triângulos)
        glGenBuffers(1, &obj.ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj.ebo);
        
        // Criar vetor plano de índices (unsigned int)
        std::vector<unsigned int> indices;
        indices.reserve(obj.triangles.size() * 3);
        for (const auto& tri : obj.triangles) {
            indices.push_back(tri[0]);
            indices.push_back(tri[1]);
            indices.push_back(tri[2]);
        }

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }

    const char* vertexShader = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        uniform mat4 mvMat;
        uniform mat4 projMat;
        void main() {
            gl_Position = projMat * mvMat * vec4(position, 1.0);
        }
        )";

    const char* fragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        }
        )";


    // Compile and link shaders
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShader, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShader, nullptr);
    glCompileShader(fs);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fs);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    // Create perspective projection matrix
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = static_cast<float>(width)/static_cast<float>(height);

    // Parameters: FOV, aspect ratio, near clip, far clip
    glm::mat4 projMat = glm::perspective(glm::radians(45.f), aspect, 1.0f, 100.0f);
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projMat");
    GLuint mvLoc = glGetUniformLocation(shaderProgram, "mvMat");

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMat));

    glm::mat4 viewMat, mvMat;

    glm::vec3 tamanho1 = calcularTamanho(obj1.vertices);
    glm::vec3 tamanho2 = calcularTamanho(obj2.vertices);

    float escala1 = 1.0f / comprimentoMaximo(tamanho1);
    float escala2 = 1.0f / comprimentoMaximo(tamanho2);

    float modelAngle = 0.0f; 

    std::vector<AABBTree> trees;
    for (auto& obj : objetos) {
        auto mesh = std::make_shared<std::vector<glm::vec3>>(obj.vertices);
        trees.emplace_back(Mesh(mesh, obj.triangles));
        trees.back().build();
    }

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 viewMat = glm::mat4(1.0f);
        viewMat = glm::translate(viewMat, glm::vec3(0.0f, 0.0f, -10.0f));
        viewMat = glm::rotate(viewMat, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
        viewMat = glm::rotate(viewMat, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));

        for (size_t i = 0; i < objetos.size(); ++i) {
            Objeto& obj = objetos[i];

            float escalaAtual = (i == 0) ? escala1 : escala2;

            glm::mat4 To = glm::translate(glm::mat4(1.0f), -obj.vertices[0]);
            glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(escalaAtual));
            glm::mat4 R = glm::mat4(1.0f);
            glm::mat4 Tb = glm::translate(glm::mat4(1.0f), obj.vertices[0]);
            glm::mat4 translacaoLateral = glm::mat4(1.0f);
            
            if (i == 1) {
                translacaoLateral = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f));
                R = glm::rotate(glm::mat4(1.0f), glm::radians(modelAngle), glm::vec3(0.0f, 1.0f, 0.0f));
            }

            obj.modelMat = translacaoLateral * Tb * R * S * To;

            trees[i].updateTransform(obj.modelMat);

            glm::mat4 mvMat = viewMat * obj.modelMat;
            glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));
            glBindVertexArray(obj.vao);
            glDrawElements(GL_TRIANGLES, obj.triangles.size() * 3, GL_UNSIGNED_INT, 0);
        }

        if (trees.size() >= 2) {
            verificaColisao(trees[0].getRoot(), trees[1].getRoot(), objetos[0].modelMat, objetos[1].modelMat);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

        modelAngle = modelAngle <= 360 ? modelAngle + 0.5f : 0;
    }

    glfwTerminate();

    return 0;
}