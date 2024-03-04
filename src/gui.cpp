#include "engine_interface.h"
#include "gui.h"
#include "input_handling.h"

#include <fstream>

static bool should_show_filedialog = false;
static bool load_obj_path          = false;
static bool load_shader_path       = false;
bool read_file(const char* file_path, char* &file_contents_holder);
//TODO make this a dynamic size?
static char shader_code_buffer[1024*16]; //16KB
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
        if(BeginTabItem("Engine Parameters"))
        {   //FIXME
            using namespace renderer;

            Spacing();
            SeparatorText("Render Target Framebuffer Parameters");
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
            SeparatorText("Render Projection Matrix Pameters");
            Spacing();
            {
                float RENDER_AR_candidate = ENGINE_SETTINGS.RENDER_AR;

                static ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
                PushItemWidth(GetWindowWidth()*0.15);
                if (InputFloat("Aspect Ratio", &RENDER_AR_candidate, 0.1, 1.0, "%.3f", flags))
                {
                    if (RENDER_AR_candidate >= 0)
                    {
                        ENGINE_SETTINGS.RENDER_AR = RENDER_AR_candidate;
                    }
                }
                PopItemWidth();
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
                PushItemWidth(GetWindowWidth()*0.15);
                DragFloat("FOV", &ENGINE_SETTINGS.FOV, 0.65, 0.0, 180.0  ); 
                PopItemWidth();
                SameLine();
                DragFloatRange2("Projection Plane", &ENGINE_SETTINGS.NEAR_PLANE, &ENGINE_SETTINGS.FAR_PLANE, 0.05, 0.001, 100.0, "%.3f", NULL, ImGuiSliderFlags_AlwaysClamp);
            }
            Spacing();
            SeparatorText("Camera Parameters");
            Spacing();
            {
                PushItemWidth(GetWindowWidth()*0.15);

                DragFloat("PHI", &camera::CAMERA_PARAMS.PHI, 1.0);
                SameLine();
                DragFloat("THETA", &camera::CAMERA_PARAMS.THETA, 1.0);
                SameLine();
                DragFloat("Distance to Origin", &camera::CAMERA_PARAMS.DIST, 0.1);

                DragFloat("Resistance (deg/s)", &camera::CAMERA_PARAMS.RESISTANCE_FACTOR, 1.f);
                SameLine();
                DragFloat("Max Speed (deg/s)", &camera::CAMERA_PARAMS.MAX_SPEED, 1);
                
                DragFloat("Speed Increment (deg/s)", &window::input_camera_acceleration, 0.5);
                PopItemWidth();
            }
            Spacing();
            SeparatorText("Object Parameters");
            {
                DragFloat("Scale Factor", &ENGINE_SETTINGS.OBJ_SCALE_FACTOR, 0.005);
                SameLine();
                DragFloat3("Displacement", glm::value_ptr(ENGINE_SETTINGS.OBJ_DISPLACEMENT), 0.01);

                static bool show_dimensions = false;
                if(Button("Calculate Dimensions"))
                {
                    renderer::calculate_object_dimensions();
                    show_dimensions = true;
                }
                SameLine();
                if (Button("Center"))
                {
                    renderer::center_object();
                }
                SameLine();
                static float scale;
                if(DragFloat("Normalize Object Scale", &scale, 0.005))
                {
                    renderer::rescale_object(scale);
                }
                if (show_dimensions)
                {
                    Text("Dimensions (before transforms) : ");SameLine();
                    std::ostringstream ss;
                    ss << renderer::ENGINE_SETTINGS.OBJ_DIMENSIONS[0] << ' ' << renderer::ENGINE_SETTINGS.OBJ_DIMENSIONS[0]
                    << ' ' << renderer::ENGINE_SETTINGS.OBJ_DIMENSIONS[0];
                    Text(ss.str().c_str());

                    ss.str(std::string()); //empty stream

                    Text("Center (before transforms) : "); SameLine();
                    ss << renderer::ENGINE_SETTINGS.OBJ_CENTER[0] << ' ' << renderer::ENGINE_SETTINGS.OBJ_CENTER[1]
                    << ' ' << renderer::ENGINE_SETTINGS.OBJ_CENTER[2];
                    Text(ss.str().c_str());
                }
            }
            Spacing();
            EndTabItem();
        }
        if(BeginTabItem("Shaders"))
        {
            //TODO allow reading from file and writing to disk (shaders)
            static bool should_load_shader = true;

            if(Button("Load File"))
            {
                window::file_dialog.Open();
                should_show_filedialog = true;
                load_shader_path = true;
            }
            static int shader_type_option = 0;
            static int prog_type_option   = 0;

            static renderer::shader_type_option shader_type = renderer::FRAGMENT_SHADER;
            static renderer::shader_prg_option prog_type    = renderer::OBJECT_SHADER;
            PushItemWidth(GetWindowWidth()*0.25);
            if (Combo("Shader Type", &shader_type_option, "Fragment Shader\0Vertex Shader\0\0\0"))
            {
                if(shader_type_option == 0)
                    shader_type = renderer::FRAGMENT_SHADER;
                if(shader_type_option == 1)
                    shader_type = renderer::VERTEX_SHADER;
                should_load_shader = true;
            }
            SameLine();
            if (Combo("Program", &prog_type_option, "Object Shader\0Screen Shader\0\0\0"))
            {
                if(prog_type_option == 0)
                    prog_type = renderer::OBJECT_SHADER;
                if(prog_type_option == 1)
                    prog_type = renderer::POSTPROCESS_SHADER;
                should_load_shader = true;
            }
            PopItemWidth();

            if (should_load_shader)
            {
                strcpy(shader_code_buffer, renderer::get_shader_source_reflection(prog_type, shader_type));
                should_load_shader = false;
            }

            ImGui::InputTextMultiline("##source", shader_code_buffer, sizeof(shader_code_buffer), whole_window->Size);

            Spacing();
            if(Button("Compile"))
            {
                renderer::update_shader(prog_type, shader_type, shader_code_buffer);
            }
            SameLine();
            if(Button("Link"))
            {
                renderer::link_program(prog_type);
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
        if (load_shader_path)
            window::file_dialog.SetTypeFilters({".frag", ".vert", ".glsl", ".fs", ".vs"});
        if (load_obj_path)
            window::file_dialog.SetTypeFilters({".obj", ".OBJ"});

        window::file_dialog.Display();
        if (window::file_dialog.HasSelected())
        {
            if(load_obj_path)
            {
                renderer::ENGINE_SETTINGS.PATH_TO_OBJ = window::file_dialog.GetSelected();
                load_obj_path = false;
            }
            if(load_shader_path)
            {
                char* source;
                read_file(window::file_dialog.GetSelected().c_str(), source);
                strcpy(shader_code_buffer, source);
                delete[] source;
                load_shader_path = false;
            }
            window::file_dialog.ClearSelected(); 
                       
            should_show_filedialog = false;
            window::file_dialog.Close();
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
    //PushItemWidth(GetFontSize());
    if(BeginMenuBar())  //main menu bar
    {
        if(BeginMenu("File"))
        {
            if(MenuItem("Import", "ctrl+o", false))
            {
                window::file_dialog.Open();
                should_show_filedialog = true;
                load_obj_path = true;
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

//FIXME Utility function. move this somewhere else!!
//Caller must ensure that file_contents_holder is delete[]`d!
bool read_file(const char* file_path, char* &file_contents_holder)
{
    std::ifstream reader;
    reader.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        reader.open(file_path);
        std::stringstream file_stream;
        file_stream << reader.rdbuf();
        reader.close();
        file_contents_holder = new char[strlen(file_stream.str().c_str())];
        strcpy(file_contents_holder, file_stream.str().c_str());
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "failed to open file : " << file_path << std::endl;
        return false;
    }
    return true;
}