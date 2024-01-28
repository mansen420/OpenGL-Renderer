#ifndef WINDOW
#define WINDOW

#include "glad/glad.h"  //include glad.h before glfw3.h
#include "GLFW/glfw3.h"
#include "global_constants.h"
#include <iostream>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "callbacks.h"
#include "event_handling.h"

namespace window
{
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

        glViewport(OPENGL_VIEWPORT_X, OPENGL_VIEWPORT_Y, OPENGL_VIEWPORT_W, OPENGL_VIEWPORT_H);

        register_GLFW_callbacks();

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(myWindow, true);
        ImGui_ImplOpenGL3_Init();

        return true;
    }
    
    inline void workspace_panel()
    {
        using namespace ImGui;
        ImGuiViewport* whole_window = GetMainViewport();
        ImVec2 pos(whole_window->Pos.x+OPENGL_VIEWPORT_W, whole_window->Pos.y);
        SetNextWindowPos(ImVec2(pos));
        SetNextWindowSize(ImVec2((WINDOW_W-OPENGL_VIEWPORT_W), WINDOW_H));
        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_MenuBar;

        if (!Begin("Work Space", NULL, flags))
        {   
            End();
            return;
        }
        Text("Some really convenient stuff!");
        ImGui::Spacing();
        SeparatorText("Something");
        SeparatorText("Something else");
        End();
    }
    inline void main_bar()
    {
        using namespace ImGui;
        ImGuiViewport* whole_window = GetMainViewport();
        ImVec2 pos(whole_window->Pos.x, whole_window->Pos.y);
        SetNextWindowPos(ImVec2(pos));
        SetNextWindowSize(ImVec2(OPENGL_VIEWPORT_W, WINDOW_H-OPENGL_VIEWPORT_H));
        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar;

        if (!Begin("Menu Bar", NULL, flags))
        {
            End();
            return;
        }
        PushItemWidth(GetFontSize());
        if(BeginMenuBar())  //main menu bar
        {
            if(BeginMenu("File"))
            {
                if(MenuItem("Import", "ctrl+o", &should_import)){}
                EndMenu();
            }
            if(BeginMenu("Options", false))
            {
                EndMenu();
            }
            ImGui::Separator();
            MenuItem("Quit", "esc", &should_quit);
            EndMenuBar();
        }
        End();
    }
    void render_gui()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow();
        ImFont* font_ptr = ImGui::GetFont();
        font_ptr->Scale = 1.5f;
        workspace_panel();
        main_bar();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    void poll_events()
    {
        events::poll();
        glfwPollEvents();
    }
    void swap_buffers()
    {
        glfwSwapBuffers(myWindow);
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