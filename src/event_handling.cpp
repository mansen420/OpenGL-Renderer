#include "event_handling.h"
#include "renderer.h"
void events::import()
{
    std::cout << "IMPORT!" << std::endl;    //temporary, obviously
}
void events::quit(){glfwSetWindowShouldClose(myWindow, true);}
void events::quit_window()
{
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
    if(should_import) {import();}
    if (should_quit){quit();}
}