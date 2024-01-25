#include "window.h"
#include "renderer.h"

GLFWwindow* myWindow;
int main()
{
    if (!window::init())
        return -1;
    while (true)
    {
        window::poll_events();

        renderer::render_background(0.6, 0.3, 0.3, 1.0);
        renderer::render_scene();

        window::render_gui();

        window::swap_buffers();
    }
    window::terminate();
    return 0;
}