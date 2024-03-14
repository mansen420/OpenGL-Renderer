#include "window.h"

#include "glad/glad.h"  //include glad.h before glfw3.h
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imfilebrowser.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "global_constants.h"
#include "callbacks.h"
#include "gui.h"
//TODO the current situation with the file dialog is not the final solution but a proof of concept.
ImGui::FileBrowser window::file_dialog;
int window::init()
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

    file_dialog.SetTitle("Import");
    file_dialog.SetTypeFilters({".obj"});
    file_dialog.Open();

    return true;
}
bool window::should_close(){return glfwWindowShouldClose(myWindow);}
void window::render_gui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //TODO something about fonts
    GUI::render();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void window::poll_events()
{
    glfwPollEvents();
}
void window::swap_buffers()
{
    glfwSwapBuffers(myWindow);
}
void window::terminate()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    file_dialog.Close();
    ImGui::DestroyContext();
    glfwTerminate();
}