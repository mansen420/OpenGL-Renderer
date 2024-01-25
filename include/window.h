#ifndef WINDOW
#define WINDOW

namespace window
{
    bool init();
    void poll_events();
    void swap_buffers();
    void render_gui();
    void terminate();
}

#endif