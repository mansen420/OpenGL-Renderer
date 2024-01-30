#include "gui.h"
void workspace_panel()
{
    using namespace ImGui;
    ImGuiViewport* whole_window = GetMainViewport();
    ImVec2 pos(whole_window->Pos.x+OPENGL_VIEWPORT_W, whole_window->Pos.y);
    SetNextWindowPos(ImVec2(pos));
    SetNextWindowSize(ImVec2((WINDOW_W-OPENGL_VIEWPORT_W), WINDOW_H));
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
    | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar;

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
void main_bar()
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