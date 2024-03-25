#include <GUI.h>


GUI::GUI(Window &window) {
    this->window = window;
}

void GUI::setupGUI() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    this->io = ImGui::GetIO(); 
    (void)this->io;
    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(this->window.getWindow(), this->window.getRenderer());
    ImGui_ImplSDLRenderer2_Init(this->window.getRenderer());    
}

void GUI::beforeUpdate() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();  
}

void GUI::destroy() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext(); 
}

ImGuiIO GUI::getIO() {
    return this->io;
}