#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include "glad/glad.h"  //include glad.h before glfw3.h
#include "GLFW/glfw3.h"

//global constants
constexpr int WINDOW_H = 600;
constexpr int WINDOW_W = 800;
//statics 
static GLFWwindow* myWindow;
static unsigned int program_ids[10];    //should support dynamic id numbers
static unsigned int VAO_ids[10];
//functions 
void frame_buffer_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow* window);
inline bool initialize();
inline void render();
inline void sendVertexData();
inline bool makeShaderProgram(const char* vertex_shader_path, const char* fragment_shader_path, unsigned int &program_id);
int main()
{
    if (!initialize())
    {
        glfwTerminate();
        return -1;
    }
    sendVertexData();
    if (!makeShaderProgram("src/vShader.vert", "src/fShader.frag", program_ids[0]))
    {
        glfwTerminate();
        return -1;
    }
    if (!makeShaderProgram("src/vShader.vert", "src/fShader2.frag", program_ids[1]))
    {
        glfwTerminate();
        return -1;
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
bool readShaderFile(const char* file_name, char* &file_contents_holder)
{
    std::ifstream reader;
    reader.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        reader.open(file_name);
        std::stringstream file_stream;
        file_stream << reader.rdbuf();
        reader.close();
        file_contents_holder = new char[strlen(file_stream.str().c_str())];
        strcpy(file_contents_holder, file_stream.str().c_str());
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "failed to open shader file : " << file_name;
        return false;
    }
    return true;
}
enum shader_type_option
{
    VERTEX_SHADER,
    FRAGMENT_SHADER
};
bool compileShader(shader_type_option shader, unsigned int &shader_id, const char* const &shader_source)
{
    GLenum shader_type;
    if (shader == VERTEX_SHADER)
        shader_type = GL_VERTEX_SHADER;
    if (shader == FRAGMENT_SHADER)
        shader_type = GL_FRAGMENT_SHADER;

    shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, &shader_source, NULL);
    glCompileShader(shader_id);
    {
        int success_status;
        char infoLog[512];
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success_status);
        if(!success_status)
        { 
            glGetShaderInfoLog(shader_id, 512, NULL, infoLog);
            std::cout << "compilation failed : " << (shader == VERTEX_SHADER ? "vertex  shader" : "fragment shader") 
            << "\n" << infoLog; 
            return false;
        }
    }
    return true;
}
bool makeShaderProgram(const char* vertex_shader_path, const char* fragment_shader_path, unsigned int &program_id)
{
    unsigned int vertex_shader_id;
    char* vertex_shader_source;
    if(!readShaderFile(vertex_shader_path, vertex_shader_source))
        return false;
    if (!compileShader(VERTEX_SHADER, vertex_shader_id, vertex_shader_source))
        return false;

    unsigned int fragment_shader_id;
    char* fragment_shader_source;
    if(!readShaderFile(fragment_shader_path, fragment_shader_source))
        return false;
    if (!compileShader(FRAGMENT_SHADER, fragment_shader_id, fragment_shader_source))
        return false;
 
    program_id  = glCreateProgram();
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);
    glLinkProgram(program_id);
    {
        int success_status;
        char infoLog[512];
        glGetProgramiv(program_id, GL_LINK_STATUS, &success_status);
        if(!success_status)
        {
            glGetProgramInfoLog(program_id, 512, NULL, infoLog);
            std::cout << "Linking failed : " << infoLog;
            return false;
        }
    } 
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);
    delete[] fragment_shader_source;
    delete[] vertex_shader_source;
    return true;
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
    glBindVertexArray(VAO_ids[0]);
    glUseProgram(program_ids[0]);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glBindVertexArray(VAO_ids[1]);
    glUseProgram(program_ids[1]);
    glDrawArrays(GL_TRIANGLES, 1, 3);
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
