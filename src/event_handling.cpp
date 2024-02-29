#include "event_handling.h"
#include "renderer.h"
#include <string>

bool events::should_update_scr_tex_coords   = false;
bool events::should_quit                    = false;
bool events::should_update_import           = false;
bool events::should_update_offscr_tex_param = false;
bool events::should_update_projection       = false;

void quit(){glfwSetWindowShouldClose(myWindow, true);}
void quit_window()
{   //TODO use modal popups to do this
    //maybe do this?
    using namespace ImGui;
    ImGuiViewport* whole_window = GetMainViewport();
    SetNextWindowPos(whole_window->Pos);
    SetNextWindowSize(ImVec2(200, 200));

    if(!Begin("Quit")){End();}
    Text("Do you want to quit?");
    End();
}

void events::react()
{
    if(should_update_offscr_tex_param)
    {
        renderer::update_offscreen_tex_params();
        should_update_offscr_tex_param = false;
    }
    if(should_update_import)
    {
        renderer::update_import();
        should_update_import = false;
    }
    if(should_update_scr_tex_coords)
    {
        renderer::update_screen_tex_coords();
        should_update_scr_tex_coords = false;
    }
    if (should_quit) 
        quit();
}
