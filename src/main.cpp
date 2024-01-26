#include "window.h"
#include "renderer.h"

//what we need :
//1. input handling
//2. GUI elements
//3. Import feature
//4. Screen space processing
//5. ???
int main()
{   
    if (!window::init())
        return -1;
    renderer::init();
    while (true)
    {
        window::poll_events();

        //renders whatever objects there are in the scene with the currently active shaders
        renderer::render_scene();

        window::render_gui();

        window::swap_buffers();
    }
    window::terminate();
    return 0;
}