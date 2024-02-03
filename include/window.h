#pragma once 

namespace window
{
    int init();
    
    void render_gui();
    void poll_events();
    void swap_buffers();
    void terminate();
}