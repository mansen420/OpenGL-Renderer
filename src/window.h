#pragma once 
#include "imgui.h"
#include "imfilebrowser.h"
namespace window
{
    extern ImGui::FileBrowser file_dialog;
    int init();

    void render_gui();
    void poll_events();
    void swap_buffers();
    void terminate();
}