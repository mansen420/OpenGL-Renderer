#pragma once

#include "glad/glad.h"  //include glad.h before glfw3.h
#include "GLFW/glfw3.h"
#include "global_constants.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "callbacks.h"

#include <iostream>
//event handler
namespace events
{
    void import()
    {
        std::cout << "IMPORT!" << std::endl;    //temporary, obviously
    }
    void quit(){glfwSetWindowShouldClose(myWindow, true);}
    void quit_window()
    {
        //maybe do this?
        using namespace ImGui;
        bool pause_main_loop = true;
        while (pause_main_loop)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if(Begin("Quit", NULL, ImGuiWindowFlags_NoDecoration))
            {
                Text("Do you want to quit?");
                End();            
            }
            
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
    }
    inline void poll()
    {
        if(should_import) {should_import = false; import();}
        if (should_quit){should_quit = false; quit();}
    }
}