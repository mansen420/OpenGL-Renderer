#include "renderer.h"
#include "gui.h"
//TODO separate the logic into the event handler module...
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
    if (BeginTabBar("Workspace")) 
    {
        if(BeginTabItem("Engine Settings"))
        {
            PushItemWidth(GetWindowWidth()*0.15);

            Spacing();
            SeparatorText("Render Target Framebuffer Settings");
            Spacing();

            using namespace renderer;
            using namespace settings;

            //TODO separate the evend handling logic and the gui stuff
            Checkbox("Clear Depth Buffer", &DEPTH_CLR_ENBLD); SameLine();
            Checkbox("Clear Color Buffer", &COLOR_CLR_ENBLD); SameLine();
            Checkbox("Clear Stencil Buffer", &STENCIL_CLR_ENBLD);
            Spacing();
            Checkbox("Enable Depth Testing", &DEPTH_TEST_ENBLD); SameLine();
            Checkbox("Enable Stencil Testing", &STENCIL_TEST_ENBLD); SameLine();
            Spacing();
            ColorEdit4("Framebuffer Clear Color", &CLR_COLOR.r, ImGuiColorEditFlags_NoInputs); SameLine();
            ColorEdit3("Depth Viewing Color", &DEPTH_VIEW_COLOR.r, ImGuiColorEditFlags_NoInputs);
            Spacing();

            //bool color_active = true, depth_active = false;
            static int active_1 = 0;
            if (RadioButton("View Color Buffer", &active_1, 0))
            {
                scr_display_mode = COLOR;
            } SameLine();
            if (RadioButton("View Depth BUffer", &active_1, 1))
            {
                scr_display_mode = DEPTH;
            }   

            Spacing();
            Text("Framebuffer Dimensions");
            Spacing();

            PopItemWidth();
            PushItemWidth(GetWindowWidth()*0.25);
            int render_w_candidate, render_h_candidate;
            render_w_candidate = int(RENDER_W); render_h_candidate = int(RENDER_H);
            bool should_update_screen_coords = false;
            if (InputInt("Width", &render_w_candidate, 50.0, 500.0, ImGuiInputTextFlags_None))
            {
                if(render_w_candidate >= 0 && render_w_candidate <= MAX_RENDER_W)
                {
                    RENDER_W = size_t(render_w_candidate);
                    should_update_screen_coords = true;
                }
            } SameLine();
            if (InputInt("Height", &render_h_candidate, 50.0, 500.0, ImGuiInputTextFlags_None))
            {
                if(render_h_candidate >= 0 && render_h_candidate <= MAX_RENDER_H)
                {
                    RENDER_H = size_t(render_h_candidate);
                    should_update_screen_coords = true;
                }
            }
            
            Spacing();
            Text("Framebuffer Texture Filtering");
            Spacing();

            //TODO the depth texture and color texture both use the same settings, maybe worth it to separate them?
            static int minification_filter = 0;
            static int mipmap_filter = 0;
            if (Combo("Minification", &minification_filter, "Linear\0Nearest\0\0"))
            {
                //FIXME please find a better way to do this, this is ugly as fuck 
                if(use_mipmaps)
                {
                    if(minification_filter == 0)
                    {
                        if(mipmap_filter == 0)
                            scr_tex_min_filter = MIPMAP_LINEAR_LINEAR;
                        if (mipmap_filter == 1)
                            scr_tex_min_filter = MIPMAP_NEAREST_LINEAR;
                    }
                    if(minification_filter == 1)
                    {
                        if(mipmap_filter == 0)
                            scr_tex_min_filter = MIPMAP_LINEAR_NEAREST;
                        if (mipmap_filter == 1)
                            scr_tex_min_filter = MIPMAP_NEAREST_NEAREST;
                    }
                }
                else
                {
                    if(minification_filter == 0)
                        scr_tex_min_filter = LINEAR;
                    if(minification_filter == 1)
                        scr_tex_min_filter = NEAREST;
                }
                update_offscreen_tex_params();
            } SameLine();
            Checkbox("Use Mipmaps", &use_mipmaps);
            if (use_mipmaps)
            {   
                SameLine();
                Combo("Mipmap Filtering", &mipmap_filter, "Linear\0Nearest\0\0");
            }
            static int magnification_filter = 0;
            if (Combo("Magnification", &magnification_filter, "Linear\0Nearest\0\0"))
            {
                if (magnification_filter == 0)
                    scr_tex_mag_filter = LINEAR;
                if (magnification_filter == 1)
                    scr_tex_mag_filter = NEAREST;
                update_offscreen_tex_params();
            }

            Spacing();
            SeparatorText("Projection Settings");
            Spacing();

            float RENDER_AR_candidate = RENDER_AR;
            if (InputFloat("Aspect Ratio", &RENDER_AR_candidate, 0.1, 1.0))
            {
                if (RENDER_AR_candidate >= 0)
                {
                    should_update_screen_coords = true;
                    RENDER_AR = RENDER_AR_candidate;
                }
            }
            if(should_update_screen_coords)
                update_screen_tex_coords();
            DragFloat("FOV", &fov, 0.65, 0.0, 180.0  ); SameLine(); //TODO 180 > fov > 0
            DragFloatRange2("Projection Plane", &near_plane, &far_plane, 0.05, 0.001, 100.0, "%.3f", NULL, ImGuiSliderFlags_AlwaysClamp);
            EndTabItem();
        }
        EndTabBar();
    }
    End();
}
void events_gui()
{
    using namespace window;
    using namespace ImGui;
    if (should_import)
    {
        file_dialog.Display();
        if (file_dialog.HasSelected())
        {
            file_dialog.ClearSelected();
            should_import = false;
        }
    }
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
            if(MenuItem("Import", "ctrl+o", false))
            {
                //this is pretty retarded
                window::file_dialog.Open();
                should_import = true;
            }
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