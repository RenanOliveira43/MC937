#include <glad.h>
#include <iostream>
#include <GLFW/glfw3.h>


int main(){

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello Triangle", nullptr, nullptr);

    if (!window) {
        glfwTerminate();
        return -1;
    }


    glfwMakeContextCurrent(window);

    // Inicializa o GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Falha ao inicializar o GLAD" << std::endl;
        return -1;
    }

    // OpenGL begins here

    // Vertex data for a triangle
    GLfloat vertices[] = {
        0.0f, 0.5f, 0.0f, // Top vertex
        -0.5f, -0.5f, 0.0f,  // Bottom-left vertex
        0.5f, -0.5f, 0.0f   // Bottom-right vertex
    };

    // Create VAO and VBO for the triangle

    GLuint VAO; //Stores the configuration of how vertex data is read from the VBO.
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO); //make it active


    GLuint VBO; //Stores vertex data (positions, colors, texture coordinates, etc.) in GPU memory.
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO); //make it active

    //Defines how vertex data is structured
    glVertexAttribPointer(0,// Attribute location in the shader
                          3,//Number of components per vertex
                          GL_FLOAT,//Data type of each component
                          GL_FALSE,//Normalize integer values to [0,1]
                          0,//Byte offset between consecutive attributes
                          nullptr);//Offset from the start of VBO where data begins;



    const char * vertex_shader =
        "#version 330 core\n"
        "in vec3 vp;"
        "void main(){"
        "gl_Position = vec4(vp, 1.0);"
        "}";

    const char * fragmet_shader =
        "#version 330 core\n"
        "out vec4 frag_color;"
        "void main(){"
        "frag_color = vec4(0.5, 0.0, 0.0, 0.5);"
        "}";


    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertex_shader, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmet_shader, nullptr);
    glCompileShader(fs);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, fs);
    glAttachShader(shaderProgram, vs);
    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {

        /* Render here */
        glClearColor(0.0f, 0.0f, 0.5f, 0.4f);
        glClear(GL_COLOR_BUFFER_BIT);


        // Upload data
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*3, &vertices[0], GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); //Enables the attribute (position)

        glDrawArrays(GL_TRIANGLES, 0, 3);


        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }



    std::cout << "Hello World" << std::endl;
    return 0;
}
