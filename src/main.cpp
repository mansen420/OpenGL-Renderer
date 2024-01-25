#include "glad/glad.h"  //include glad.h before glfw3.h
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>

#include "global_constants.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

GLFWwindow* myWindow;
inline bool initialize();
int main()
{
    if (!initialize())
    {
        glfwTerminate();
        return -1;
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(myWindow, true);
    ImGui_ImplOpenGL3_Init();
    while (true)
    {
        glClear(GL_COLOR_BUFFER_BIT);   
        glClearColor(0.6f, 0.2f, 0.2f, 1.0f);

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(myWindow);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
inline bool initialize()
{
    //glfw boilerplate
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
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
    return true;
}
