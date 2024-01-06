#include <stdio.h>
#include <imgui.h>

int main (int argc, char **argv) {
    printf("[MAIN] Setting up ImGUI lib ...\n");
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    return 0;
}
