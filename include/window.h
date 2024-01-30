#pragma once 

namespace window
{
    bool init();
    
    void render_gui();
    void poll_events();
    void swap_buffers();
    void terminate();
}