#include "window.h"
#include "renderer.h"

//what we need :
//1. input handling (1/2)
//2. GUI elements (x)
//3. Import feature (1/2)
//4. Screen space processing
//5. shader pre-processor 
//6. shader post-processor
//7. center and rescale objects on load
//8. have default objects (cube or sphere) for experimenting with shaders
//9. allow editing of object physical attributes on gui 
//we need abstraction for ()shaders, ()uniforms, (x)drawing
int main()
{   
    if (!window::init())
        return -1;
    renderer::init();
    while (!glfwWindowShouldClose(myWindow))
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