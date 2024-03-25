#pragma once

#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_sdl2.h>
#include <ImGui/imgui_impl_sdlrenderer2.h>
#include <ImGui/ImGuiFileDialog.h>
#include <ImGui/ImGuiFileDialogConfig.h>

#include <Window.h>


class GUI {
    private :
        ImGuiIO io;
        Window window;

    public:
        GUI(Window &window);
        void setupGUI();
        void beforeUpdate();
        void destroy(); 

        ImGuiIO getIO();
};