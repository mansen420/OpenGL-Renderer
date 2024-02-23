#include "engine_interface.h"
#include "gui.h"
//TODO separate the logic into the event handler module...
//update : right now, gui.cpp only directly handles global varaiables declared in engine_state.h
static bool should_show_filedialog = false;
void workspace_panel()
{
    //FIXME probably a bad idea
    using namespace ImGui;
    ImGuiViewport* whole_window = GetMainViewport();
    {

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
        {   //FIXME
            using namespace renderer;

            Spacing();
            SeparatorText("Render Target Framebuffer Settings");
            Spacing();
            {
                PushItemWidth(GetWindowWidth()*0.15);

                //TODO separate the evend handling logic and the gui stuff
                Checkbox ("Clear Depth Buffer"  , &ENGINE_SETTINGS.DEPTH_CLR_ENBLD); SameLine();
                Checkbox ("Clear Color Buffer"  , &ENGINE_SETTINGS.COLOR_CLR_ENBLD); SameLine();
                Checkbox ("Clear Stencil Buffer", &ENGINE_SETTINGS.STENCIL_CLR_ENBLD);
                
                Spacing();
                
                Checkbox("Enable Depth Testing"  , &ENGINE_SETTINGS.DEPTH_TEST_ENBLD  ); SameLine();
                Checkbox("Enable Stencil Testing", &ENGINE_SETTINGS.STENCIL_TEST_ENBLD); SameLine();

                Spacing();

                ColorEdit4("Framebuffer Clear Color",        &ENGINE_SETTINGS.CLR_COLOR.r, ImGuiColorEditFlags_NoInputs); 
                SameLine();
                ColorEdit3("Depth Viewing Color"    , &ENGINE_SETTINGS.DEPTH_VIEW_COLOR.r, ImGuiColorEditFlags_NoInputs);

                Spacing();

                static int active_1 = 0;
                if (RadioButton("View Color Buffer", &active_1, 0))
                {
                    ENGINE_SETTINGS.DISPLAY_BUFFER = COLOR;
                }   SameLine();
                if (RadioButton("View Depth BUffer", &active_1, 1))
                {
                    ENGINE_SETTINGS.DISPLAY_BUFFER = DEPTH;
                }   
            }
            Spacing();
            Text("Framebuffer Texture Filtering");
            Spacing();
            {
                //TODO the depth texture and color texture both use the same settings, maybe worth it to separate them?
                constexpr unsigned int LINEAR = 0, NEAREST = 1;
                static int magnification_filter = LINEAR;
                static int minification_filter  = LINEAR;
                static int mipmap_filter        = LINEAR;

                if (Combo("Minification", &minification_filter, "Linear\0Nearest\0\0"))
                {
                    //FIXME please find a better way to do this, this is ugly as fuck 
                    if(ENGINE_SETTINGS.USE_MIPMAPS)
                    {
                        if(minification_filter == LINEAR)
                        {
                            if(mipmap_filter == LINEAR)
                                ENGINE_SETTINGS.SCR_TEX_MIN_FLTR = texture_filtering::MIPMAP_LINEAR_LINEAR;
                            if (mipmap_filter == NEAREST)
                                ENGINE_SETTINGS.SCR_TEX_MIN_FLTR = texture_filtering::MIPMAP_NEAREST_LINEAR;
                        }
                        if(minification_filter == NEAREST)
                        {
                            if(mipmap_filter == LINEAR)
                                ENGINE_SETTINGS.SCR_TEX_MIN_FLTR = texture_filtering::MIPMAP_LINEAR_NEAREST;
                            if (mipmap_filter == NEAREST)
                                ENGINE_SETTINGS.SCR_TEX_MIN_FLTR = texture_filtering::MIPMAP_NEAREST_NEAREST;
                        }
                    }
                    else
                    {
                        if(minification_filter == LINEAR)
                            ENGINE_SETTINGS.SCR_TEX_MIN_FLTR = texture_filtering::LINEAR;
                        if(minification_filter == NEAREST)
                            ENGINE_SETTINGS.SCR_TEX_MIN_FLTR = texture_filtering::NEAREST;
                    }
                } 
                SameLine();
                
                Checkbox("Use Mipmaps", &ENGINE_SETTINGS.USE_MIPMAPS);
                if (ENGINE_SETTINGS.USE_MIPMAPS)
                {  
                    SameLine();
                    //FIXME this combo doesn't open
                    //TODO figure out why using mipmaps cuases the screen to go dark
                    //update : it seems that the image is dark because no mipmaps are generated, i.e. it samples from non-existant mipmaps thus the darker image.
                    //telling the GL to generate mipmaps every time we update the tex params causes an invlid operation error.
                    if (Combo("Mipmap", &mipmap_filter, "Linear\0Nearest\0\0"))
                    {
                        if (minification_filter == LINEAR)
                        {
                            if(mipmap_filter == LINEAR)
                                ENGINE_SETTINGS.SCR_TEX_MIN_FLTR = texture_filtering::MIPMAP_LINEAR_LINEAR;
                            if(mipmap_filter == NEAREST)
                                ENGINE_SETTINGS.SCR_TEX_MIN_FLTR = texture_filtering::MIPMAP_NEAREST_LINEAR;
                        } 
                        else if (minification_filter == NEAREST)
                        {
                            if(mipmap_filter == LINEAR)
                                ENGINE_SETTINGS.SCR_TEX_MIN_FLTR = texture_filtering::MIPMAP_LINEAR_NEAREST;
                            if(mipmap_filter == NEAREST)
                                ENGINE_SETTINGS.SCR_TEX_MIN_FLTR = texture_filtering::MIPMAP_NEAREST_NEAREST;
                        }
                    }
                }
                
                if (Combo("Magnification", &magnification_filter, "Linear\0Nearest\0\0"))
                {
                    if (magnification_filter == LINEAR)
                        ENGINE_SETTINGS.SCR_TEX_MAG_FLTR = texture_filtering::LINEAR;
                    if (magnification_filter == NEAREST)
                        ENGINE_SETTINGS.SCR_TEX_MAG_FLTR = texture_filtering::NEAREST;
                }
            }
            Spacing();
            Text("Render Dimensions");
            Spacing();
            {
                PopItemWidth();
                PushItemWidth(GetWindowWidth()*0.25);

                int render_w_candidate = int(ENGINE_SETTINGS.RENDER_W), render_h_candidate = int(ENGINE_SETTINGS.RENDER_H);

                if (InputInt("Width", &render_w_candidate, 50.0, 500.0, ImGuiInputTextFlags_None))
                {
                    if(render_w_candidate >= 0 && render_w_candidate <= MAX_RENDER_W)
                    {
                        ENGINE_SETTINGS.RENDER_W = size_t(render_w_candidate);
                    }
                } 
                SameLine();
                if (InputInt("Height", &render_h_candidate, 50.0, 500.0, ImGuiInputTextFlags_None))
                {
                    if(render_h_candidate >= 0 && render_h_candidate <= MAX_RENDER_H)
                    {
                        ENGINE_SETTINGS.RENDER_H = size_t(render_h_candidate);
                    }
                }
            }
            Spacing();
            {
                static int current_item = 1;
                Combo("Render View Mode", &current_item, 
                "Fit To Viewport\0Keep Aspect Ratio\0\0");
                if(current_item == 0)
                    ENGINE_SETTINGS.RENDER_TO_VIEW_MODE = FIT_TO_VIEW;
                if(current_item == 1)
                    ENGINE_SETTINGS.RENDER_TO_VIEW_MODE = CROP;
                SameLine();
                Checkbox("Show Actual Size", &ENGINE_SETTINGS.SHOW_REAL_RENDER_SIZE);
            }
            Spacing();
            SeparatorText("Render Projection Matrix Settings");
            Spacing();
            {
                float RENDER_AR_candidate = ENGINE_SETTINGS.RENDER_AR;

                static ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
                if (InputFloat("Aspect Ratio", &RENDER_AR_candidate, 0.1, 1.0, "%.3f", flags))
                {
                    if (RENDER_AR_candidate >= 0)
                    {
                        ENGINE_SETTINGS.RENDER_AR = RENDER_AR_candidate;
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
                    ENGINE_SETTINGS.RENDER_AR = float(ENGINE_SETTINGS.RENDER_W)/ENGINE_SETTINGS.RENDER_H;
                }
                if (current_item == 2)
                {
                    flags = flags|ImGuiInputTextFlags_ReadOnly;
                    ENGINE_SETTINGS.RENDER_AR = float(OPENGL_VIEWPORT_W)/OPENGL_VIEWPORT_H;
                }

                Spacing();

                DragFloat("FOV", &ENGINE_SETTINGS.FOV, 0.65, 0.0, 180.0  ); 
                SameLine();
                DragFloatRange2("Projection Plane", &ENGINE_SETTINGS.NEAR_PLANE, &ENGINE_SETTINGS.FAR_PLANE, 0.05, 0.001, 100.0, "%.3f", NULL, ImGuiSliderFlags_AlwaysClamp);
            }
            EndTabItem();
        }
        if(BeginTabItem("Shaders"))
        {
            //TODO make this a dynamic size
            static char shader_code_buffer[1024*16];
            static bool first_time = true;
            if (first_time)
                strcpy(shader_code_buffer, renderer::get_shader_source_reflection(renderer::OBJECT_SHADER, renderer::FRAGMENT_SHADER));
            first_time = false;

            GetFont()->Scale =1.5f;
            PushFont(GetFont());
            ImGui::InputTextMultiline("##source", shader_code_buffer, sizeof(shader_code_buffer), whole_window->Size);
            GetFont()->Scale =1.0f;
            PopFont();

            Spacing();
            if(Button("Compile"))
            {
                renderer::update_shader(renderer::OBJECT_SHADER, renderer::FRAGMENT_SHADER, shader_code_buffer);
            }
            SameLine();
            if(Button("Link"))
            {
                renderer::link_program(renderer::OBJECT_SHADER);
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
            renderer::ENGINE_SETTINGS.PATH_TO_OBJ = window::file_dialog.GetSelected();

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
        if (MenuItem("Quit", "esc"))
        {
            glfwSetWindowShouldClose(myWindow, true);
        }
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