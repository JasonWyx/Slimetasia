#pragma once
#include "External Libraries/imgui/imgui.h"

void ImGuiRenderer(ImDrawData* draw_data);
void ImGuiRenderDrawLists(ImDrawData* draw_data);
bool ImGuiCreateDeviceObjects();
bool ImGuiCreateFontsTexture();