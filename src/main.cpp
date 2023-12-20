#include "glad/glad.h"  //include glad.h before glfw3.h
#include "GLFW/glfw3.h"
#include "shader_utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
//global constants
constexpr int WINDOW_H = 600;
constexpr int WINDOW_W = 800;
//statics 
static GLFWwindow* myWindow;

static unsigned int program_ids[10];    //TODO should support dynamic id numbers
static unsigned int VAO_ids[10];
static unsigned int tex_ids[10];

static glm::vec3 cam_pos(0, 0, 1);
static glm::vec3 cam_front(0, 0, -1);
static glm::vec3 cam_up(0, 1, 0);

static float frame_delta = 0.0;
static glm::vec2 mouse_pos(float(WINDOW_W)/2.0, float(WINDOW_H)/2.0);
float yaw = -90.0f;
float pitch;
//functions 
void frame_buffer_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow* window);
inline bool initialize();
inline void render();
inline void sendVertexData();
bool gen_texture(const char* file_path, unsigned int &tex_id);
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
    stbi_set_flip_vertically_on_load(true);
    gen_texture("container.jpg", tex_ids[0]);
    gen_texture("wall.jpg", tex_ids[1]);
    //renderloop
    glEnable(GL_DEPTH_TEST);
    float previous_frame_time = 0.0;
    while (!glfwWindowShouldClose(myWindow))
    {
        render();
        frame_delta = glfwGetTime() - previous_frame_time;
        previous_frame_time = glfwGetTime();
        process_input(myWindow);
        glfwSwapBuffers(myWindow);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}
bool gen_texture(const char* file_path, unsigned int &tex_id)
{
    int img_width, img_height, img_nrChannels;
    unsigned char* data = stbi_load(file_path, &img_width, &img_height, &img_nrChannels, 0);
    if (!data)
    {
        std::cout << "reading texture file failed : " << file_path << std::endl;
        return false;
    }

    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img_width, img_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    
    return true;
}
void sendVertexData()
{
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    float vertices[] = {
    //pos                 //tex
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO_ids[0]);
    glBindVertexArray(VAO_ids[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}
inline void send_transforms()
{
    using namespace glm;
    mat4 model(1.0f);
    model = rotate(model, radians(0.f), vec3(1, 0, 0));
    mat4 view(1.0f);
    float theta = glfwGetTime();
    // cam_pos = vec3(cos(theta)*2.0, 0, sin(theta)*2.0);
    view = lookAt(cam_pos, cam_pos + cam_front, cam_up);
    mat4 projection = perspective(radians(45.f), float(WINDOW_W)/WINDOW_H, 0.1f, 100.f);
    
    glUniformMatrix4fv(glGetUniformLocation(program_ids[0], "model_transform"), 1, GL_FALSE, value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(program_ids[0], "view_transform"), 1, GL_FALSE, value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(program_ids[0], "projection_transform"), 1, GL_FALSE, value_ptr(projection));
}
void render()
{
    glClearColor(0.65f, 0.45f, 0.75f, 1.f);
    // glClearColor(0.45f, 0.35f, 0.70f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)  ;
    glBindVertexArray(VAO_ids[0]);
    glUseProgram(program_ids[0]);

    send_transforms();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_ids[0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, tex_ids[1]);

    glUniform1i(glGetUniformLocation(program_ids[0], "tex_sampler0"), 0);
    glUniform1i(glGetUniformLocation(program_ids[0], "tex_sampler1"), 1);

    glDrawArrays(GL_TRIANGLES, 0, 36);
}
void frame_buffer_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
void process_input(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    const float speed = 2*frame_delta;
    const glm::vec3 cam_right = glm::normalize(glm::cross(cam_front, cam_up)) * speed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cam_pos += speed * cam_front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cam_pos -= speed * cam_front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cam_pos -= cam_right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cam_pos += cam_right;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cam_up += cam_right;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cam_up -= cam_right;
}
void mouse_pos_callback(GLFWwindow* window, double x_pos, double y_pos)
{
    glm::vec2 offset = glm::vec2(x_pos, y_pos) - mouse_pos;
    mouse_pos = glm::vec2(x_pos, y_pos);
    offset *= 0.1f; //sensitivity 
    yaw += offset.x;
    pitch -= offset.y;  //mouse y range is inverted 

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;
    
    glm::vec3 cam_angle;
    cam_angle.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    cam_angle.y = sin(glm::radians(pitch));
    cam_angle.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cam_front = glm::normalize(cam_angle);
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
    glfwSetInputMode(myWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(myWindow, mouse_pos_callback);
    return true;
}
