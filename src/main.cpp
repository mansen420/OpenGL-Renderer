#include "glad/glad.h"  //include glad.h before glfw3.h
#include "GLFW/glfw3.h"
#include "shader_utils.h"

//global constants
constexpr int WINDOW_H = 600;
constexpr int WINDOW_W = 800;
//statics 
static GLFWwindow* myWindow;
static unsigned int program_ids[10];    //TODO should support dynamic id numbers
static unsigned int VAO_ids[10];
//functions 
void frame_buffer_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow* window);
inline bool initialize();
inline void render();
inline void sendVertexData();
int main()
{
    if (!initialize())
    {
        glfwTerminate();
        return -1;
    }
    sendVertexData();
    {   //link shaders using a single vertex shader id. Program switching is expensive.
        //the same shader can be attached to multiple programs, and the inverse is true.
        unsigned int vShader, fShader, fShader2;
        bool shaders_made = 
        compileShaderFromPath(VERTEX_SHADER, vShader, "src/vShader.vert") &&
        compileShaderFromPath(FRAGMENT_SHADER, fShader, "src/fShader.frag")&&
        compileShaderFromPath(FRAGMENT_SHADER, fShader2, "src/fShader2.frag")&&
        linkShaders(program_ids[0], vShader, fShader) &&
        linkShaders(program_ids[1], vShader, fShader2);
        if (!shaders_made)
        {
            glfwTerminate();
            return -1;
        }
        glDeleteShader(vShader);
        glDeleteShader(fShader);
        glDeleteShader(fShader2);
    }
    //renderloop
    while (!glfwWindowShouldClose(myWindow))
    {
        render();
        process_input(myWindow);
        glfwSwapBuffers(myWindow);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
void sendVertexData()
{
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    float vertices[] = 
    {
        0.5f, 0.5f, 0.0f, // top right
        0.5f, -0.5f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f // top left
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glGenVertexArrays(1, &VAO_ids[0]);
    glBindVertexArray(VAO_ids[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    unsigned int indices[] = 
    { 
        0, 1, 3, // first triangle
        1, 2, 3 // second triangle
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices, GL_STATIC_DRAW);

    glBindVertexArray(0);

 
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    float vertices2[] = 
    {
        0.7f, 0.7f, 0.0f, // top right
        0.7f, -0.3f, 0.0f, // bottom right
        -0.3f, -0.3f, 0.0f, // bottom left
        -0.3f, 0.7f, 0.0f // top left
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), &vertices2, GL_STATIC_DRAW);

    glGenVertexArrays(1 , &VAO_ids[1]);
    glBindVertexArray(VAO_ids[1]);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}
void render()
{
    glClearColor(0.65f, 0.45f, 0.75f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
    for (int i = 0; i < 10; i++)
    {
        glBindVertexArray(VAO_ids[0]);
        glUseProgram(program_ids[0]);
        float ratio = 2*(float(i)/10)-1.0;
        glUniform3f(glGetUniformLocation(program_ids[0], "pos_offset"), ratio + 2*(float(rand())/RAND_MAX),
        ratio + 2*(float(rand())/RAND_MAX), ratio);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        //glBindVertexArray(VAO_ids[1]);
        //glUseProgram(program_ids[1]);
        //glDrawArrays(GL_TRIANGLES, 1, 3);
    }
//  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
void frame_buffer_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void process_input(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}
inline bool initialize()
{
    //glfw boilerplate
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    myWindow = glfwCreateWindow(WINDOW_W, WINDOW_H, "OPGL", NULL, NULL);
    if (myWindow == NULL)
    {
        glfwTerminate();
        std::cout << "failed to instantiate window";
        return false;
    }
    glfwMakeContextCurrent(myWindow);
    //load gl functions. Don't call gl functions before this
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "failed to initialize GLAD" << std::endl;
        return false;
    }
    glViewport(0, 0, WINDOW_W, WINDOW_H);
    glfwSetFramebufferSizeCallback(myWindow, frame_buffer_callback);
    return true;
}
