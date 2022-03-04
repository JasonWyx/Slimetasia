#pragma once
#include "External Libraries/imgui/imgui.h"

struct GLFWwindow;

namespace ImGuiUtils
{
    IMGUI_API bool Init(GLFWwindow* window, bool install_callbacks, const char* glsl_version = NULL);
    IMGUI_API void Shutdown();
    IMGUI_API void StartFrame();
    IMGUI_API void RenderDrawData(ImDrawData* draw_data);

    // Use if you want to reset your rendering device without losing ImGui state.
    IMGUI_API void InvalidateDeviceObjects();
    IMGUI_API bool CreateDeviceObjects();
};  // namespace ImGuiUtils