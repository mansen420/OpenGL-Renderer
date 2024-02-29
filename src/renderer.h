#pragma once

/// @brief Core engine module
namespace renderer
{ //TODO this defines the public interface to the event handler.

    void terminate();
    void update_import();
    void clear_buffers();
    void render_scene();
    void update_screen_tex_coords();
    void update_offscreen_tex_params();
    int init();
}