#include "AppConsole.h"

#include <fstream>
#include <iostream>
#include <limits>

void AppConsole::Draw()
{
    // SetNextWindowSize(ImVec2((GetIO().DisplaySize.x) / 4 * 3, 360.f));
    // SetNextWindowPos(ImVec2((float)(0.f), GetIO().DisplaySize.y - 360.f));

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    if (ActiveWindow)
    {
        BeginDock("Console", &ActiveWindow);
        if (Button("Trigger Auto Scroll")) m_TriggerAutoScroll = !m_TriggerAutoScroll;
        SameLine();
        if (Button("Clear Log ")) ClearLog();
        SameLine();
        if (Button("Output Log ")) OutLog();
        SameLine();
        PushItemWidth(60.f);
        ImGui::Text("Console's Max Lines : ");
        SameLine();
        InputScalar("", ImGuiDataType_U32, &m_MaxLines);
        PopItemWidth();
        Separator();
        const float footer_height_to_reserve = GetStyle().ItemSpacing.y + GetFrameHeightWithSpacing();
        BeginDockChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (unsigned i = 0; i < m_Items.size(); i++)
        {
            if (m_Items[i] == "") continue;
            std::string item = m_Items[i];
            ImVec4 col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            PushStyleColor(ImGuiCol_Text, col);
            TextUnformatted((item).c_str());
            PopStyleColor();
        }
        if (m_ScrollToBtm && m_TriggerAutoScroll) SetScrollHere(1.0f);
        m_ScrollToBtm = false;
        EndDockChild();
        EndDock();
    }
}

void AppConsole::SetMaxLines(unsigned lines)
{
    m_MaxLines = lines;
}

void AppConsole::AddLog(const char* log)
{
    if (log != std::string(""))
    {
        if (m_Items.size() == m_MaxLines) m_Items.pop_front();
        m_Items.push_back(std::string(log));
        m_ScrollToBtm = true;
    }
}

void AppConsole::OutLog()
{
    std::ofstream output;
    output.open("OutputLog.txt");
    for (auto l : m_Items)
    {
        output << l;
    }
    output.close();
}

void AppConsole::ClearLog()
{
    m_Items.clear();
    m_Items.resize(m_MaxLines);
}
