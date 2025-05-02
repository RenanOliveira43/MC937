#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <cstdio>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;

bool loadOBJ(const std::string& path) {
    FILE* file = std::fopen(path.c_str(), "r");
    if (!file) {
        std::cerr << "Erro ao abrir arquivo: " << path << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_positions;
    char line[128];

    while (fgets(line, sizeof(line), file)) {
        if (line[0] == 'v' && line[1] == ' ') {
            glm::vec3 pos;
            sscanf(line, "v %f %f %f", &pos.x, &pos.y, &pos.z);
            temp_positions.push_back(pos);
        } else if (line[0] == 'f') {
            int vi[3];
            sscanf(line, "f %d %d %d", &vi[0], &vi[1], &vi[2]);

            glm::vec3 v0 = temp_positions[vi[0] - 1];
            glm::vec3 v1 = temp_positions[vi[1] - 1];
            glm::vec3 v2 = temp_positions[vi[2] - 1];

            vertices.push_back(v0);
            vertices.push_back(v1);
            vertices.push_back(v2);

            glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
            normals.push_back(normal);
            normals.push_back(normal);
            normals.push_back(normal);
        }
    }

    fclose(file);
    return true;
}

int main(int argc, char** argv) {
    if (argc < 2 || !loadOBJ(argv[1])) {
        std::cerr << "Uso: ./viewer arquivo.obj" << std::endl;
        return 1;
    }

    ///////////////////////////////////////////////////////////
    // WINDOW INITIALIZATION
    ///////////////////////////////////////////////////////////

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
    {
        return -1;
    }

    // Set OpenGL version to 3.3 and core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello Triangle", nullptr, nullptr);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;

    // Initialize GLEW to load OpenGL function pointers
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "ERROR: GLEW Initialization Failed\n";
        return -1;
    }

    ///////////////////////////////////////////////////////////
    // GEOMETRY DEFINITION - Pyramid with vertices and normals
    ///////////////////////////////////////////////////////////

    // Define pyramid vertices (4 triangular faces + square base)
    // std::vector<glm::vec3> pyramid =
    //     { glm::vec3(-1.0f, -1.0f, 1.0f),  // Front face
    //         glm::vec3(1.0f, -1.0f, 1.0f),
    //         glm::vec3(0.0f, 1.0f, 0.0f),
    //         glm::vec3(1.0f, -1.0f, 1.0f),  // Right face
    //         glm::vec3(1.0f, -1.0f, -1.0f),
    //         glm::vec3(0.0f, 1.0f, 0.0f),
    //         glm::vec3(1.0f, -1.0f, -1.0f), // Back face
    //         glm::vec3(-1.0f, -1.0f, -1.0f),
    //         glm::vec3(0.0f, 1.0f, 0.0f),
    //         glm::vec3(-1.0f, -1.0f, -1.0f), // Left face
    //         glm::vec3(-1.0f, -1.0f, 1.0f),
    //         glm::vec3(0.0f, 1.0f, 0.0f),
    //         glm::vec3(-1.0f, -1.0f, -1.0f), // Base (2 triangles)
    //         glm::vec3(1.0f, -1.0f, 1.0f),
    //         glm::vec3(-1.0f, -1.0f, 1.0f),
    //         glm::vec3(1.0f, -1.0f, 1.0f),
    //         glm::vec3(-1.0f, -1.0f, -1.0f),
    //         glm::vec3(1.0f, -1.0f, -1.0f)
    //     };

    // // Normal vectors for each vertex (pre-calculated for flat shading)
    // std::vector<glm::vec3> pyramidNormals =
    //     { // Front face normals (normalized vector pointing forward+up)
    //         glm::vec3(0.0f, 0.707f, 0.707f),
    //         glm::vec3(0.0f, 0.707f, 0.707f),
    //         glm::vec3(0.0f, 0.707f, 0.707f),

    //         // Right face normals (pointing right+up)
    //         glm::vec3(0.707f, 0.707f, 0.0f),
    //         glm::vec3(0.707f, 0.707f, 0.0f),
    //         glm::vec3(0.707f, 0.707f, 0.0f),

    //         // Back face normals (pointing back+up)
    //         glm::vec3(0.0f, 0.707f, -0.707f),
    //         glm::vec3(0.0f, 0.707f, -0.707f),
    //         glm::vec3(0.0f, 0.707f, -0.707f),

    //         // Left face normals (pointing left+up)
    //         glm::vec3(-0.707f, 0.707f, 0.0f),
    //         glm::vec3(-0.707f, 0.707f, 0.0f),
    //         glm::vec3(-0.707f, 0.707f, 0.0f),

    //         // Base normals (all pointing down)
    //         glm::vec3(0.0f, -1.0f, 0.0f),
    //         glm::vec3(0.0f, -1.0f, 0.0f),
    //         glm::vec3(0.0f, -1.0f, 0.0f),

    //         glm::vec3(0.0f, -1.0f, 0.0f),
    //         glm::vec3(0.0f, -1.0f, 0.0f),
    //         glm::vec3(0.0f, -1.0f, 0.0f)
    //     };

    ///////////////////////////////////////////////////////////
    // BUFFER SETUP - VAO and VBOs
    ///////////////////////////////////////////////////////////

    GLuint VAO; // Vertex Array Object - stores vertex attribute configurations
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO); // Activate this VAO

    // Create 2 Vertex Buffer Objects (VBOs)
    GLuint vbo[2];
    glGenBuffers(2, vbo);

    // First VBO - vertex positions
    glGenBuffers(1, &vbo[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*3*sizeof(float), &vertices[0], GL_STATIC_DRAW);

    // Second VBO - vertex normals
    glGenBuffers(1, &vbo[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, normals.size()*3*sizeof(float), &normals[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Enable vertex attribute at index 0 (positions)

    ///////////////////////////////////////////////////////////
    // SHADER SETUP - Vertex and Fragment shaders
    ///////////////////////////////////////////////////////////

    // Vertex shader implements Phong lighting model
    const char* vertexShader =
        "#version 330 core\n"
        "uniform mat4 projMat;" // Projection matrix
        "uniform mat4 mvMat;"   // Model-view matrix

        // Vertex attributes
        "layout (location=0) in vec3 vertPos;"
        "layout (location=1) in vec3 vertNormal;"

        // Output to fragment shader
        "out vec4 varyingColor;"

        // Light properties structure
        "struct PositionalLight"
        "{ vec4 ambient;"
        "vec4 diffuse;"
        "vec4 specular;"
        "vec3 position;"
        "};"

        // Material properties structure
        "struct Material"
        "{ vec4 ambient;"
        "vec4 diffuse;"
        "vec4 specular;"
        "float shininess;"
        "};"

        // Uniform variables
        "uniform vec4 globalAmbient;"  // Global ambient light
        "uniform PositionalLight light;" // Light source
        "uniform Material material;"    // Surface material
        "uniform mat4 norm_matrix;"     // For transforming normals

        "void main()"
        "{"
        // Transform vertex to view space
        "vec4 P =  mvMat*vec4(vertPos,1.0);"

        // Transform and normalize normal vector
        "vec3 N = normalize((norm_matrix * vec4(vertNormal, 1.0)).xyz);"

        // Calculate light direction (L) and view direction (V)
        "vec3 L = normalize(light.position - P.xyz);"
        "vec3 V = normalize(-P.xyz);"

        // Calculate reflection vector (R)
        "vec3 R = reflect(-L, N);"

        // Gouraud lighting calculations:
        // Ambient = global ambient + light's ambient
        "vec3 ambient = ((globalAmbient * material.ambient) + (light.ambient * material.ambient)).xyz;"

        // Diffuse = Lambert's cosine law
        "vec3 diffuse = light.diffuse.xyz * material.diffuse.xyz * max(dot(N,L), 0.0);"

        // Specular = Blinn-Phong highlight
        "vec3 specular= material.specular.xyz * light.specular.xyz * pow(max(dot(R,V), 0.0f), material.shininess);"

        // Combine all components
        "varyingColor = vec4((ambient + diffuse + specular), 1.0);"

        // Final vertex position
        "gl_Position = (projMat*mvMat)*vec4(vertPos, 1.0);"
        "}";

    // Fragment shader - simply outputs the interpolated color
    const char* fragmentShader =
        "#version 330 core\n"
        "out vec4 color;"
        "in vec4 varyingColor;"
        "void main()"
        "{"
        "color = varyingColor;" // Pass through the color from vertex shader
        "}";

    // Compile and link shaders
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
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projMat));

    ///////////////////////////////////////////////////////////
    // LIGHTING AND MATERIAL SETUP
    ///////////////////////////////////////////////////////////

    glm::mat4 modelMat, viewMat, mvMat;

    // Light properties
    glm::vec3 currentLightPos, lightPosV; // World and view space positions
    float lightPos[3]; // Will store view space position

    // Light position in world space
    currentLightPos = glm::vec3(0.0f, 10.0f, 0.0f);

    // White light properties
    float globalAmbient[4] = {1.0f, 1.0f, 1.0f, 1.0f }; // Global ambient light

    // Light source properties (white light)
    float lightAmbient[4] = {1.0f, 1.0f, 1.0f, 1.0f };
    float lightDiffuse[4] = {1.0f, 1.0f, 1.0f, 1.0f };
    float lightSpecular[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    // Gold material properties
    float matAmbGold[4] = {0.2473f, 0.1995f, 0.0745f, 1.f}; // Ambient reflectivity
    float matDifGold[4] = {0.7516f, 0.6065f, 0.2265f, 1.f}; // Diffuse reflectivity
    float matSpeGold[4] = {0.6283f, 0.5558f, 0.3661f, 1.f}; // Specular reflectivity
    float matShiGold = 15.f; // Shininess exponent

    // Propriedades para papel
    float matAmbPaper[4] = {0.1f, 0.1f, 0.1f, 1.f}; // Refletividade ambiente
    float matDifPaper[4] = {0.8f, 0.8f, 0.8f, 1.f}; // Refletividade difusa
    float matSpePaper[4] = {0.2f, 0.2f, 0.2f, 1.f}; // Refletividade especular
    float matShiPaper = 5.f; // Exponente de brilho

    // Propriedades para vidro
    float matAmbGlass[4] = {0.0f, 0.0f, 0.0f, 1.f}; // Refletividade ambiente (translúcido)
    float matDifGlass[4] = {0.0f, 0.0f, 0.0f, 1.f}; // Refletividade difusa (translúcido)
    float matSpeGlass[4] = {0.9f, 0.9f, 0.9f, 1.f}; // Refletividade especular
    float matShiGlass = 128.f; // Exponente de brilho

    // Propriedades para plástico brilhante
    float matAmbPlastic[4] = {0.1f, 0.1f, 0.1f, 1.f}; // Refletividade ambiente
    float matDifPlastic[4] = {0.5f, 0.5f, 0.5f, 1.f}; // Refletividade difusa
    float matSpePlastic[4] = {0.9f, 0.9f, 0.9f, 1.f}; // Refletividade especular
    float matShiPlastic = 30.f; // Exponente de brilho

    // Propriedades para marfim
    float matAmbIvory[4] = {0.3f, 0.3f, 0.2f, 1.f}; // Refletividade ambiente
    float matDifIvory[4] = {0.7f, 0.7f, 0.5f, 1.f}; // Refletividade difusa
    float matSpeIvory[4] = {0.4f, 0.4f, 0.4f, 1.f}; // Refletividade especular
    float matShiIvory = 20.f; // Exponente de brilho


    // Animation variables
    float modelAngle = 0.f; // Model rotation angle
    float camAngle = 0.f;   // Camera rotation angle

    // Camera position
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 22.0f);

    ///////////////////////////////////////////////////////////
    // RENDERING LOOP
    ///////////////////////////////////////////////////////////
    while (!glfwWindowShouldClose(window))
    {
        /* Clear the screen */
        glClearColor(0.2, 0.2,0.5,0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Get uniform locations for lighting parameters
        GLuint mvLoc = glGetUniformLocation(shaderProgram, "mvMat");

        // Light uniform locations
        GLuint globalAmbLoc = glGetUniformLocation(shaderProgram, "globalAmbient");
        GLuint ambLoc = glGetUniformLocation(shaderProgram, "light.ambient");
        GLuint diffLoc = glGetUniformLocation(shaderProgram, "light.diffuse");
        GLuint specLoc = glGetUniformLocation(shaderProgram, "light.specular");
        GLuint posLoc = glGetUniformLocation(shaderProgram, "light.position");

        // Material uniform locations
        GLuint mAmbLoc = glGetUniformLocation(shaderProgram, "material.ambient");
        GLuint mDiffLoc = glGetUniformLocation(shaderProgram, "material.diffuse");
        GLuint mSpecLoc = glGetUniformLocation(shaderProgram, "material.specular");
        GLuint mShiLoc = glGetUniformLocation(shaderProgram, "material.shininess");

        /* VIEW MATRIX SETUP */
        glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 upDirection = glm::vec3(0.0f, 1.0f, 0.0f);

        // Camera rotation calculations
        auto CTo = glm::translate(glm::mat4(1.f), -cameraPos);
        auto CTb = glm::translate(glm::mat4(1.f), cameraPos);
        auto CR = glm::rotate(glm::mat4(1.f), glm::radians(-camAngle), glm::vec3(0.f, 1.f, 0.f));

        // // Calculate rotated camera position
        auto cameraPosR = glm::vec3(glm::vec4(cameraPos, 1.0) * (CTb * CR * CTo));


        // Create view matrix
        glm::mat4 viewMat = glm::lookAt(cameraPosR, targetPos, upDirection);

        /* MODEL MATRIX SETUP */
        // Rotate model around its base
        auto To = glm::translate(glm::mat4(1.f), -vertices[0]);
        auto S = glm::scale(glm::mat4(1.f), glm::vec3(45.0f, 45.0f, 45.0f));
        auto R = glm::rotate(glm::mat4(1.f), glm::radians(modelAngle), glm::vec3(0.f,1.f,0.f));
        auto Tb = glm::translate(glm::mat4(1.f), vertices[0]);

        modelMat = Tb*S*R*To;

        // Combined model-view matrix
        mvMat = viewMat * modelMat;

        // Send to shader
        glUniformMatrix4fv(mvLoc, 1, GL_FALSE, glm::value_ptr(mvMat));

        /* LIGHT POSITION TRANSFORMATION */
        // Convert light position to view space
        lightPosV = glm::vec3(viewMat * glm::vec4(currentLightPos, 1.0));
        lightPos[0] = lightPosV.x;
        lightPos[1] = lightPosV.y;
        lightPos[2] = lightPosV.z;

        /* SET LIGHTING AND MATERIAL UNIFORMS */
        glProgramUniform4fv(shaderProgram, globalAmbLoc, 1, globalAmbient);
        glProgramUniform4fv(shaderProgram, ambLoc, 1, lightAmbient);
        glProgramUniform4fv(shaderProgram, diffLoc, 1, lightDiffuse);
        glProgramUniform4fv(shaderProgram, specLoc, 1, lightSpecular);
        glProgramUniform3fv(shaderProgram, posLoc, 1, lightPos);
        glProgramUniform4fv(shaderProgram, mAmbLoc, 1, matAmbIvory);
        glProgramUniform4fv(shaderProgram, mDiffLoc, 1, matDifIvory);
        glProgramUniform4fv(shaderProgram, mSpecLoc, 1, matSpeIvory);
        glProgramUniform1f(shaderProgram, mShiLoc, matShiIvory);
        

        /* NORMAL MATRIX - For correct normal transformation */
        GLuint nLoc = glGetUniformLocation(shaderProgram, "norm_matrix");
        glm::mat4 invTrMat = glm::transpose(glm::inverse(mvMat));
        glUniformMatrix4fv(nLoc, 1, GL_FALSE, glm::value_ptr(invTrMat));

        /* VERTEX ATTRIBUTE SETUP */
        // Position attribute
        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(0);

        // Normal attribute
        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(1);

        /* DRAW THE PYRAMID */
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        /* SWAP BUFFERS AND POLL EVENTS */
        glfwSwapBuffers(window);
        glfwPollEvents();

        /* UPDATE ANIMATION ANGLES */
        modelAngle = modelAngle <= 360 ? modelAngle+0.5f : 0;
        //camAngle = camAngle < 360 ? camAngle+ 0.5f : 0;
    }

    glfwTerminate();
    return 0;
}