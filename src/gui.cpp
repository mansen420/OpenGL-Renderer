#include "engine_state.h"       //*for setting global variables
#include "event_handling.h"     //*for calling engine state update functions  
#include "gui.h"
//TODO separate the logic into the event handler module...
//update : right now, gui.cpp only directly handles global varaiables declared in engine_state.h
static bool should_show_filedialog = false;
void workspace_panel()
{
    //FIXME probably a bad idea
    using namespace ImGui;
    {
        ImGuiViewport* whole_window = GetMainViewport();

        ImVec2 pos(whole_window->Pos.x + OPENGL_VIEWPORT_W, whole_window->Pos.y);
        SetNextWindowPos(ImVec2(pos));
        //TODO parametrize this
        SetNextWindowSize(ImVec2((WINDOW_W - OPENGL_VIEWPORT_W), WINDOW_H));

        ImGuiWindowFlags flags = 0;

        flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse
        | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar;
        if (!Begin("Work Space", NULL, flags))
        {   
            //TODO throw error 
            End();
            return;
        }
    }
    if (BeginTabBar("Workspace")) 
    {
        if(BeginTabItem("Engine Settings"))
        {
            using namespace renderer;
            using namespace settings;

            Spacing();
            SeparatorText("Render Target Framebuffer Settings");
            Spacing();
            {
                PushItemWidth(GetWindowWidth()*0.15);

                //TODO separate the evend handling logic and the gui stuff
                Checkbox ("Clear Depth Buffer"  , &DEPTH_CLR_ENBLD  ); SameLine();
                Checkbox ("Clear Color Buffer"  , &COLOR_CLR_ENBLD  ); SameLine();
                Checkbox ("Clear Stencil Buffer", &STENCIL_CLR_ENBLD);
                
                Spacing();
                
                Checkbox("Enable Depth Testing"  , &DEPTH_TEST_ENBLD  ); SameLine();
                Checkbox("Enable Stencil Testing", &STENCIL_TEST_ENBLD); SameLine();

                Spacing();

                ColorEdit4("Framebuffer Clear Color",        &CLR_COLOR.r, ImGuiColorEditFlags_NoInputs); 
                SameLine();
                ColorEdit3("Depth Viewing Color"    , &DEPTH_VIEW_COLOR.r, ImGuiColorEditFlags_NoInputs);

                Spacing();

                static int active_1 = 0;
                if (RadioButton("View Color Buffer", &active_1, 0))
                {
                    scr_display_mode = COLOR;
                }   SameLine();
                if (RadioButton("View Depth BUffer", &active_1, 1))
                {
                    scr_display_mode = DEPTH;
                }   
            }
            Spacing();
            Text("Framebuffer Dimensions");
            Spacing();
            {
                PopItemWidth();
                PushItemWidth(GetWindowWidth()*0.25);

                int render_w_candidate = int(RENDER_W), render_h_candidate = int(RENDER_H);

                if (InputInt("Width", &render_w_candidate, 50.0, 500.0, ImGuiInputTextFlags_None))
                {
                    if(render_w_candidate >= 0 && render_w_candidate <= MAX_RENDER_W)
                    {
                        RENDER_W = size_t(render_w_candidate);

                        events::should_update_scr_tex_coords = true;
                    }
                } 
                SameLine();
                if (InputInt("Height", &render_h_candidate, 50.0, 500.0, ImGuiInputTextFlags_None))
                {
                    if(render_h_candidate >= 0 && render_h_candidate <= MAX_RENDER_H)
                    {
                        RENDER_H = size_t(render_h_candidate);

                        events::should_update_scr_tex_coords = true;
                    }
                }
            }
            Spacing();
            Text("Framebuffer Texture Filtering");
            Spacing();
            {
                //TODO the depth texture and color texture both use the same settings, maybe worth it to separate them?

                static int magnification_filter = 0;
                static int minification_filter  = 0;
                static int mipmap_filter        = 0;

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
                    events::should_update_offscr_tex_param = true;
                } 
                SameLine();
                
                Checkbox("Use Mipmaps", &use_mipmaps);
                if (use_mipmaps)
                {   
                    SameLine();
                    Combo("", &mipmap_filter, "Linear\0Nearest\0\0");
                }
                
                if (Combo("Magnification", &magnification_filter, "Linear\0Nearest\0\0"))
                {
                    if (magnification_filter == 0)
                        scr_tex_mag_filter = LINEAR;
                    if (magnification_filter == 1)
                        scr_tex_mag_filter = NEAREST;
                    events::should_update_offscr_tex_param = true;
                }
            }
            Spacing();
            SeparatorText("Projection Settings");
            Spacing();
            {
                float RENDER_AR_candidate = RENDER_AR;

                static ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
                if (InputFloat("Aspect Ratio", &RENDER_AR_candidate, 0.1, 1.0, "%.3f", flags))
                {
                    if (RENDER_AR_candidate >= 0)
                    {
                        RENDER_AR = RENDER_AR_candidate;
                        events::should_update_projection = true;
                    }
                }
                SameLine();
                static int current_item = 1;
                Combo("Aspect Ratio Behavior", &current_item, "Independent\0Follows Render Dimensions\0Follows View Dimensions\0\0");

                if (current_item == 0)
                {
                    flags = ImGuiInputTextFlags_None;
                }
                if (current_item == 1)
                {
                    flags = flags|ImGuiInputTextFlags_ReadOnly;
                    RENDER_AR = float(RENDER_W)/RENDER_H;
                }
                if (current_item == 2)
                {
                    flags = flags|ImGuiInputTextFlags_ReadOnly;
                    RENDER_AR = float(OPENGL_VIEWPORT_W)/OPENGL_VIEWPORT_H;
                }

                DragFloat("FOV", &fov, 0.65, 0.0, 180.0  ); 
                SameLine();
                DragFloatRange2("Projection Plane", &near_plane, &far_plane, 0.05, 0.001, 100.0, "%.3f", NULL, ImGuiSliderFlags_AlwaysClamp);
            }
            EndTabItem();
        }
        EndTabBar();
    }
    End();
}
void conditional_gui()
{
    if (should_show_filedialog)
    {
        window::file_dialog.Display();
        if (window::file_dialog.HasSelected())
        {
            renderer::settings::path_to_object = window::file_dialog.GetSelected();
            events::should_update_import = true;

            window::file_dialog.ClearSelected();            
            should_show_filedialog = false;
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
                //this is pretty retarded. why do I have to open it everytime?
                window::file_dialog.Open();
                should_show_filedialog = true;
            }
            EndMenu();
        }
        if(BeginMenu("Options", false))
        {
            EndMenu();
        }
        ImGui::Separator();
        MenuItem("Quit", "esc", &events::should_quit);
        EndMenuBar();
    }
    End();
}
void GUI::render()
{
    workspace_panel();
    main_bar();
    conditional_gui();
}