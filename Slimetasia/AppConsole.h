#pragma once
#include <deque>
#include <iostream>

#include "External Libraries\imgui\imgui.h"

using namespace ImGui;

class AppConsole
{
    std::deque<std::string> m_Items;
    unsigned m_MaxLines;
    bool m_ScrollToBtm;
    bool m_TriggerAutoScroll;

public:
    AppConsole(unsigned MAX_LINES = 50)
        : m_MaxLines(MAX_LINES)
        , m_ScrollToBtm(false)
        , m_TriggerAutoScroll(true)
        , m_IsActiveWindow(false)
    {
        m_Items.resize(MAX_LINES);
    }
    void Draw();
    void SetMaxLines(unsigned lines);
    void AddLog(const char* log);
    void OutLog();
    void ClearLog();

    bool m_IsActiveWindow;
};
