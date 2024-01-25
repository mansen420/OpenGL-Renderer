#ifndef WINDOW
#define WINDOW

#include "glad/glad.h"  //include glad.h before glfw3.h
#include "GLFW/glfw3.h"
#include "global_constants.h"
#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

namespace window
{
    GLFWwindow* myWindow;
    bool init()
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
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(myWindow, true);
        ImGui_ImplOpenGL3_Init();
        return true;
    }
    void poll_events()
    {
        glfwPollEvents();
    }
    void swap_buffers()
    {
        glfwSwapBuffers(myWindow);
    }
    void render_gui()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();


        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    void terminate()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwTerminate();
    }
}

#endif