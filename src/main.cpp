#include "window.h"
#include "renderer.h"

//planned features :
//1. input handling (1/2)
//2. GUI elements (1/2)
//3. Import feature (1/2)
//4. Screen space processing (x)
//5. shader pre-processor 
//6. shader post-processor 
//7. center and rescale objects on load
//8. have default objects (cube or sphere) for experimenting with shaders
//9. allow editing of object physical attributes on gui 
//10. allow positioning of viewport inside renderport
//11. write manual
//12. display GL state information on gui
//13. make camera system
//we need abstraction for (1/2)shaders, ()uniforms, (x)drawing
int main()
{   
    if (!window::init()){window::terminate(); return -1;}
    if (!renderer::init()){window::terminate(); renderer::terminate(); return-1;}
    while (!glfwWindowShouldClose(myWindow))
    {
        window::poll_events();

        renderer::render_scene();

        window::render_gui();

        window::swap_buffers();
    }
    renderer::terminate();
    window::terminate();
    return 0;
}