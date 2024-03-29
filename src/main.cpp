#include "window.h"
#include "engine_interface.h"
//planned features :
//1. input handling (x)
//2. GUI elements (x)
//3. Import feature (x)
//4. Screen space processing (x)
//5. shader pre-processor ~~(probably not)~~ (actually did it!)
//6. shader post-processor (x)
//7. center and rescale objects on load (1/2)
//8. have default objects (cube or sphere) for experimenting with shaders (x)
//9. allow editing of object physical attributes on gui (1/2) 
//10. allow positioning of viewport inside renderport (3/4)
//11. write manual ()
//12. display GL state information on gui ()
//13. make camera system (x)
//14. display render stats and read-only info on gui ()
//15. separate engine interface from gui engine reflection (mostly yes)
//16. rework object module ()
//we need abstraction for (1/2)shaders, ()uniforms, (x)drawing
int main()
{   
    if (!window::init()){window::terminate(); return -1;}
    if (!renderer::init()){window::terminate(); renderer::terminate(); return-1;}
    while (!window::should_close())
    {
        window::poll_events();

        renderer::update_state();
        renderer::render_scene();

        window::process_input();
        window::render_gui();

        window::swap_buffers();
    }
    renderer::terminate();
    window::terminate();
    return 0;
}