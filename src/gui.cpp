#include "renderer.h"
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
    PushItemWidth(GetWindowWidth()*0.15);

    SeparatorText("Framebuffer Settings");

    using namespace renderer;
    using namespace settings;

    Checkbox("Clear Depth Buffer", &DEPTH_CLR_ENBLD);
    Checkbox("Clear Color Buffer", &COLOR_CLR_ENBLD);
    Checkbox("Clear Stencil Buffer", &STENCIL_CLR_ENBLD);
    Spacing();
    Checkbox("Enable Depth Testing", &DEPTH_TEST_ENBLD);
    Checkbox("Enable Stencil Testing", &STENCIL_TEST_ENBLD);
    Spacing();
    ColorEdit4("Framebuffer Clear Color", &CLR_COLOR.r, ImGuiColorEditFlags_NoInputs);

    SeparatorText("Render Settings");

    PopItemWidth();
    PushItemWidth(GetWindowWidth()*0.25);
    int render_w_candidate, render_h_candidate;
    render_w_candidate = int(RENDER_W); render_h_candidate = int(RENDER_H);
    bool should_update_screen_coords = false;
    if (InputInt("Rendering Width", &render_w_candidate, 50.0, 500.0, ImGuiInputTextFlags_None))
    {
        if(render_w_candidate >= 0 && render_w_candidate <= MAX_RENDER_W)
        {
            RENDER_W = size_t(render_w_candidate);
            should_update_screen_coords = true;
        }
    }
    if (InputInt("Rendering Height", &render_h_candidate, 50.0, 500.0, ImGuiInputTextFlags_None))
    {
        if(render_h_candidate >= 0 && render_h_candidate <= MAX_RENDER_H)
        {
            RENDER_H = size_t(render_h_candidate);
            should_update_screen_coords = true;
        }
    }
    float RENDER_AR_candidate = RENDER_AR;
    if (InputFloat("Rendering Aspect Ratio", &RENDER_AR_candidate, 0.1, 1.0))
    {
        if (RENDER_AR_candidate >= 0)
        {
            should_update_screen_coords = true;
            RENDER_AR = RENDER_AR_candidate;
        }
    }
    if(should_update_screen_coords)
        update_screen_tex_coords();
    End();
}
void main_bar()
{
    using namespace ImGui;
    ImGuiViewport* whole_window = GetMainViewport();
    ImVec2 pos(whole_window->Pos.x, whole_window->Pos.y);
    SetNextWindowPos(ImVec2(pos));
    SetNextWindowSize(ImVec2(OPENGL_VIEWPORT_W, -1));
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