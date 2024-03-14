#pragma once 
#include "imgui.h"
#include "imfilebrowser.h"
#include "input_handling.h"
namespace window
{
    extern ImGui::FileBrowser file_dialog;
    int init();
    bool should_close();
    void render_gui();
    void poll_events();
    void swap_buffers();
    void terminate();
}