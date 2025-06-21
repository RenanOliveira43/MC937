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
    GLuint vao, vbo[2];
    GLuint ebo;
};

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

bool interseptaTriangulo(const std::array<unsigned, 3>& triA, const std::vector<glm::vec3>& coordsA,
                       const std::array<unsigned, 3>& triB, const std::vector<glm::vec3>& coordsB) {
    glm::vec3 A0 = coordsA[triA[0]];
    glm::vec3 A1 = coordsA[triA[1]];
    glm::vec3 A2 = coordsA[triA[2]];
    
    glm::vec3 B0 = coordsB[triB[0]];
    glm::vec3 B1 = coordsB[triB[1]];
    glm::vec3 B2 = coordsB[triB[2]];

    // Compute plane equation of triangle A
    glm::vec3 N1 = glm::cross(A1 - A0, A2 - A0);
    float d1 = -glm::dot(N1, A0);
    
    // Test if all points of B lie on same side of A's plane
    float distB0 = glm::dot(N1, B0) + d1;
    float distB1 = glm::dot(N1, B1) + d1;
    float distB2 = glm::dot(N1, B2) + d1;
    
    if ((distB0 > 0 && distB1 > 0 && distB2 > 0) ||
        (distB0 < 0 && distB1 < 0 && distB2 < 0))
        return false;

    // Compute plane equation of triangle B
    glm::vec3 N2 = glm::cross(B1 - B0, B2 - B0);
    float d2 = -glm::dot(N2, B0);
    
    // Test if all points of A lie on same side of B's plane
    float distA0 = glm::dot(N2, A0) + d2;
    float distA1 = glm::dot(N2, A1) + d2;
    float distA2 = glm::dot(N2, A2) + d2;
    
    if ((distA0 > 0 && distA1 > 0 && distA2 > 0) ||
        (distA0 < 0 && distA1 < 0 && distA2 < 0))
        return false;

    // Compute intersection line
    glm::vec3 D = glm::cross(N1, N2);
    
    // Project onto axis where D is largest
    int axis;
    if (fabs(D.x) > fabs(D.y) && fabs(D.x) > fabs(D.z))
        axis = 0;
    else if (fabs(D.y) > fabs(D.z))
        axis = 1;
    else
        axis = 2;

    // Simplified projection and interval overlap test
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

bool intersect(const AABB& a, const AABB& b) {
    return (a.min_corner.x <= b.max_corner.x && a.max_corner.x >= b.min_corner.x) &&
           (a.min_corner.y <= b.max_corner.y && a.max_corner.y >= b.min_corner.y) &&
           (a.min_corner.z <= b.max_corner.z && a.max_corner.z >= b.min_corner.z);
}

void verificaColisao(const AABBNode* a, const AABBNode* b) {
    if (!a || !b) return;
    if (!intersect(a->mesh.aabb, b->mesh.aabb)) return;

    if (a->isLeaf() && b->isLeaf()) {
        for (const auto& triA : a->mesh.triangles) {
            for (const auto& triB : b->mesh.triangles) {
                if (interseptaTriangulo(triA, *a->mesh.coordinates, 
                                       triB, *b->mesh.coordinates)) {
                    std::cout << "Colisão detectada entre triângulos:\n";
                    std::cout << "Triângulo A: " << triA[0] << ", " << triA[1] << ", " << triA[2] << "\n";
                    std::cout << "Triângulo B: " << triB[0] << ", " << triB[1] << ", " << triB[2] << "\n";
                    
                    return;
                }
            }
        }
    }
    else if (a->isLeaf()) {
        verificaColisao(a, b->left_child.get());
        verificaColisao(a, b->right_child.get());
    }
    else if (b->isLeaf()) {
        verificaColisao(a->left_child.get(), b);
        verificaColisao(a->right_child.get(), b);
    }
    else {
        // Both are internal nodes - decide which to recurse into first
        float sizeA = glm::length(a->mesh.aabb.max_corner - a->mesh.aabb.min_corner);
        float sizeB = glm::length(b->mesh.aabb.max_corner - b->mesh.aabb.min_corner);
        
        if (sizeA > sizeB) {
            verificaColisao(a->left_child.get(), b);
            verificaColisao(a->right_child.get(), b);
        }
        else {
            verificaColisao(a, b->left_child.get());
            verificaColisao(a, b->right_child.get());
        }
    }
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

    auto coords1 = std::make_shared<std::vector<glm::vec3>>(obj1.vertices);
    auto coords2 = std::make_shared<std::vector<glm::vec3>>(obj2.vertices);
    
    Mesh mesh1(coords1, obj1.triangles);
    AABBTree tree1(mesh1);
    tree1.build();

    Mesh mesh2(coords2, obj2.triangles);
    AABBTree tree2(mesh2);
    tree2.build();

    // OPENGL 
    GLFWwindow* window;

    if (!glfwInit()) {
        return -1;
    }

    window = glfwCreateWindow(1920, 1080, "Modelo", nullptr, nullptr);
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
            FragColor = vec4(1.0, 0.8, 0.2, 1.0);
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


    ///////////////////////////////////////////////////////////
    // PROJECTION MATRIX SETUP
    ///////////////////////////////////////////////////////////

    // Create perspective projection matrix
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = static_cast<float>(width)/static_cast<float>(height);

    // Parameters: FOV, aspect ratio, near clip, far clip
    glm::mat4 projMat = glm::perspective(glm::radians(45.f), aspect, 1.0f, 100.0f);
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projMat");
    GLuint mvLoc = glGetUniformLocation(shaderProgram, "mvMat");

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMat));

    glm::mat4 modelMat, viewMat, mvMat;


    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.2f, 0.5f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 viewMat = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f),glm::vec3(0.0f, 1.0f, 0.0f));

        for (size_t i = 0; i < objetos.size(); ++i) {
            Objeto& obj = objetos[i];
            glm::mat4 To = glm::translate(glm::mat4(1.0f), -obj.vertices[0]); 
            glm::mat4 S  = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));     
            glm::mat4 Tb = glm::translate(glm::mat4(1.0f), obj.vertices[0]);
            glm::mat4 modelMat = Tb * S * To;

            glm::mat4 mvMat = viewMat * modelMat;

            glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

            glBindVertexArray(obj.vao);
            glDrawElements(GL_TRIANGLES, obj.triangles.size() * 3, GL_UNSIGNED_INT, 0);

        }

        verificaColisao(tree1.root.get(), tree2.root.get());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwTerminate();

    return 0;
}


